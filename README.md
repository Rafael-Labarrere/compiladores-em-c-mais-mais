Esse repositório tem o intuito de auxiliar nos estudos de compiladores
Simula as análises léxica, semantica e sintática. Feita em C++

# Compilador Portugol — Análises Léxica, Sintática e Semântica

Estudo prático de construção de compiladores desenvolvido na disciplina de Compiladores do Centro Universitário de Brasília (CEUB). O projeto implementa, de forma incremental, as três primeiras fases de um compilador para uma linguagem no estilo **Portugol**, com saídas didáticas que mostram o funcionamento interno de cada fase (relatório de tokens, árvore sintática indentada e tabela semântica).

> **Linguagem de implementação:** C++ (STL: `string`, `map`, `vector`, `iostream`, `fstream`). Todo o código é *header-only* (`.h`) com um `main.cpp` por etapa.

---

## Sumário

- [Visão geral](#visão-geral)
- [A linguagem](#a-linguagem)
- [Fases do compilador](#fases-do-compilador)
- [Estrutura do repositório](#estrutura-do-repositório)
- [Como compilar e executar](#como-compilar-e-executar)
- [Exemplos](#exemplos)
- [Limitações conhecidas](#limitações-conhecidas)
- [Autor](#autor)

---

## Visão geral

Cada fase é construída sobre a anterior:

```
Código-fonte Portugol (.txt / .alg)
        │
        ▼
┌────────────────────┐
│  Análise Léxica    │  lexer.h + tags.h
│  tokens + símbolos │
└────────────────────┘
        │
        ▼
┌────────────────────┐
│  Análise Sintática │  parser.h
│  descendente       │  árvore impressa com indentação
│  recursivo         │
└────────────────────┘
        │
        ▼
┌────────────────────┐
│  Análise Semântica │  semantico.h (TabelaSemantica)
│  tipos e escopos   │  integrada ao parser
└────────────────────┘
```

---

## A linguagem

### Palavras reservadas

| Grupo               | Palavras                                                         |
|---------------------|------------------------------------------------------------------|
| Estrutura e fluxo   | `algoritmo`, `inicio`, `fim`, `se`, `entao`, `senao`, `enquanto`, `faca`, `para`, `de`, `ate` |
| Tipos               | `inteiro`, `real`, `caractere`, `logico`                         |
| Valores lógicos     | `verdadeiro`, `falso`                                            |
| Entrada e saída     | `escreva`, `leia`                                                |

### Operadores e delimitadores

- Atribuição: `<-`
- Relacionais: `=`, `<>`, `<`, `>`, `<=`, `>=`
- Aritméticos: `+`, `-`, `*`, `/`
- Delimitadores: `;`, `:`, `,`, `(`, `)`

### Gramática reconhecida

```
Programa      → 'algoritmo' (STR | ID) 'inicio' ListaComandos 'fim'
ListaComandos → { Declaracao } { Comando }
Declaracao    → Tipo ':' ID { ',' ID } ';'
Comando       → Atribuicao | Escrita | Leitura | Se | Enquanto | Para
Atribuicao    → ID '<-' Expressao ';'
Escrita       → 'escreva' '(' Expressao ')' ';'
Leitura       → 'leia' '(' ID ')' ';'
Se            → 'se' Condicao 'entao' ListaComandos [ 'senao' ListaComandos ] 'fim'
Enquanto      → 'enquanto' Condicao 'faca' ListaComandos 'fim'
Para          → 'para' ID 'de' Expressao 'ate' Expressao 'faca' ListaComandos 'fim'
Condicao      → Expressao OpRel Expressao
Expressao     → Termo { ('+' | '-') Termo }
Termo         → Fator { ('*' | '/') Fator }
Fator         → '(' Expressao ')' | Valor
Valor         → ID | NUM | REAL_VAL | STR | 'verdadeiro' | 'falso'
```

---

## Fases do compilador

### 1. Análise léxica — `lexer.h` e `tags.h`

O `Lexer` lê o código-fonte caractere a caractere e produz objetos `Token`, com subclasses `Num`, `Real`, `String` e `Word`. As tags dos tokens ficam no `enum Tag` (a partir de 256, para não colidir com caracteres ASCII).

- Reconhece inteiros, reais, strings entre aspas, identificadores e palavras reservadas.
- Mantém uma **tabela de símbolos** (`map<string, Word*>`) pré-carregada com as palavras reservadas; identificadores novos são inseridos ao serem encontrados.
- Trata operadores compostos (`<-`, `<=`, `>=`, `<>`).
- Reporta **erros léxicos** com linha e coluna para símbolos não reconhecidos (`$`, `#`, `@`) e segue a análise.

O `main` do analisador léxico imprime um relatório sequencial de tokens e, ao final, a tabela de símbolos:

| Categoria         | Exemplo de saída                         |
|-------------------|------------------------------------------|
| Literal inteiro   | `<LITERAL_INT, 10>`                      |
| Literal real      | `<LITERAL_REAL, 3.14>`                   |
| Literal string    | `<LITERAL_STR, "texto">`                 |
| Identificador     | `<ID, "contador">`                       |
| Palavra reservada | `<RESERVADA, "algoritmo">`               |
| Atribuição        | `<OP_ATRIBUICAO, '<-'>`                  |
| Relacionais       | `<OP_RELACIONAL, '<=' (menor ou igual)>` |
| Outros            | `<OP_OU_DELIM, ';'>`                     |

### 2. Análise sintática — `parser.h`

Parser **descendente recursivo**, com uma função por regra da gramática. Cada regra imprime seu nó com indentação, produzindo uma visualização textual da árvore de derivação (`|-- PROGRAMA`, `|-- COMANDO`, ...), e cada token consumido aparece como `> token`.

- **Recuperação de erros em modo pânico:** ao encontrar um erro sintático, informa linha, token esperado e token encontrado, e avança até um ponto de sincronização (`;`, `fim`, `senao` ou `inicio`).
- Relatório final com contagem de erros.

### 3. Análise semântica — `semantico.h`

A classe `TabelaSemantica` é integrada ao parser. As funções `expressao()`, `termo()`, `fator()` e `valor()` passam a **retornar o tipo** da subárvore reconhecida, e as regras de comando chamam as verificações:

| Verificação                  | Regra                                                                 |
|------------------------------|-----------------------------------------------------------------------|
| Declaração duplicada         | Erro se o nome já existe **no mesmo escopo**                          |
| Identificador não declarado  | Erro em qualquer uso de ID sem declaração visível                     |
| Atribuição                   | Tipos devem coincidir; `real ← inteiro` é permitido (promoção)       |
| Comparação                   | Tipos iguais, ou mistura `inteiro`/`real`                             |
| Operações aritméticas        | Apenas `inteiro`/`real`; se algum operando é `real`, o resultado é `real` |
| Condição de `se`/`enquanto`  | Deve ser do tipo `logico`                                             |
| Laço `para`                  | Variável de controle e limites devem ser `inteiro` ou `real`          |
| Escopos                      | Novo escopo nos blocos `se`, `senao`, `enquanto` e `para`; variáveis do bloco são descartadas ao sair |

Cada entrada da tabela guarda **nome, tipo, categoria e nível de escopo**, e a tabela é impressa ao final da análise. Erros semânticos usam o tipo especial `"ERRO"`, que **não propaga** novos erros em cascata.

---

## Estrutura do repositório

```
.
├── lexico/
│   ├── main.cpp          # Driver: relatório de tokens + tabela de símbolos
│   ├── lexer.h
│   └── tags.h
├── sintatico/
│   ├── main.cpp          # Driver: árvore sintática
│   ├── lexer.h
│   ├── tags.h
│   └── parser.h          # Parser descendente recursivo
├── semantico/
│   ├── main.cpp          # Driver: sintático + semântico
│   ├── lexer.h
│   ├── tags.h
│   ├── parser.h          # Parser com ações semânticas
│   └── semantico.h       # TabelaSemantica
├── exemplos/             # Programas Portugol de teste
├── .gitignore
└── README.md
```

> **Dica:** evite versionar binários e arquivos gerados (`*.exe`, `*.gch`). Sugestão de `.gitignore`:
>
> ```
> *.exe
> *.gch
> *.o
> ```

---

## Como compilar e executar

**Requisitos:** `g++` com suporte a C++98 ou superior. O código foi escrito evitando recursos modernos (`NULL` em vez de `nullptr`, laços por índice), então compila em compiladores mais antigos, como os usados em ambientes de laboratório.

```bash
# Léxico
cd lexico
g++ main.cpp -o lexico
./lexico

# Sintático
cd sintatico
g++ main.cpp -o sintatico
./sintatico

# Semântico (análise completa)
cd semantico
g++ main.cpp -o compilador
./compilador
```

O programa solicita o caminho do arquivo-fonte:

```
### ANALISADOR SINTATICO (Portugol) ###
Arquivo: ../exemplos/valido.txt
```

---

## Exemplos

### Programa válido

```
algoritmo "soma"
inicio
    inteiro: a, b;
    real: media;

    leia(a);
    leia(b);
    media <- (a + b) / 2;
    escreva(media);
fim
```

A análise completa imprime a árvore sintática, a tabela semântica (`a`, `b` e `media` com tipo e escopo 0) e, ao final, a mensagem de sucesso: `Programa semanticamente valido!`.

### Erros semânticos detectados

```
algoritmo "erros"
inicio
    inteiro: x;
    caractere: nome;

    x <- "texto";      // incompatibilidade de tipos na atribuição
    y <- 10;           // identificador 'y' não declarado
    nome <- x + 1;     // atribuição de inteiro a caractere
fim
```

Saída esperada (no `stderr`):

```
>>> ERRO SEMANTICO [Linha 6]: Incompatibilidade de tipos na atribuicao: lado esquerdo e 'inteiro', lado direito e 'caractere'.
>>> ERRO SEMANTICO [Linha 7]: Identificador 'y' nao declarado.
>>> ERRO SEMANTICO [Linha 8]: Incompatibilidade de tipos na atribuicao: lado esquerdo e 'caractere', lado direito e 'inteiro'.
```

> Os números de linha dependem do arquivo de teste; confira com uma execução real antes de publicar.

---

## Limitações conhecidas

- A condição de `se`/`enquanto` aceita apenas uma comparação relacional (`expr op expr`); não há operadores lógicos (`e`, `ou`, `nao`).
- Declarações de variáveis precisam vir **antes** dos comandos de cada bloco.
- Não há suporte a funções, procedimentos, vetores ou comentários na linguagem-fonte.
- Identificadores aceitam letras e dígitos (sem `_`).
- O projeto cobre as fases de *front-end* (léxica, sintática, semântica); não há geração de código.

---

## Autor

Rafael Labarrere — estudante do Centro Universitário de Brasília (CEUB), desenvolvido na disciplina de Compiladores.