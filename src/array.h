#ifndef ARRAY_H
#define ARRAY_H 1

#include <stdlib.h>

typedef struct result {
	void *p;
	char *error;
} result;

typedef struct array array;

array *array_new(void);
void array_del(array *self);

result array_at(array *self, size_t index);
result array_append(array *self, void *p);
result array_remove(array *self, size_t index);

size_t array_len(array *self);
size_t array_cap(array *self);

#endif // ARRAY_H
