#ifndef PARSER_H
#define PARSER_H

/*
 * =============================================================
 *  PARSER  –  Analisador Sintatico + Semantico para Portugol
 * =============================================================
 *
 *  Mudanças em relação ao parser original:
 *
 *  1. Inclui "semantico.h" e mantém uma instância de
 *     TabelaSemantica.
 *
 *  2. listaComandos() agora reconhece DECLARACOES antes dos
 *     comandos executáveis:
 *       Declaracao -> Tipo ':' ListaIds ';'
 *
 *  3. valor() / fator() / termo() / expressao() passam a
 *     RETORNAR o tipo semântico da sub-árvore reconhecida.
 *
 *  4. Ações semânticas embutidas:
 *       – declararVariavel ao processar declarações
 *       – verificarDeclarado em todo uso de ID
 *       – verificarAtribuicao na regra de atribuição
 *       – verificarComparacao na condição relacional
 *       – verificarCondicaoLogica em se / enquanto
 *       – verificar variável de controle e limites em para
 *
 *  5. Escopos abertos/fechados nos blocos se, enquanto e para.
 * =============================================================
 */

#include <iostream>
#include <string>
#include "tags.h"
#include "lexer.h"
#include "semantico.h"

using namespace std;

class Parser {
private:
    Lexer&          lexer;
    Token*          current;
    int             errorCount = 0;
    int             level      = 0;     // indentação da árvore sintática
    TabelaSemantica tabSem;             // tabela semântica separada

