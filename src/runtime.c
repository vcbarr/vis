#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

#include "cursor.h"
#include "inventory.h"

struct runtime {
	inventory *inv;
	cursor *cur;
	char *current_filepath;
	bool modified;
};

static size_t codepoints_byte_delta(char const *s);
static bool file_exists(char const *s);
static equipment *get_current_equipment(runtime *self);
static bool invalid_quantity(quantity qty);
static void print_inventory_table_window(runtime *self, size_t start_index, size_t end_index);
static char *prompt(char const *msg);
static size_t prompt_size_t(char const *msg);
static bool prompt_yes_no(char const *msg);
static size_t shift_values(size_t *from, size_t *to, size_t count);
static size_t utf8_strlen_codepoints(const char *s);

static size_t codepoints_byte_delta(char const *s) {
	return strlen(s) - utf8_strlen_codepoints(s);
}

static bool file_exists(char const *s) {
	FILE *f = fopen(s, "r");

	if (f != (void *) 0) {
		fclose(f);
		return true;
	}

	return false;
}

static equipment *get_current_equipment(runtime *self) {
	if (self == (void *) 0) {
		return (void *) 0;
	}

	if (inventory_len(self->inv) == 0) {
		return (void *) 0;
	}

	equipment *e = inventory_at(self->inv, cursor_get_pos(self->cur));

	if (e == (void *) 0) {
		fprintf(stderr, "erro: não foi possível acessar o equipamento\n");
	}

	return e;
}

static bool invalid_quantity(quantity qty) {
	return !(qty.available > 0 || qty.borrowed > 0 || qty.in_maintenance > 0);
}

static void print_inventory_table_window(runtime *self, size_t start_index, size_t end_index) {
	assert(self != (void *) 0);

	size_t cursor_pos = cursor_get_pos(self->cur);

	size_t max_id_width = utf8_strlen_codepoints("ID");
	size_t max_name_width = utf8_strlen_codepoints("Nome");

	const size_t min_qty_col_width = 7;

	for (size_t i = start_index; i <= end_index; ++i) {
		equipment *e = inventory_at(self->inv, i);

		if (e == (void *) 0) {
			fprintf(stderr, "erro: não foi possível acessar o equipamento\n");
			continue;
		}

		size_t id_len = utf8_strlen_codepoints(equipment_get_id(e));

		if (id_len > max_id_width) {
			max_id_width = id_len;
		}
		size_t name_len = utf8_strlen_codepoints(equipment_get_name(e));
		if (name_len > max_name_width) {
			max_name_width = name_len;
		}
	}

	max_id_width += 2;
	max_name_width += 2;

	max_id_width = max_id_width > utf8_strlen_codepoints("ID") ? max_id_width :
								     utf8_strlen_codepoints("ID");
	max_name_width = max_name_width > utf8_strlen_codepoints("Nome") ?
				 max_name_width :
				 utf8_strlen_codepoints("Nome");

	printf("  %-*s %-*s %-*s %-*s %-*s\n", (int) max_id_width, "ID", (int) max_name_width,
	       "Nome", (int) min_qty_col_width, "Disp.", (int) min_qty_col_width, "Emp.",
	       (int) min_qty_col_width, "Manut.");

	printf("  ");

	for (size_t k = 0; k < max_id_width; ++k) {
		printf("-");
	}

	printf(" ");

	for (size_t k = 0; k < max_name_width; ++k) {
		printf("-");
	}

	printf(" ");

	for (size_t k = 0; k < min_qty_col_width; ++k) {
		printf("-");
	}

	printf(" ");

	for (size_t k = 0; k < min_qty_col_width; ++k) {
		printf("-");
	}

	printf(" ");

	for (size_t k = 0; k < min_qty_col_width; ++k) {
		printf("-");
	}

	printf("\n");

	for (size_t i = start_index; i <= end_index; ++i) {
		equipment *e = inventory_at(self->inv, i);
		if (e == (void *) 0) {
			fprintf(stderr, "erro: não foi possível acessar o equipamento\n");
			continue;
		}

		bool is_current = (i == cursor_pos);
		quantity qty = equipment_get_quantity(e);

		char const *id = equipment_get_id(e);
		char const *name = equipment_get_name(e);

		printf("%s %-*s %-*s %-*zu %-*zu %-*zu\n",
		       is_current ? ">" : " ", // cursor
		       (int) (max_id_width + codepoints_byte_delta(id)), id,
		       (int) (max_name_width + codepoints_byte_delta(name)), name,
		       (int) min_qty_col_width, qty.available, (int) min_qty_col_width,
		       qty.borrowed, (int) min_qty_col_width, qty.in_maintenance);
	}
}

