#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "runtime.h"

char const *PROGRAM_NAME = (void *) 0;

static char const *HELP_MENU =
	"\nSumário:\n\n"
	"\tO 'vis' (vim inventory system) é uma ferramenta de gerenciamento de inventário\n"
	"\tinterativa, operando através de comandos de teclado. Ele exibe uma lista de\n"
	"\titens do inventário, com um cursor ('>') indicando o item selecionado. A maioria\n"
	"\tdas operações é aplicada ao item atualmente selecionado pelo cursor. Para mover\n"
	"\to cursor, utilize 'j' (baixo) e 'k' (cima).\n"
	"\nComandos:\n\n"
	"\ta : Adicionar novo equipamento ao inventário\n"
	"\td : Remover equipamento selecionado do inventário\n"
	"\tb : Emprestar equipamento\n"
	"\tr : Devolver equipamento emprestado\n"
	"\tm : Enviar equipamento para manutenção\n"
	"\tM : Retornar equipamento da manutenção\n"
	"\tj : Mover cursor para baixo\n"
	"\tk : Mover cursor para cima\n"
	"\tp : Exibir visão atual do inventário\n"
	"\tP : Exibir todo o inventário\n"
	"\tw : Salvar inventário\n"
	"\tW : Carregar inventário de um arquivo\n"
	"\tq : Sair da aplicação\n"
	"\t? : Exibir este menu de ajuda\n\n";

void linebuf_disable(void) {
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void linebuf_enable(void) {
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= (ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int main(int argc, char const **argv) {
	atexit(linebuf_enable);
	linebuf_disable();

	char const *filename = (void *) 0;

	PROGRAM_NAME = argv[0];

	if (argc > 2) {
		fprintf(stderr, "usage: %s [inventory file]\n", PROGRAM_NAME);
		return EXIT_FAILURE;
	}

	if (argc == 2) {
		filename = argv[1];
	}

	bool running = true;
	runtime *r = runtime_new(filename);

	printf("vis - vim inventory system\n");
	printf("~ ");

	while (running) {
		int c = fgetc(stdin);

		printf("%c\n", c);

		linebuf_enable();

		switch (c) {
		case 'a':
			runtime_equipment_add(r);
			break;
		case 'b':
			runtime_equipment_borrow(r);
			break;
		case 'r':
			runtime_equipment_return_borrowed(r);
			break;
		case 'm':
			runtime_equipment_fix(r);
			break;
		case 'M':
			runtime_equipment_return_fixed(r);
			break;
		case 'd':
			runtime_equipment_remove(r);
			break;
		case 'p':
			runtime_inventory_print_current(r);
			break;
		case 'P':
			runtime_inventory_print_all(r);
			break;
		case 'j':
			runtime_cursor_down(r);
			break;
		case 'k':
			runtime_cursor_up(r);
			break;
		case 'h':
		case 'l':
			fprintf(stderr, "%s: erro: comando %c: não implementado ainda\n",
				PROGRAM_NAME, c);
			break;
		case 'w':
			runtime_inventory_save(r);
			break;
		case 'W':
			runtime_inventory_load(r);
			break;
		case EOF:
		case '':
		case 'q':
			runtime_halt(r);
			running = false;
			break;
		case '?':
			printf("%s", HELP_MENU);
			break;
		default:
			fprintf(stderr, "%s: erro: comando não implementado\n", PROGRAM_NAME);
			fprintf(stderr, "pressione ? para ajuda\n");
			break;
		}

		if (running) {
			printf("~ ");
		}

		linebuf_disable();
	}

	runtime_del(r);

	return EXIT_SUCCESS;
}
