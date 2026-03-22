#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "array.h"

#define BUFSIZE 8

struct array {
	void **data;
	size_t len;
	size_t cap;
};

struct array *array_new(void) {
	struct array *self = malloc(sizeof(struct array));

	if (self == (void *) 0) {
		return (void *) 0;
	}

	self->data = malloc(sizeof(void *) * BUFSIZE);

	if (self->data == (void *) 0) {
		free(self);
		return (void *) 0;
	}

	self->len = 0;
	self->cap = BUFSIZE;

	return self;
}

void array_del(struct array *self) {
	if (self == (void *) 0) {
		return;
	}

	assert(self->data != (void *) 0);

	free(self->data);
	free(self);
}

result array_at(struct array *self, size_t index) {
	if (self == (void *) 0) {
		return (result) {
			.p = (void *) 0,
			.error = strdup("array is null"),
		};
	}

	assert(self->data != (void *) 0);

	if (index >= self->len) {
		return (result) {
			.p = (void *) 0,
			.error = strdup("index out of bounds"),
		};
	}

	return (result) { .p = self->data[index], .error = (void *) 0 };
}

result array_append(struct array *self, void *p) {
	if (self == (void *) 0) {
		return (result) { .p = (void *) 0, .error = strdup("array is null") };
	}

	if (self->len == self->cap) {
		size_t new_capacity = BUFSIZE;

		if (self->cap != 0) {
			new_capacity = self->cap * 2;
		}

		size_t new_size = sizeof(void *) * new_capacity;

		void **tmp = realloc(self->data, new_size);

		if (tmp == (void *) 0) {
			return (result) { .p = (void *) 0,
					  .error = strdup("failed to reallocate memory") };
		};

		self->data = tmp;
		self->cap = new_capacity;
	}

	self->data[self->len] = p;
	self->len++;

	return (result) { .p = (void *) p, .error = (void *) 0 };
}

result array_remove(struct array *self, size_t index) {
	if (self == (void *) 0) {
		return (result) {
			.p = (void *) 0,
			.error = strdup("array is null"),
		};
	}

	assert(self->data != (void *) 0);

	if (index >= self->len) {
		return (result) {
			.p = (void *) 0,
			.error = strdup("index out of bounds"),
		};
	}

	void *removed_element = self->data[index];

	if (index < self->len - 1) {
		memmove(&self->data[index], &self->data[index + 1],
			(self->len - 1 - index) * sizeof(void *));
	}

	self->len--;

	return (result) { .p = removed_element, .error = (void *) 0 };
}

size_t array_len(struct array *self) {
	if (self == (void *) 0) {
		return 0;
	}

	return self->len;
}

size_t array_cap(struct array *self) {
	if (self == (void *) 0) {
		return 0;
	}

	return self->cap;
}
