# vis - Vim Inventory System

![vis](images/logo.png)

> vis is a high-performance terminal-based inventory management system developed as a *Systems Programming Project* using the C11 standard. Designed specifically for university laboratory environments, it streamlines the tracking, lending, and maintenance of sensitive hardware such as oscilloscopes, multimeters, microcontrollers, etc.
> **This project served as the final summative evaluation for Programação Estruturada at the Federal University of Piauí (UFPI), where it was defended through a comprehensive code review focusing on memory safety, logic flow, and C11 implementation standards.**

`vis` é um sistema que facilita o gerenciamento de equipamentos como osciloscópios, fontes,
multímetros, placas Arduino, dentre outros, que são emprestados para alunos realizarem experimentos.

Com um menu interativo para cadastrar novos equipamentos, consultar um equipamento específico,
listar todos os equipamentos, e com funcionalidade de backup que permite importar ou exportar um
backup de equipamentos já cadastrados.

O padrão utilizado é C11, então você precisará de um compilador que implemente pelo menos C11.

Do próprio menu de ajuda:

```
Sumário:

	O 'vis' (vim inventory system) é uma ferramenta de gerenciamento de inventário
	interativa, operando através de comandos de teclado. Ele exibe uma lista de
	itens do inventário, com um cursor ('>') indicando o item selecionado. A maioria
	das operações é aplicada ao item atualmente selecionado pelo cursor. Para mover
	o cursor, utilize 'j' (baixo) e 'k' (cima).

Comandos:

	a : Adicionar novo equipamento ao inventário
	d : Remover equipamento selecionado do inventário
	b : Emprestar equipamento
	r : Devolver equipamento emprestado
	m : Enviar equipamento para manutenção
	M : Retornar equipamento da manutenção
	j : Mover cursor para baixo
	k : Mover cursor para cima
	p : Exibir visão atual do inventário
	P : Exibir todo o inventário
	w : Salvar inventário
	W : Carregar inventário de um arquivo
	q : Sair da aplicação
	? : Exibir este menu de ajuda
```

## Compilando e Executando

### Requisitos de Sistema

- Sistema Operacional Linux/UNIX
- Compilador com suporte a pelo menos C11
- `make`
- `coreutils` (para `make`)

### Compilação

Para compilar, basta entrar no diretório `src` e executar o comando.

```
$ make
```

### Execução

Execute o binário compilado e se divirta! :)

```
$ ./vis
```

Há um arquivo em `examples/` chamado `exemplo.tsv`. Abra-o com o programa:

```
$ ./vis exemplo.tsv
```

## Copyright

Veja `LICENSE`.
