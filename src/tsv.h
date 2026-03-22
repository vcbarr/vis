#ifndef TSV_H
#define TSV_H 1

#include <stdio.h>
#include <stdlib.h>

#include "array.h"

typedef array row;

typedef struct tsv tsv;

tsv *tsv_new(row *header);
void tsv_del(tsv *t);

void tsv_clear_row(row *row);

row *tsv_at(tsv *self, size_t index);
row *tsv_header(tsv *self);
row *tsv_append(tsv *self, row *row);
row *tsv_remove(tsv *self, size_t index);

size_t tsv_len(tsv *self);

tsv *tsv_import_file(FILE *f);
void tsv_export_file(tsv *t, FILE *f);

#endif // TSV_H
