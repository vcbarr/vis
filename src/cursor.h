#ifndef CURSOR_H
#define CURSOR_H 1

#include <stdlib.h>

typedef struct cursor cursor;

cursor *cursor_new(void);
void cursor_del(cursor *c);

void cursor_move_up(cursor *self);
void cursor_move_down(cursor *self);
size_t cursor_get_pos(cursor *self);
void cursor_set_pos(cursor *self, size_t index);
void cursor_set_cap(cursor *self, size_t cap);

#endif // CURSOR_H
