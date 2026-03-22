#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inventory.h"

#include "array.h"
#include "tsv.h"

struct equipment {
	char *id;
	char *name;
	struct quantity qty;
};

struct inventory {
	// array<equipment*>
	array *items;
};

static void inventory_clear_contents(struct inventory *self) {
	if (self == (void *) 0 || self->items == (void *) 0) {
		return;
	}

	size_t len = array_len(self->items);

	for (size_t i = 0; i < len; ++i) {
		result res = array_at(self->items, i);

		if (res.error != (void *) 0) {
			free(res.error);
			continue;
		}

		if (res.p != (void *) 0) {
			equipment_del(res.p);
		}
	}

	array_del(self->items);

	self->items = array_new();
}

static char *size_t_to_str(size_t qty) {
	// 22 because generally size_t may contain 45 decimal digits
	// considering log_10(2**64) is approx. equal to 20.

	char buffer[32];
	int chars_written = snprintf(buffer, sizeof(buffer), "%zu", qty);

	if (chars_written < 0 || (size_t) chars_written >= sizeof(buffer)) {
		return (void *) 0;
	}

	return strdup(buffer);
}

static int append_cell_str_dup(row *target_row, const char *str) {
	if (str == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	char *duplicated_str = strdup(str);

	if (duplicated_str == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	result res = array_append(target_row, duplicated_str);

	if (res.error != (void *) 0) {
		free(res.error);
		free(duplicated_str);
		return INVENTORY_FAILURE;
	}

	return INVENTORY_SUCCESS;
}

static int append_cell_str_move(row *target_row, char *str) {
	if (str == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	result res = array_append(target_row, str);

	if (res.error != (void *) 0) {
		free(res.error);
		free(str);
		return INVENTORY_FAILURE;
	}

	return INVENTORY_SUCCESS;
}

static bool header_field_differs(tsv *t, size_t index, const char *expected) {
	row *header = tsv_header(t);

	if (header == (void *) 0 || array_len(header) != 5) {
		return true;
	}

	result res = array_at(header, index);

	if (res.error != (void *) 0) {
		free(res.error);
		return true;
	}

	char *actual = (char *) res.p;
	if (actual == (void *) 0 || strcmp(actual, expected) != 0) {
		return true;
	}

	return false;
}

static char *get_cell_content(int *status, row *row_data, size_t index) {
	result res = array_at(row_data, index);

	if (res.error != (void *) 0) {
		free(res.error);
		*status = INVENTORY_FAILURE;
		return (void *) 0;
	}

	char *content_str = (char *) res.p;

	if (content_str == (void *) 0) {
		*status = INVENTORY_FAILURE;
		return (void *) 0;
	}

	return content_str;
}

static size_t str_to_size_t(int *status, const char *str) {
	if (str == (void *) 0 || *str == '\0') {
		*status = INVENTORY_FAILURE;
		return 0;
	}

	char *endptr;
	errno = 0;

	size_t value = strtoul(str, &endptr, 10);

	if (errno != 0 || *endptr != '\0' || str == endptr) {
		*status = INVENTORY_FAILURE;
		return 0;
	}

	return value;
}

equipment *equipment_new(char const *id, char const *name, struct quantity qty) {
	if (id == (void *) 0 || name == (void *) 0) {
		return (void *) 0;
	}

	struct equipment *e = malloc(sizeof(struct equipment));

	if (e == (void *) 0) {
		return (void *) 0;
	}

	e->id = strdup(id);

	if (e->id == (void *) 0) {
		free(e);
		return (void *) 0;
	}

	e->name = strdup(name);

	if (e->name == (void *) 0) {
		free(e->id);
		free(e);
		return (void *) 0;
	}

	e->qty = qty;

	return e;
}

void equipment_del(equipment *e) {
	if (e == (void *) 0) {
		return;
	}

	assert(e->id != (void *) 0);
	assert(e->name != (void *) 0);

	free(e->id);
	free(e->name);
	free(e);
}

char const *equipment_get_id(equipment *self) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	return self->id;
}

char const *equipment_get_name(equipment *self) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	return self->name;
}

quantity equipment_get_quantity(equipment *self) {
	if (self == (void *) 0) {
		return (quantity) { 0 };
	}

	return self->qty;
}

void equipment_set_quantity(equipment *self, quantity qty) {
	if (self == (void *) 0) {
		return;
	}

	if (qty.available > 0 || qty.borrowed > 0 || qty.in_maintenance > 0) {
		self->qty = qty;
	}

	return;
}

inventory *inventory_new(void) {
	struct inventory *self = malloc(sizeof(struct inventory));
	if (self == (void *) 0) {
		return (void *) 0;
	}

	self->items = array_new();

	if (self->items == (void *) 0) {
		free(self);
		return (void *) 0;
	}

	return self;
}

void inventory_del(inventory *inv) {
	if (inv == (void *) 0) {
		return;
	}

	inventory_clear_contents(inv);

	array_del(inv->items);

	free(inv);
}

equipment *inventory_at(inventory *self, size_t index) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	assert(self->items != (void *) 0);

	result res = array_at(self->items, index);
	if (res.error != (void *) 0) {
		free(res.error);
		return (void *) 0;
	}

	return res.p;
}

