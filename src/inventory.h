#ifndef INVENTORY_H
#define INVENTORY_H 1

#include <stdio.h>
#include <stdlib.h>

#include "array.h"

typedef struct quantity {
	size_t available;
	size_t borrowed;
	size_t in_maintenance;
} quantity;

typedef struct equipment equipment;

typedef struct inventory inventory;

equipment *equipment_new(char const *id, char const *name, quantity qty);
void equipment_del(equipment *e);

char const *equipment_get_id(equipment *self);
char const *equipment_get_name(equipment *self);
quantity equipment_get_quantity(equipment *self);

void equipment_set_quantity(equipment *self, quantity qty);

inventory *inventory_new(void);
void inventory_del(inventory *inv);

equipment *inventory_at(inventory *self, size_t index);
equipment *inventory_append(inventory *self, equipment *e);
equipment *inventory_remove(inventory *self, size_t index);

size_t inventory_len(inventory *self);

// status codes for import and export functions
enum {
	INVENTORY_SUCCESS = 0,
	INVENTORY_FAILURE = -1,
};

int inventory_export(inventory *self, FILE *f);
int inventory_import(inventory *self, FILE *f);

#endif // INVENTORY_H
