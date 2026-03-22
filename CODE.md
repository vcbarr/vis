# Estilo de Código

Este documento define as convenções e o estilo de código utilizados neste projeto. O objetivo é
garantir consistência e legibilidade, facilitando a manutenção e a colaboração. O estilo é
fortemente influenciado pelas [Linux Kernel Coding Style](https://www.kernel.org/doc/html/latest/process/coding-style.html),
e esta deve ser a referência primária para qualquer dúvida não explicitamente coberta aqui.

## Idioma

O código é escrito em inglês, inclusive nomes de variáveis, comentários, abreviações, dentre outros.
A única exceção disso são saídas que são vistas pelo usuário, que são em português.

## Estrutura de Arquivos

Geralmente, módulos têm um header (`.h`) e um arquivo de implementação (`.c`). O arquivo de cabeçalho
deve conter as declarações públicas, enquanto o arquivo de implementação deve conter a lógica privada
e a definição concreta de eventuais tipos opacos definidos no header.

```c
// module.h - public

typedef struct type type;

void module_type_verb(type *self);

// module.c - private

#include "module.h"

struct type {
        // ... struct members ...
};

void module_type_verb(struct type *self);
```

Com exceções pontuais, o código segue essa estrutura.

## Estilo de Nomenclatura

### Funções

Para funções que estão aliadas a algum tipo (membros ou métodos), o código segue a estrutura
`módulo_tipo_ação_complemento()`. O primeiro argumento, invariavelmente, é chamado de `self`.

Exemplos:

  - `array_new()`
  - `array_del()`
  - `cursor_move_up(cursor *self)`
  - `inventory_append(inventory *self, equipment *e)`

Com algumas exceções pontuais, por exemplo, `_new`, `_del`, `at`, etc, os sufixos devem ser verbos.
Nem toda ação requer complemento.

### Macros

Function Macros devem ser evitadas.

### Variáveis e Membros de Struct

Variáveis e membros de `struct` utilizam `snake_case`. Exemplos: `self->len`, `e->id`,
`qty.available`.

## Espaçamento e Formatação

### Indentação

Utilizamos **tabs** para indentação. A largura da tabulação é de 8 caracteres, conforme o padrão do
Kernel Linux. Não misture tabs e espaços.

### Linhas em Branco

Use linhas em branco para separar blocos lógicos de código e melhorar a legibilidade. Evite linhas
em branco excessivas.

### Limite de Colunas

Procure manter as linhas de código com um máximo de 100 colunas para melhor visualização em
terminais. Quebre linhas longas de forma sensata.

### Chaves

Chaves abertas (`{`) devem estar no final da linha da declaração da função ou do bloco de código, e
não na próxima linha, invariavelmente.

```c
int function(args...) {
        // code ...
}
```

### Type Casting

Um espaço deve existir após todo `type casting`.

```c
T t = /* some value */;
Q q = (Q) t;
```

### Ponteiros e Asteriscos

O asterisco de declaração de ponteiro deve estar junto ao nome da variável.

```c
void *p; // correct
void* p; // incorrect
```

### Operadores

Operadores binários devem ter um espaço antes e depois.

```c
a = b + c;
if (x == y)
```

## Inclusão de Arquivos (`#include`)

Os includes devem ser organizados da seguinte forma: *System Includes* (`<arquivo.h>`) primeiro, em
ordem alfabética. Seguidos por uma linha em branco, o cabeçalho da interface do módulo atual que
está sendo implementado (ex: `"array.h"`). Após outra linha em branco, vêm os demais *Local Includes*
(`"arquivo.h"`), também em ordem alfabética.

Exemplo:

```c
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "array.h"

#include "tsv.h"
```

## Constantes e `NULL` Pointers

- Ponteiros nulos devem ser explicitamente comparados com `(void *) 0`, em vez de `NULL` ou `0`.
Esse é um uso apenas por razões de explicitude e estilística.

## Asserções

- `assert()` é utilizado para verificar pré-condições críticas que, se violadas, indicam um erro de
programação (bugs lógicos). Não é usado para validação de entrada do usuário ou condições de runtime esperadas.

## Funções Auxiliares (`static`)

Funções auxiliares que não são expostas na interface pública do módulo são declaradas como `static`.
Isso restringe sua visibilidade ao arquivo de implementação, promovendo encapsulamento e evitando
eventuais conflitos de nomes.

Exemplos:

- `inventory_clear_contents()`
- `size_t_to_str()`
- `prompt()`


## Módulos

+-------------+------------------------------------------------------------------------------------+
| Módulo      |                                                                                    |
+-------------+------------------------------------------------------------------------------------+
| `array`     | Implementa um array dinâmico genérico (`void **data`).                             |
| `cursor`    | Gerencia a posição de um cursor dentro de uma sequência de itens.                  |
| `inventory` | Gerencia equipamentos com o auxílio de outros módulos.                             |
| `runtime`   | Gerencia o estado geral da aplicação, orquestrando as interações entre os módulos. | 
+-------------+------------------------------------------------------------------------------------+