equipment *inventory_append(inventory *self, equipment *e) {
	if (self == (void *) 0 || e == (void *) 0) {
		return (void *) 0;
	}

	assert(self->items != (void *) 0);

	result res = array_append(self->items, e);

	if (res.error != (void *) 0) {
		free(res.error);
		return (void *) 0;
	}

	return res.p;
}

equipment *inventory_remove(inventory *self, size_t index) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	assert(self->items != (void *) 0);

	result res = array_remove(self->items, index);

	if (res.error != (void *) 0) {
		free(res.error);
		return (void *) 0;
	}

	return res.p;
}

size_t inventory_len(inventory *self) {
	if (self == (void *) 0) {
		return 0;
	}

	assert(self->items != (void *) 0);

	return array_len(self->items);
}

int inventory_export(inventory *self, FILE *f) {
	if (self == (void *) 0 || f == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	assert(self->items != (void *) 0);

	row *header_row = array_new();

	if (header_row == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	if (append_cell_str_dup(header_row, "id") || append_cell_str_dup(header_row, "name") ||
	    append_cell_str_dup(header_row, "available") ||
	    append_cell_str_dup(header_row, "borrowed") ||
	    append_cell_str_dup(header_row, "in_maintenance")) {
		tsv_clear_row(header_row);
		return INVENTORY_FAILURE;
	}

	tsv *tmp = tsv_new(header_row);

	if (tmp == (void *) 0) {
		// if tsv_new fails, it frees header_row for us
		return INVENTORY_FAILURE;
	}

	size_t inv_len = inventory_len(self);

	for (size_t i = 0; i < inv_len; ++i) {
		equipment *eq = inventory_at(self, i);

		if (eq == (void *) 0) {
			tsv_del(tmp);
			return INVENTORY_FAILURE;
		}

		row *row_data = array_new();

		if (row_data == (void *) 0) {
			tsv_del(tmp);
			return INVENTORY_FAILURE;
		}

		char *avl_str = size_t_to_str(eq->qty.available);
		char *bor_str = size_t_to_str(eq->qty.borrowed);
		char *imt_str = size_t_to_str(eq->qty.in_maintenance);

		if (append_cell_str_dup(row_data, eq->id) ||
		    append_cell_str_dup(row_data, eq->name) ||
		    append_cell_str_move(row_data, avl_str) ||
		    append_cell_str_move(row_data, bor_str) ||
		    append_cell_str_move(row_data, imt_str)) {
			tsv_clear_row(row_data);
			tsv_del(tmp);

			return INVENTORY_FAILURE;
		}

		row *appended_row = tsv_append(tmp, row_data);
		if (appended_row == (void *) 0) {
			tsv_clear_row(row_data);
			tsv_del(tmp);
			return INVENTORY_FAILURE;
		}
	}

	tsv_export_file(tmp, f);

	tsv_del(tmp);

	return INVENTORY_SUCCESS;
}

int inventory_import(inventory *self, FILE *f) {
	if (self == (void *) 0 || f == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	tsv *imported_tsv = tsv_import_file(f);

	if (imported_tsv == (void *) 0) {
		return INVENTORY_FAILURE;
	}

	if (header_field_differs(imported_tsv, 0, "id") ||
	    header_field_differs(imported_tsv, 1, "name") ||
	    header_field_differs(imported_tsv, 2, "available") ||
	    header_field_differs(imported_tsv, 3, "borrowed") ||
	    header_field_differs(imported_tsv, 4, "in_maintenance")) {
		tsv_del(imported_tsv);
		return INVENTORY_FAILURE;
	}

	inventory_clear_contents(self);

	size_t rows_len = tsv_len(imported_tsv);
	int status = INVENTORY_SUCCESS;

	for (size_t i = 0; i < rows_len; ++i) {
		row *row_data = tsv_at(imported_tsv, i);

		if (row_data == (void *) 0) {
			status = INVENTORY_FAILURE;
			break;
		}

		if (array_len(row_data) != 5) {
			status = INVENTORY_FAILURE;
			break;
		}

		char *id_str = get_cell_content(&status, row_data, 0);
		char *name_str = get_cell_content(&status, row_data, 1);
		char *avail_str = get_cell_content(&status, row_data, 2);
		char *borr_str = get_cell_content(&status, row_data, 3);
		char *in_maint_str = get_cell_content(&status, row_data, 4);

		size_t available = str_to_size_t(&status, avail_str);
		size_t borrowed = str_to_size_t(&status, borr_str);
		size_t in_maintenance = str_to_size_t(&status, in_maint_str);

		quantity qty = { .available = available,
				 .borrowed = borrowed,
				 .in_maintenance = in_maintenance };

		equipment *new_eq = equipment_new(id_str, name_str, qty);

		if (new_eq == (void *) 0) {
			status = INVENTORY_FAILURE;
			break;
		}

		equipment *appended_eq = inventory_append(self, new_eq);

		if (appended_eq == (void *) 0) {
			equipment_del(new_eq);
			status = INVENTORY_FAILURE;
			break;
		}
	}

	tsv_del(imported_tsv);

	if (status != INVENTORY_SUCCESS) {
		inventory_clear_contents(self);
	}

	return status;
}