static char *prompt(char const *msg) {
	printf("%s", msg);

	int cap = 128;

	char *buf = malloc(cap * sizeof(char));

	if (buf == (void *) 0) {
		return (void *) 0;
	}

	int len = 0;
	int c;

	while ((c = getchar()) != EOF && c != '\n') {
		if (len >= cap - 1) {
			cap *= 2;
			char *tmp = realloc(buf, cap * sizeof(char));

			if (tmp == (void *) 0) {
				free(buf);
				return (void *) 0;
			}

			buf = tmp;
		}

		buf[len++] = c;
	}

	buf[len] = '\0';

	if (len == 0 && c == EOF) {
		free(buf);
		return (void *) 0;
	}

	char *shrunk_buf = realloc(buf, (len + 1) * sizeof(char));

	if (shrunk_buf == (void *) 0) {
		free(buf);
		return (void *) 0;
	}

	return shrunk_buf;
}

static size_t prompt_size_t(char const *msg) {
	char *val_str = prompt(msg);

	if (val_str == (void *) 0 || strlen(val_str) == 0) {
		fprintf(stderr, "erro: quantidade inválida\n");
		free(val_str);
		return 0;
	}

	size_t val;

	if (sscanf(val_str, "%zu", &val) != 1) {
		fprintf(stderr, "erro: sscanf: não foi possível parsear o valor\n");
		free(val_str);
		return 0;
	}

	free(val_str);

	return val;
}

static bool prompt_yes_no(char const *msg) {
	char *answer = prompt(msg);

	if (answer == (void *) 0) {
		return false;
	}

	if (strcmp(answer, "s") == 0 || strcmp(answer, "S") == 0) {
		free(answer);
		return true;
	}

	free(answer);
	return false;
}

static size_t shift_values(size_t *from, size_t *to, size_t count) {
	if (count > *from) {
		fprintf(stderr, "erro: não há equipamento suficiente para a operação\n");
		return 0;
	}

	*from -= count;
	*to += count;

	return count;
}

static size_t utf8_strlen_codepoints(const char *s) {
	size_t count = 0;
	while (*s) {
		if ((*s & 0xC0) != 0x80) { // If it's not a continuation byte
			count++;
		}
		s++;
	}
	return count;
}

runtime *runtime_new(char const *filename) {
	runtime *self = malloc(sizeof(struct runtime));

	if (self == (void *) 0) {
		fprintf(stderr, "erro: malloc: %s\n", strerror(errno));
		return (void *) 0;
	}

	self->inv = inventory_new();

	if (self->inv == (void *) 0) {
		fprintf(stderr, "erro: não foi possível criar o inventário\n");
		free(self);
		return (void *) 0;
	}

	self->cur = cursor_new();

	if (self->cur == (void *) 0) {
		fprintf(stderr, "erro: não foi possível criar o cursor\n");
		inventory_del(self->inv);
		free(self);
		return (void *) 0;
	}

	self->current_filepath = (void *) 0;
	self->modified = false;

	if (filename != (void *) 0) {
		self->current_filepath = strdup(filename);

		if (self->current_filepath == (void *) 0) {
			fprintf(stderr, "erro: strdup: %s\n", strerror(errno));
			cursor_del(self->cur);
			inventory_del(self->inv);
			free(self);
			return (void *) 0;
		}

		FILE *f = fopen(self->current_filepath, "r");

		if (f == (void *) 0) {
			fprintf(stderr, "erro: não foi possível abrir %s: %s\n",
				self->current_filepath, strerror(errno));
			cursor_del(self->cur);
			inventory_del(self->inv);
			free(self);
			return (void *) 0;
		}

		int status = inventory_import(self->inv, f);
		fclose(f);

		if (status != INVENTORY_SUCCESS) {
			fprintf(stderr,
				"erro: não foi possível carregar os dados"
				"do arquivo %s\n",
				self->current_filepath);

			free(self->current_filepath);
			cursor_del(self->cur);
			inventory_del(self->inv);
			free(self);

			return (void *) 0;
		}
	}

	cursor_set_cap(self->cur, inventory_len(self->inv));
	cursor_set_pos(self->cur, 0);

	if (self->current_filepath != (void *) 0) {
		printf("info: %s carregado com sucesso\n", self->current_filepath);
	} else {
		printf("info: guardando informações em memória\n");
	}
	printf("info: %zu entradas, cursor em %zu\n", inventory_len(self->inv),
	       cursor_get_pos(self->cur));

	return self;
}

void runtime_del(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	inventory_del(self->inv);
	cursor_del(self->cur);

	if (self->current_filepath != (void *) 0) {
		free(self->current_filepath);
		self->current_filepath = (void *) 0;
	}

	free(self);
}

