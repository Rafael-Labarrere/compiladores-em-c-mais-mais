#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include "tags.h"
#include "lexer.h"

using namespace std;

class Parser {
private:
    Lexer& lexer;
    Token* current;
    int errorCount = 0;
    int level = 0; // Controla a indentação da árvore

    // Função auxiliar para imprimir a árvore com indentação
    void printNode(string name) {
        for(int i = 0; i < level; i++) cout << "  ";
        cout << "|-- " << name << endl;
    }

    string tokenName(Token* t) {
        if (!t) return "fim de arquivo";
        switch (t->tag) {
            case NUM:        return "numero inteiro";
            case REAL_VAL:   return "numero real";
            case STR:        return "string";
            case ID:         return "ID:" + static_cast<Word*>(t)->lexeme;
            case ALGORITMO:  return "'algoritmo'";
            case INICIO:     return "'inicio'";
            case FIM:        return "'fim'";
            case ATRIBUICAO: return "'<-'";
            default: {
                string s = "'";
                s += (char)t->tag;
                s += "'";
                return s;
            }
        }
    }

    void error(const string& expected) {
        errorCount++;
        cerr << "\n>>> ERRO SINTATICO [Linha " << lexer.getLine() << "]: Esperado " << expected
             << ", mas encontrado " << tokenName(current) << "." << endl;
        
        while (current && current->tag != ';' && current->tag != FIM && 
               current->tag != SENAO && current->tag != INICIO) {
            move();
        }
        if (current && current->tag == ';') move();
    }

    void move() {
        current = lexer.scan();
    }

    void match(int tag, const string& name) {
        if (current && current->tag == tag) {
            // Opcional: imprimir o terminal casado na árvore
            for(int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << endl;
            move();
        } else {
            error(name);
        }
    }

    // --- Produções da Gramática com Lógica de Árvore ---

    void programa() {
        printNode("PROGRAMA");
        level++;
        
        match(ALGORITMO, "'algoritmo'");
        if (current && (current->tag == STR || current->tag == ID)) {
            for(int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << endl;
            move();
        } else {
            error("nome do algoritmo");
        }
        
        match(INICIO, "'inicio'");
        listaComandos();
        match(FIM, "'fim'");
        
        level--;
    }

    void listaComandos() {
        printNode("LISTA_COMANDOS");
        level++;
        while (current && isInicioComando()) {
            comando();
        }
        level--;
    }

    bool isInicioComando() {
        if (!current) return false;
        int tag = current->tag;
        return tag == ID || tag == ESCREVA || tag == LEIA ||
               tag == SE || tag == ENQUANTO || tag == PARA;
    }

    void comando() {
        printNode("COMANDO");
        level++;
        switch (current->tag) {
            case ID:       atribuicao(); break;
            case ESCREVA:  escrita();    break;
            case LEIA:     leitura();    break;
            case SE:       se();         break;
            case ENQUANTO: enquanto();   break;
            case PARA:     para();       break;
            default: error("comando");
        }
        level--;
    }

    void atribuicao() {
        printNode("ATRIBUICAO");
        level++;
        match(ID, "identificador");
        match(ATRIBUICAO, "'<-'");
        valor();
        match(';', "';'");
        level--;
    }

    void escrita() {
        printNode("ESCRITA");
        level++;
        match(ESCREVA, "'escreva'");
        match('(', "'('");
        valor();
        match(')', "')'");
        match(';', "';'");
        level--;
    }

    void leitura() {
        printNode("LEITURA");
        level++;
        match(LEIA, "'leia'");
        match('(', "'('");
        match(ID, "identificador");
        match(')', "')'");
        match(';', "';'");
        level--;
    }

    void se() {
        printNode("SE_ENTAO");
        level++;
        match(SE, "'se'");
        condicao();
        match(ENTAO, "'entao'");
        listaComandos();
        if (current && current->tag == SENAO) {
            printNode("SENAO");
            level++;
            move();
            listaComandos();
            level--;
        }
        match(FIM, "'fim'");
        level--;
    }

    void enquanto() {
        printNode("ENQUANTO");
        level++;
        match(ENQUANTO, "'enquanto'");
        condicao();
        match(FACA, "'faca'");
        listaComandos();
        match(FIM, "'fim'");
        level--;
    }

    void para() {
        printNode("PARA");
        level++;
        match(PARA, "'para'");
        match(ID, "identificador");
        match(DE, "'de'");
        valor();
        match(ATE, "'ate'");
        valor();
        match(FACA, "'faca'");
        listaComandos();
        match(FIM, "'fim'");
        level--;
    }

    void condicao() {
        printNode("CONDICAO");
        level++;
        valor();
        operadorRelacional();
        valor();
        level--;
    }

    void operadorRelacional() {
        printNode("OPER_REL");
        int t = current ? current->tag : 0;
        if (t == '=' || t == DIFERENTE || t == '>' || t == '<' || t == MAIOR_IGUAL || t == MENOR_IGUAL) {
            for(int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << endl;
            move();
        } else {
            error("operador relacional");
        }
    }

    void valor() {
        printNode("VALOR");
        int t = current ? current->tag : 0;
        if (t == ID || t == NUM || t == REAL_VAL || t == STR || t == VERDADEIRO || t == FALSO) {
            for(int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << endl;
            move();
        } else {
            error("valor");
        }
    }

public:
    Parser(Lexer& lex) : lexer(lex), current(nullptr) {}

    void parse() {
        cout << "\n--- ARVORE SINTATICA ---\n" << endl;
        move();
        programa();
        
        if (errorCount == 0) {
            cout << "\n>>> SUCESSO: Programa sintaticamente valido!" << endl;
        } else {
            cout << "\n>>> FALHA: Foram encontrados " << errorCount << " erro(s)." << endl;
        }
    }
};

#endif