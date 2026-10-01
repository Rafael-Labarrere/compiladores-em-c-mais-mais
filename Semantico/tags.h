#ifndef TAGS_H
#define TAGS_H

enum Tag {
    // Literais e Identificadores
    NUM = 256, REAL_VAL, STR, ID,
    // Estrutura e Fluxo
    ALGORITMO, INICIO, FIM, SE, ENTAO, SENAO, ENQUANTO, FACA, PARA, DE, ATE,
    // Tipos de Variáveis
    INTEIRO, REAL, CARACTERE, LOGICO, VERDADEIRO,  FALSO,
    // Entrada e Saída
    ESCREVA, LEIA,
    // Operadores Compostos
    ATRIBUICAO, // <-
    DIFERENTE,  // <>
    MAIOR_IGUAL,// >=
    MENOR_IGUAL // <=
};

#endif