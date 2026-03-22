#ifndef RUNTIME_H
#define RUNTIME_H 1

#include <stdlib.h>

typedef struct runtime runtime;

runtime *runtime_new(char const *filename);
void runtime_del(runtime *self);

void runtime_equipment_add(runtime *self);
void runtime_equipment_remove(runtime *self);

void runtime_equipment_borrow(runtime *self);
void runtime_equipment_return_borrowed(runtime *self);
void runtime_equipment_fix(runtime *self);
void runtime_equipment_return_fixed(runtime *self);

void runtime_cursor_down(runtime *self);
void runtime_cursor_up(runtime *self);

void runtime_inventory_save(runtime *self);
void runtime_inventory_load(runtime *self);

void runtime_inventory_print_all(runtime *self);
void runtime_inventory_print_current(runtime *self);

void runtime_halt(runtime *self);

#endif // RUNTIME_H
