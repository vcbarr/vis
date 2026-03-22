#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "cursor.h"

struct cursor {
	size_t pos;
	size_t cap;
};

struct cursor *cursor_new(void) {
	struct cursor *c = malloc(sizeof(struct cursor));

	if (c != (void *) 0) {
		c->cap = 0;
		c->pos = 0;
	}

	return c;
}

void cursor_del(struct cursor *c) {
	if (c == (void *) 0) {
		return;
	}

	free(c);
}

void cursor_move_up(cursor *self) {
	if (self == (void *) 0) {
		return;
	}

	if (self->cap == 0) {
		assert(self->pos == 0);
		return;
	}

	if (self->pos > 0) {
		self->pos--;
	}
}

void cursor_move_down(cursor *self) {
	if (self == (void *) 0) {
		return;
	}

	if (self->cap == 0) {
		assert(self->pos == 0);
		return;
	}

	if (self->pos < self->cap - 1) {
		self->pos++;
	}
}

size_t cursor_get_pos(cursor *self) {
	if (self == (void *) 0) {
		return 0;
	}

	return self->pos;
}

void cursor_set_pos(cursor *self, size_t pos) {
	if (self == (void *) 0) {
		return;
	}

	if (self->cap == 0) {
		self->pos = 0;
		return;
	}

	if (pos >= self->cap) {
		self->pos = self->cap - 1;
		return;
	}

	self->pos = pos;
}

void cursor_set_cap(cursor *self, size_t cap) {
	if (self == (void *) 0) {
		return;
	}

	if (cap == 0) {
		self->cap = 0;
		self->pos = 0;
		return;
	}

	if (self->cap > cap) {
		self->pos = cap - 1;
	}

	self->cap = cap;

	return;
}
