#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tsv.h"

#define MAX_LINE_LENGTH 8192

struct tsv {
	// array<char const*>
	row *header;

	// a 2d-array of type array<array<char const*>>
	array *rows;
};

static bool is_cr_or_lf(char c) {
	return c == '\n' || c == '\r';
}

static row *parse_line_to_row(char *line) {
	if (line == (void *) 0 || *line == '\0') {
		return array_new();
	}

	row *parsed_row = array_new();

	if (parsed_row == (void *) 0) {
		return (void *) 0;
	}

	size_t len = strlen(line);

	if (len > 0 && is_cr_or_lf(line[len - 1])) {
		line[len - 1] = '\0';

		if (len > 1 && is_cr_or_lf(line[len - 2])) {
			line[len - 2] = '\0';
		}
	}

	char *token;
	char *saveptr;

	token = strtok_r(line, "\t", &saveptr);

	while (token != (void *) 0) {
		char *field = strdup(token);

		if (field == (void *) 0) {
			tsv_clear_row(parsed_row);

			return (void *) 0;
		}

		result res = array_append(parsed_row, field);
		if (res.error != (void *) 0) {
			free(res.error);
			free(field);

			tsv_clear_row(parsed_row);

			return (void *) 0;
		}

		token = strtok_r(NULL, "\t", &saveptr);
	}

	return parsed_row;
}

static void write_row_to_file(FILE *f, row *r) {
	assert(f != (void *) 0 && r != (void *) 0);

	size_t header_len = array_len(r);

	for (size_t i = 0; i < header_len; ++i) {
		result res = array_at(r, i);

		if (res.error != (void *) 0) {
			free(res.error);
		}

		if (res.p != (void *) 0) {
			fprintf(f, "%s", (char *) res.p);
		}

		if (i < header_len - 1) {
			fprintf(f, "\t");
		}
	}

	fprintf(f, "\n");
}

tsv *tsv_new(row *header) {
	if (header == (void *) 0) {
		return (void *) 0;
	}

	tsv *self = malloc(sizeof(tsv));

	if (self == (void *) 0) {
		tsv_clear_row(header);
		return (void *) 0;
	}

	self->header = header;
	self->rows = array_new();

	if (self->rows == (void *) 0) {
		tsv_clear_row(header);
		free(self);
		return (void *) 0;
	}

	return self;
}

void tsv_del(tsv *t) {
	if (t == (void *) 0) {
		return;
	}

	if (t->header != (void *) 0) {
		tsv_clear_row(t->header);
		t->header = (void *) 0;
	}

	if (t->rows != (void *) 0) {
		size_t rows_len = array_len(t->rows);

		for (size_t i = 0; i < rows_len; ++i) {
			result res = array_at(t->rows, i);

			if (res.error != (void *) 0) {
				free(res.error);
				continue;
			}

			tsv_clear_row(res.p);
		}

		array_del(t->rows);
		t->rows = (void *) 0;
	}

	free(t);
}

void tsv_clear_row(row *row) {
	size_t len = array_len(row);

	for (size_t i = 0; i < len; ++i) {
		result r = array_at(row, i);
		if (r.p != (void *) 0) {
			free(r.p);
		}
	}

	array_del(row);
}

row *tsv_at(tsv *self, size_t index) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	assert(self->rows != (void *) 0);

	result res = array_at(self->rows, index);
	if (res.error != (void *) 0) {
		free(res.error);
		return (void *) 0;
	}

	return res.p;
}

row *tsv_header(tsv *self) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	assert(self->header != (void *) 0);

	return self->header;
}

row *tsv_append(tsv *self, row *new_row) {
	if (self == (void *) 0 || new_row == (void *) 0) {
		return (void *) 0;
	}

	assert(self->rows != (void *) 0);

	result res = array_append(self->rows, new_row);

	if (res.error != (void *) 0) {
		free((void *) res.error);
		return (void *) 0;
	}

	return res.p;
}

row *tsv_remove(tsv *self, size_t index) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	assert(self->rows != (void *) 0);

	result res = array_remove(self->rows, index);

	if (res.error != (void *) 0) {
		free((void *) res.error);
		return (void *) 0;
	}

	return (row *) res.p;
}

size_t tsv_len(tsv *self) {
	if (self == (void *) 0) {
		return 0;
	}

	assert(self->rows != (void *) 0);

	return array_len(self->rows);
}

tsv *tsv_import_file(FILE *f) {
	if (f == (void *) 0) {
		return (void *) 0;
	}

	char line_buffer[MAX_LINE_LENGTH];
	row *header_row = (void *) 0;
	tsv *new_tsv = (void *) 0;

	if (fgets(line_buffer, sizeof(line_buffer), f) == (void *) 0) {
		return (void *) 0;
	}

	header_row = parse_line_to_row(line_buffer);

	if (header_row == (void *) 0) {
		return (void *) 0;
	}

	new_tsv = tsv_new(header_row);

	if (new_tsv == (void *) 0) {
		return (void *) 0;
	}

	while (fgets(line_buffer, sizeof(line_buffer), f) != (void *) 0) {
		row *data_row = parse_line_to_row(line_buffer);

		if (data_row == (void *) 0) {
			tsv_del(new_tsv);
			return (void *) 0;
		}

		row *appended_row = tsv_append(new_tsv, data_row);

		if (appended_row == (void *) 0) {
			tsv_clear_row(data_row);
			tsv_del(new_tsv);
			return (void *) 0;
		}
	}

	return new_tsv;
}

void tsv_export_file(tsv *t, FILE *f) {
	if (t == (void *) 0 || f == (void *) 0) {
		return;
	}

	assert(t->header != (void *) 0);
	assert(t->rows != (void *) 0);

	write_row_to_file(f, t->header);

	size_t rows_len = array_len(t->rows);
	for (size_t i = 0; i < rows_len; ++i) {
		result res = array_at(t->rows, i);

		if (res.error != (void *) 0) {
			free(res.error);
			continue;
		}

		if (res.p != (void *) 0) {
			row *r = res.p;

			write_row_to_file(f, r);
		}
	}
}