    // -------------------------------------------------------
    // Auxiliares de impressão da árvore
    // -------------------------------------------------------
    void printNode(const string& name) {
        for (int i = 0; i < level; i++) cout << "  ";
        cout << "|-- " << name << "\n";
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
            case INTEIRO:    return "'inteiro'";
            case REAL:       return "'real'";
            case CARACTERE:  return "'caractere'";
            case LOGICO:     return "'logico'";
            default: {
                string s = "'";
                s += (char)t->tag;
                s += "'";
                return s;
            }
        }
    }

    // -------------------------------------------------------
    // Recuperação de erros sintáticos
    // -------------------------------------------------------
    void error(const string& expected) {
        errorCount++;
        cerr << "\n>>> ERRO SINTATICO [Linha " << lexer.getLine()
             << "]: Esperado " << expected
             << ", mas encontrado " << tokenName(current) << ".\n";

        // Recuperação: avança até um ponto de sincronização
        while (current && current->tag != ';' && current->tag != FIM &&
               current->tag != SENAO && current->tag != INICIO) {
            move();
        }
        if (current && current->tag == ';') move();
    }

    void move()  { current = lexer.scan(); }

    void match(int tag, const string& name) {
        if (current && current->tag == tag) {
            for (int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << "\n";
            move();
        } else {
            error(name);
        }
    }

    // -------------------------------------------------------
    // Retorna o lexema do token atual (sem consumi-lo)
    // Deve ser chamado ANTES do match(ID, ...)
    // -------------------------------------------------------
    string lexemeAtual() {
        if (current && current->tag == ID)
            return static_cast<Word*>(current)->lexeme;
        return "";
    }

    // -------------------------------------------------------
    // Converte tag de tipo em string semântica
    // -------------------------------------------------------
    string tagParaTipo(int tag) {
        switch (tag) {
            case INTEIRO:   return "inteiro";
            case REAL:      return "real";
            case CARACTERE: return "caractere";
            case LOGICO:    return "logico";
            default:        return "desconhecido";
        }
    }

    bool isTipo() {
        if (!current) return false;
        int t = current->tag;
        return t == INTEIRO || t == REAL || t == CARACTERE || t == LOGICO;
    }

    // -------------------------------------------------------
    // PROGRAMA
    // -------------------------------------------------------
    void programa() {
        printNode("PROGRAMA");
        level++;

        match(ALGORITMO, "'algoritmo'");
        if (current && (current->tag == STR || current->tag == ID)) {
            for (int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << "\n";
            move();
        } else {
            error("nome do algoritmo");
        }

        match(INICIO, "'inicio'");
        listaComandos();
        match(FIM, "'fim'");

        level--;
    }

    // -------------------------------------------------------
    // LISTA DE COMANDOS
    //   Reconhece declarações (Tipo : ListaIds ;)
    //   e depois comandos executáveis.
    // -------------------------------------------------------
    void listaComandos() {
        printNode("LISTA_COMANDOS");
        level++;

        // Primeiro: zero ou mais declarações de variáveis
        while (current && isTipo()) {
            declaracao();
        }

        // Depois: zero ou mais comandos executáveis
        while (current && isInicioComando()) {
            comando();
        }

        level--;
    }

    // -------------------------------------------------------
    // DECLARACAO  ->  Tipo ':' ListaIds ';'
    // -------------------------------------------------------
    void declaracao() {
        printNode("DECLARACAO");
        level++;

        // Captura o tipo antes de consumir
        string tipo = tagParaTipo(current->tag);
        int linhaDecl = lexer.getLine();

        for (int i = 0; i < level + 1; i++) cout << "  ";
        cout << "> " << tokenName(current) << "\n";
        move(); // consome o tipo

        match(':', "':'");

        // Primeira variável (obrigatória)
        if (current && current->tag == ID) {
            string nome = lexemeAtual();
            match(ID, "identificador");
            tabSem.declararVariavel(nome, tipo, linhaDecl);
        } else {
            error("identificador");
        }

        // Demais variáveis separadas por vírgula
        while (current && current->tag == ',') {
            move(); // consome ','
            if (current && current->tag == ID) {
                string nome = lexemeAtual();
                match(ID, "identificador");
                tabSem.declararVariavel(nome, tipo, lexer.getLine());
            } else {
                error("identificador");
            }
        }

        match(';', "';'");
        level--;
    }

    bool isInicioComando() {
        if (!current) return false;
        int tag = current->tag;
        return tag == ID     || tag == ESCREVA || tag == LEIA ||
               tag == SE     || tag == ENQUANTO || tag == PARA;
    }

    // -------------------------------------------------------
    // COMANDO
    // -------------------------------------------------------
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
            default:       error("comando");
        }
        level--;
    }

    // -------------------------------------------------------
    // ATRIBUICAO  ->  ID '<-' expressao ';'
    //
    //  Ação semântica:
    //    1. Captura lexema ANTES do match(ID)
    //    2. Verifica se foi declarado → obtém tipoEsq
    //    3. Avalia expressão → obtém tipoDir
    //    4. Verifica compatibilidade de tipos
    // -------------------------------------------------------
    void atribuicao() {
        printNode("ATRIBUICAO");
        level++;

        int linha = lexer.getLine();
        string nome = lexemeAtual();          // ← captura ANTES do match
        match(ID, "identificador");

        string tipoEsq = tabSem.verificarDeclarado(nome, linha);

        match(ATRIBUICAO, "'<-'");

        string tipoDir = expressao();         // avalia lado direito

        tabSem.verificarAtribuicao(tipoEsq, tipoDir, linha);

        match(';', "';'");
        level--;
    }

    // -------------------------------------------------------
    // ESCRITA  ->  escreva '(' expressao ')' ';'
    //
    //  Ação semântica:
    //    – verifica que o identificador (se for ID) foi declarado
    // -------------------------------------------------------
    void escrita() {
        printNode("ESCRITA");
        level++;
        match(ESCREVA, "'escreva'");
        match('(', "'('");
        expressao(); // tipo não precisa ser restrito em escrita
        match(')', "')'");
        match(';', "';'");
        level--;
    }

    // -------------------------------------------------------
    // LEITURA  ->  leia '(' ID ')' ';'
    //
    //  Ação semântica:
    //    – verifica que o identificador foi declarado
    // -------------------------------------------------------
    void leitura() {
        printNode("LEITURA");
        level++;
        match(LEIA, "'leia'");
        match('(', "'('");

        int linha = lexer.getLine();
        string nome = lexemeAtual();          // ← captura ANTES do match
        match(ID, "identificador");
        tabSem.verificarDeclarado(nome, linha);

        match(')', "')'");
        match(';', "';'");
        level--;
    }

    // -------------------------------------------------------
    // SE  ->  se condicao entao listaComandos [senao listaComandos] fim
    //
    //  Ação semântica:
    //    – bloco then e else em novo escopo
    // -------------------------------------------------------
    void se() {
        printNode("SE_ENTAO");
        level++;
        match(SE, "'se'");

        string tipoCondicao = condicao();
        // Uma condição relacional (expr rel expr) já devolve "logico".
        // Verificamos mesmo assim para proteger contra futuros usos de ID logico.
        tabSem.verificarCondicaoLogica(tipoCondicao, lexer.getLine());

        match(ENTAO, "'entao'");

        tabSem.entrarEscopo();
        listaComandos();
        tabSem.sairEscopo();

        if (current && current->tag == SENAO) {
            printNode("SENAO");
            level++;
            move();
            tabSem.entrarEscopo();
            listaComandos();
            tabSem.sairEscopo();
            level--;
        }

        match(FIM, "'fim'");
        level--;
    }

    // -------------------------------------------------------
    // ENQUANTO  ->  enquanto condicao faca listaComandos fim
    //
    //  Ação semântica:
    //    – bloco em novo escopo
    // -------------------------------------------------------
    void enquanto() {
        printNode("ENQUANTO");
        level++;
        match(ENQUANTO, "'enquanto'");

        string tipoCondicao = condicao();
        tabSem.verificarCondicaoLogica(tipoCondicao, lexer.getLine());

        match(FACA, "'faca'");

        tabSem.entrarEscopo();
        listaComandos();
        tabSem.sairEscopo();

        match(FIM, "'fim'");
        level--;
    }

    // -------------------------------------------------------
    // PARA  ->  para ID de expressao ate expressao faca listaComandos fim
    //
    //  Ação semântica:
    //    – variável de controle deve existir e ser inteiro/real
    //    – limites de e ate devem ser inteiro/real
    //    – bloco em novo escopo
    // -------------------------------------------------------
    void para() {
        printNode("PARA");
        level++;
        match(PARA, "'para'");

        int linha = lexer.getLine();
        string nomeCtrl = lexemeAtual();       // ← captura ANTES do match
        match(ID, "identificador");

        string tipoCtrl = tabSem.verificarDeclarado(nomeCtrl, linha);
        if (tipoCtrl != "ERRO" && tipoCtrl != "inteiro" && tipoCtrl != "real") {
            // emite erro semântico manualmente via método público
            // (reutilizamos o mesmo mecanismo do objeto)
            tabSem.declararVariavel("__dummy__" + nomeCtrl, "inteiro", -1); // provoca nada
            cerr << "\n>>> ERRO SEMANTICO [Linha " << linha << "]: "
                 << "Variavel de controle '" << nomeCtrl
                 << "' deve ser do tipo 'inteiro' ou 'real', mas e '"
                 << tipoCtrl << "'.\n";
        }

        match(DE, "'de'");
        string tipoInicio = expressao();
        if (tipoInicio != "ERRO" && tipoInicio != "inteiro" && tipoInicio != "real") {
            cerr << "\n>>> ERRO SEMANTICO [Linha " << lexer.getLine() << "]: "
                 << "Limite inicial do 'para' deve ser inteiro ou real, mas e '"
                 << tipoInicio << "'.\n";
        }

        match(ATE, "'ate'");
        string tipoFim = expressao();
        if (tipoFim != "ERRO" && tipoFim != "inteiro" && tipoFim != "real") {
            cerr << "\n>>> ERRO SEMANTICO [Linha " << lexer.getLine() << "]: "
                 << "Limite final do 'para' deve ser inteiro ou real, mas e '"
                 << tipoFim << "'.\n";
        }

        match(FACA, "'faca'");

        tabSem.entrarEscopo();
        listaComandos();
        tabSem.sairEscopo();

        match(FIM, "'fim'");
        level--;
    }

    // -------------------------------------------------------
    // CONDICAO  ->  expressao operadorRelacional expressao
    //
    //  Retorna "logico" se os tipos forem compatíveis.
    // -------------------------------------------------------
    string condicao() {
        printNode("CONDICAO");
        level++;

        int linha = lexer.getLine();
        string tipoEsq = expressao();
        operadorRelacional();
        string tipoDir = expressao();

        tabSem.verificarComparacao(tipoEsq, tipoDir, linha);

        level--;
        return "logico"; // uma comparação relacional sempre produz logico
    }

    void operadorRelacional() {
        printNode("OPER_REL");
        int t = current ? current->tag : 0;
        if (t == '=' || t == DIFERENTE || t == '>' || t == '<' ||
            t == MAIOR_IGUAL || t == MENOR_IGUAL) {
            for (int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << "\n";
            move();
        } else {
            error("operador relacional");
        }
    }

    // -------------------------------------------------------
    // EXPRESSAO  ->  termo { ('+' | '-') termo }
    //
    //  Retorna o tipo resultante.
    // -------------------------------------------------------
    string expressao() {
        printNode("EXPRESSAO");
        level++;

        int linha = lexer.getLine();
        string tipo = termo();

        while (current && (current->tag == '+' || current->tag == '-')) {
            for (int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << "\n";
            move();
            string tipoDir = termo();
            tipo = tabSem.tipoOperacao(tipo, tipoDir, linha);
        }

        level--;
        return tipo;
    }

    // -------------------------------------------------------
    // TERMO  ->  fator { ('*' | '/') fator }
    //
    //  Retorna o tipo resultante.
    // -------------------------------------------------------
    string termo() {
        printNode("TERMO");
        level++;

        int linha = lexer.getLine();
        string tipo = fator();

        while (current && (current->tag == '*' || current->tag == '/')) {
            for (int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << "\n";
            move();
            string tipoDir = fator();
            tipo = tabSem.tipoOperacao(tipo, tipoDir, linha);
        }

        level--;
        return tipo;
    }

    // -------------------------------------------------------
    // FATOR  ->  '(' expressao ')'  |  VALOR
    //
    //  Retorna o tipo resultante.
    // -------------------------------------------------------
    string fator() {
        printNode("FATOR");
        level++;
        string tipo;

        if (current && current->tag == '(') {
            for (int i = 0; i < level + 1; i++) cout << "  ";
            cout << "> " << tokenName(current) << "\n";
            move();
            tipo = expressao();
            match(')', "')'");
        } else {
            tipo = valor();
        }

        level--;
        return tipo;
    }

    // -------------------------------------------------------
    // VALOR  ->  ID | NUM | REAL_VAL | STR | verdadeiro | falso
    //
    //  Retorna o tipo semântico:
    //    ID         → tipo declarado na tabela
    //    NUM        → "inteiro"
    //    REAL_VAL   → "real"
    //    STR        → "caractere"
    //    verdadeiro / falso → "logico"
    // -------------------------------------------------------
    string valor() {
        printNode("VALOR");
        int linha = lexer.getLine();

        if (!current) { error("valor"); return "ERRO"; }

        string tipo;
        switch (current->tag) {
            case ID: {
                string nome = lexemeAtual();   // ← captura ANTES do match
                match(ID, "identificador");
                tipo = tabSem.verificarDeclarado(nome, linha);
                break;
            }
            case NUM:
                for (int i = 0; i < level + 1; i++) cout << "  ";
                cout << "> " << tokenName(current) << "\n";
                move();
                tipo = "inteiro";
                break;
            case REAL_VAL:
                for (int i = 0; i < level + 1; i++) cout << "  ";
                cout << "> " << tokenName(current) << "\n";
                move();
                tipo = "real";
                break;
            case STR:
                for (int i = 0; i < level + 1; i++) cout << "  ";
                cout << "> " << tokenName(current) << "\n";
                move();
                tipo = "caractere";
                break;
            case VERDADEIRO:
            case FALSO:
                for (int i = 0; i < level + 1; i++) cout << "  ";
                cout << "> " << tokenName(current) << "\n";
                move();
                tipo = "logico";
                break;
            default:
                error("valor (identificador, numero, string ou logico)");
                tipo = "ERRO";
                break;
        }
        return tipo;
    }

public:
    Parser(Lexer& lex) : lexer(lex), current(nullptr) {}

    void parse() {
        cout << "\n--- ARVORE SINTATICA ---\n\n";
        move();
        programa();

        // Imprime a tabela semântica ao final (útil para arguição)
        tabSem.imprimir();

        int errosSem = tabSem.getErros();
        int errosSin = errorCount;

        if (errosSin == 0 && errosSem == 0) {
            cout << "\n>>> SUCESSO: Programa semanticamente valido!\n";
        } else {
            if (errosSin > 0)
                cout << "\n>>> FALHA SINTATICA: " << errosSin << " erro(s) sintatico(s).\n";
            if (errosSem > 0)
                cout << "\n>>> FALHA SEMANTICA: " << errosSem << " erro(s) semantico(s).\n";
        }
    }
};

#endif