void runtime_equipment_add(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	printf("info: adicionando novo equipamento\n");

	char *id_str = prompt(">> ID: ");

	if (id_str == (void *) 0) {
		fprintf(stderr, "erro: id inválido\n");

		return;
	}

	char *name_str = prompt(">> Nome: ");

	if (name_str == (void *) 0) {
		free(id_str);

		fprintf(stderr, "erro: nome inválido\n");

		return;
	}

	quantity qty = (quantity) {
		.available = prompt_size_t(">> Quantidade Disponível: "),
		.borrowed = prompt_size_t(">> Quantidade Emprestada: "),
		.in_maintenance = prompt_size_t(">> Quantidade em Manutenção: "),
	};

	if (invalid_quantity(qty)) {
		fprintf(stderr, "erro: precisa-se ter pelo menos um equipamento para adicionar\n");
		return;
	}

	equipment *e = equipment_new(id_str, name_str, qty);

	free(id_str);
	free(name_str);

	if (e == (void *) 0) {
		fprintf(stderr, "erro: não foi possível criar o equipamento\n");
		return;
	}

	if (inventory_append(self->inv, e) != e) {
		equipment_del(e);
		fprintf(stderr, "erro: não foi possível adicionar o equipamento ao inventário\n");
		return;
	}

	cursor_set_cap(self->cur, inventory_len(self->inv));

	printf("info: equipamento adicionado com sucesso\n");

	self->modified = true;
}

void runtime_equipment_remove(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	size_t total_items = inventory_len(self->inv);

	if (total_items == 0) {
		printf("info: sem equipamentos no inventório\n");
		return;
	}

	size_t current_pos = cursor_get_pos(self->cur);

	equipment *removed_equipment = inventory_remove(self->inv, current_pos);

	if (removed_equipment == (void *) 0) {
		fprintf(stderr, "erro: não foi possível remover o equipamento\n");
		return;
	}

	cursor_set_cap(self->cur, inventory_len(self->inv));

	runtime_inventory_print_current(self);

	self->modified = true;
}

void runtime_equipment_borrow(runtime *self) {
	equipment *e = get_current_equipment(self);

	if (e == (void *) 0) {
		return;
	}

	quantity qty = equipment_get_quantity(e);

	size_t count = prompt_size_t(">> Quantos equipamentos serão emprestados? ");

	if (shift_values(&qty.available, &qty.borrowed, count) != count) {
		return;
	}

	equipment_set_quantity(e, qty);

	self->modified = true;
}

void runtime_equipment_return_borrowed(runtime *self) {
	equipment *e = get_current_equipment(self);

	if (e == (void *) 0) {
		return;
	}

	quantity qty = equipment_get_quantity(e);

	size_t count = prompt_size_t(">> Quantos equipamentos serão retornados? ");

	if (shift_values(&qty.borrowed, &qty.available, count) != count) {
		return;
	}

	equipment_set_quantity(e, qty);

	self->modified = true;
}

void runtime_equipment_fix(runtime *self) {
	equipment *e = get_current_equipment(self);

	if (e == (void *) 0) {
		return;
	}

	quantity qty = equipment_get_quantity(e);

	size_t count = prompt_size_t(">> Quantos equipamentos serão enviados para reparo? ");

	if (shift_values(&qty.available, &qty.in_maintenance, count) != count) {
		return;
	}

	equipment_set_quantity(e, qty);

	self->modified = true;
}

void runtime_equipment_return_fixed(runtime *self) {
	equipment *e = get_current_equipment(self);

	if (e == (void *) 0) {
		return;
	}

	quantity qty = equipment_get_quantity(e);

	size_t count = prompt_size_t(">> Quantos equipamentos serão retornados? ");

	if (shift_values(&qty.in_maintenance, &qty.available, count) != count) {
		return;
	}

	equipment_set_quantity(e, qty);

	self->modified = true;
}

void runtime_cursor_down(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	if (inventory_len(self->inv) == 0) {
		printf("info: sem equipamentos no inventório\n");
		return;
	}

	cursor_move_down(self->cur);
	runtime_inventory_print_current(self);
}

void runtime_cursor_up(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	if (inventory_len(self->inv) == 0) {
		printf("info: sem equipamentos no inventório\n");
		return;
	}

	cursor_move_up(self->cur);
	runtime_inventory_print_current(self);
}

void runtime_inventory_print_current(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	size_t total_items = inventory_len(self->inv);

	if (total_items == 0) {
		printf("info: sem equipamentos no inventório\n");
		return;
	}

	size_t cursor_pos = cursor_get_pos(self->cur);

	size_t start_index;
	size_t end_index;
	const size_t window_size = 5;

	if (total_items <= window_size) {
		start_index = 0;
		end_index = total_items - 1;
	} else {
		if (cursor_pos < window_size / 2) {
			start_index = 0;
		} else if (cursor_pos >= total_items - (window_size / 2 + (window_size % 2))) {
			start_index = total_items - window_size;
		} else {
			start_index = cursor_pos - window_size / 2;
		}
		end_index = start_index + window_size - 1;
	}

	print_inventory_table_window(self, start_index, end_index);
}

void runtime_inventory_print_all(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	size_t total_items = inventory_len(self->inv);

	if (total_items == 0) {
		printf("info: sem equipamentos no inventório\n");
		return;
	}

	size_t start_index = 0;
	size_t end_index = total_items - 1;

	print_inventory_table_window(self, start_index, end_index);
}

void runtime_inventory_save(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	if (!self->modified) {
		return;
	}

	char *target = self->current_filepath;
	bool prompted_new_path = false;

	if (target == (void *) 0) {
		prompted_new_path = true;

		target = prompt(">> Salvar como: ");

		if (target == (void *) 0) {
			fprintf(stderr, "erro: nome inválido\n");
			return;
		}

		if (strcmp(target, "") == 0) {
			fprintf(stderr, "erro: nome vazio\n");
			if (prompted_new_path) {
				free(target);
			}
			return;
		}
	}

	if (file_exists(target)) {
		char const *msg = ">> O arquivo já existe. "
				  "Deseja sobreescrevê-lo? [s/n] ";
		if (!prompt_yes_no(msg)) {
			fprintf(stderr, "aviso: abortando\n");
			if (prompted_new_path) {
				free(target);
			}
			return;
		}
	}

	FILE *f = fopen(target, "w");

	if (f == (void *) 0) {
		char const *fmt = "erro: não foi possível abrir o arquivo '%s': %s\n";

		fprintf(stderr, fmt, target, strerror(errno));

		if (prompted_new_path) {
			free(target);
		}

		return;
	}

	int status = inventory_export(self->inv, f);

	fclose(f);

	if (status != INVENTORY_SUCCESS) {
		char const *fmt = "erro: falha ao salvar o inventário em '%s'.\n";

		fprintf(stderr, fmt, target);

		if (prompted_new_path) {
			free(target);
		}

		return;
	}

	printf("info: inventário salvo com sucesso em '%s'\n", target);
	self->modified = false;

	if (prompted_new_path) {
		self->current_filepath = target;
	}
}

void runtime_inventory_load(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	if (self->modified) {
		char const *msg = ">> Você tem alterações não salvas. "
				  "Deseja continuar e descartá-las? [s/n]: ";

		if (!prompt_yes_no(msg)) {
			printf("aviso: abortando\n");
			return;
		}
	}

	char *filename_to_load = prompt(">> Abrir arquivo: ");

	if (filename_to_load == (void *) 0 || strlen(filename_to_load) == 0) {
		fprintf(stderr, "erro: arquivo inválido\n");
		fprintf(stderr, "aviso: abortando\n");

		free(filename_to_load);

		return;
	}

	FILE *f = fopen(filename_to_load, "r");

	if (f == (void *) 0) {
		fprintf(stderr, "erro: não foi possível abrir o arquivo '%s': %s\n",
			filename_to_load, strerror(errno));

		free(filename_to_load);

		return;
	}

	inventory *tmp = inventory_new();

	if (tmp == (void *) 0) {
		fprintf(stderr, "erro: não foi possível criar o inventário\n");
		fclose(f);
		free(filename_to_load);
		return;
	}

	int status = inventory_import(tmp, f);

	fclose(f);
	if (status != INVENTORY_SUCCESS) {
		fprintf(stderr, "erro: Falha ao carregar o inventário de '%s'.\n",
			filename_to_load);
		free(filename_to_load);
		inventory_del(tmp);
		return;
	}

	inventory_del(self->inv);
	self->inv = tmp;

	cursor_set_cap(self->cur, inventory_len(self->inv));
	cursor_set_pos(self->cur, 0);

	if (self->current_filepath != (void *) 0) {
		free(self->current_filepath);
	}

	self->current_filepath = filename_to_load;

	self->modified = false;

	printf("info: %zu entradas, cursor em %zu\n", inventory_len(self->inv),
	       cursor_get_pos(self->cur));

	runtime_inventory_print_current(self);
}

void runtime_halt(runtime *self) {
	if (self == (void *) 0) {
		return;
	}

	if (self->modified) {
		printf("aviso: você tem alterações não salvas\n");
		char const *msg = ">> Deseja salvar as alterações antes de sair? [s/n] ";

		if (prompt_yes_no(msg)) {
			runtime_inventory_save(self);
		} else {
			printf("info: descartando alterações não salvas\n");
		}
	}

	printf("info: encerrando a aplicação\n");
}
