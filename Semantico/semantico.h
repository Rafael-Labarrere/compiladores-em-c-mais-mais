#ifndef SEMANTICO_H
#define SEMANTICO_H

/*
 * =============================================================
 *  TABELA SEMANTICA  -  Analisador Semantico para Portugol
 * =============================================================
 *
 *  Cada entrada guarda:
 *    nome     - lexema do identificador
 *    tipo     - "inteiro" | "real" | "caractere" | "logico"
 *    categoria- "variavel"  (extensivel a "constante", etc.)
 *    escopo   - nivel de aninhamento (0 = global)
 * =============================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "tags.h"

using namespace std;

// ---------------------------------------------------------------
// Registro da tabela semantica
// ---------------------------------------------------------------
struct SimboloSemantico {
    string nome;
    string tipo;       // "inteiro", "real", "caractere", "logico"
    string categoria;  // "variavel"
    int    escopo;     // nivel de aninhamento (0 = global)
};

// ---------------------------------------------------------------
// Classe principal - gerencia escopos e verificacoes
// ---------------------------------------------------------------
class TabelaSemantica {
private:
    vector<SimboloSemantico> tabela;
    int escopoAtual = 0;
    int erros = 0;

    void erroSemantico(int linha, const string& msg) {
        erros++;
        cerr << "\n>>> ERRO SEMANTICO [Linha " << linha << "]: " << msg << endl;
    }

public:
    // Controle de escopo
    void entrarEscopo() { escopoAtual++; }

    void sairEscopo() {
        vector<SimboloSemantico> restantes;
        for (auto& s : tabela)
            if (s.escopo < escopoAtual)
                restantes.push_back(s);
        tabela = restantes;
        escopoAtual--;
    }

    // Declaracao de variavel
    //   - verifica duplicidade no mesmo escopo
    //   - insere na tabela
    void declararVariavel(const string& nome, const string& tipo, int linha) {
        for (auto& s : tabela) {
            if (s.nome == nome && s.escopo == escopoAtual) {
                erroSemantico(linha,
                    "Variavel '" + nome + "' ja declarada neste escopo.");
                return;
            }
        }
        SimboloSemantico novo;
        novo.nome      = nome;
        novo.tipo      = tipo;
        novo.categoria = "variavel";
        novo.escopo    = escopoAtual;
        tabela.push_back(novo);
    }

    // Busca um simbolo por nome no escopo visivel
    SimboloSemantico* buscar(const string& nome) {
        for (int i = (int)tabela.size() - 1; i >= 0; i--) {
            if (tabela[i].nome == nome && tabela[i].escopo <= escopoAtual)
                return &tabela[i];
        }
        return nullptr;
    }

    // Verifica se identificador foi declarado
    //   Retorna o tipo se encontrado, "ERRO" se nao encontrado
    string verificarDeclarado(const string& nome, int linha) {
        SimboloSemantico* s = buscar(nome);
        if (!s) {
            erroSemantico(linha,
                "Identificador '" + nome + "' nao declarado.");
            return "ERRO";
        }
        return s->tipo;
    }

    // Verifica compatibilidade de tipos em atribuicao
    //   inteiro <- inteiro   : OK
    //   real    <- inteiro   : OK (promocao)
    //   real    <- real      : OK
    //   logico  <- logico    : OK
    //   caractere<- caractere: OK
    //   qualquer <- ERRO     : nao propaga novo erro
    void verificarAtribuicao(const string& tipoEsq,
                             const string& tipoDir,
                             int linha) {
        if (tipoDir == "ERRO" || tipoEsq == "ERRO") return;
        if (tipoEsq == tipoDir) return;
        if (tipoEsq == "real" && tipoDir == "inteiro") return;
        erroSemantico(linha,
            "Incompatibilidade de tipos na atribuicao: "
            "lado esquerdo e '" + tipoEsq + "', "
            "lado direito e '" + tipoDir + "'.");
    }

    // Verifica que dois tipos sao compativeis para comparacao
    void verificarComparacao(const string& tipoEsq,
                             const string& tipoDir,
                             int linha) {
        if (tipoDir == "ERRO" || tipoEsq == "ERRO") return;
        if (tipoEsq == tipoDir) return;
        if ((tipoEsq == "real"    && tipoDir == "inteiro") ||
            (tipoEsq == "inteiro" && tipoDir == "real"))    return;
        erroSemantico(linha,
            "Comparacao entre tipos incompativeis: '"
            + tipoEsq + "' e '" + tipoDir + "'.");
    }

    // Verifica que o tipo de uma condicao (se/enquanto) e logico
    void verificarCondicaoLogica(const string& tipo, int linha) {
        if (tipo == "ERRO") return;
        if (tipo != "logico") {
            erroSemantico(linha,
                "Condicao deve ser do tipo 'logico', mas e do tipo '"
                + tipo + "'.");
        }
    }

    // Determina o tipo resultante de uma operacao aritmetica
    string tipoOperacao(const string& t1, const string& t2, int linha) {
        if (t1 == "ERRO" || t2 == "ERRO") return "ERRO";
        if (t1 == "real" || t2 == "real") return "real";
        if (t1 == "inteiro" && t2 == "inteiro") return "inteiro";
        erroSemantico(linha,
            "Operacao aritmetica invalida entre tipos '"
            + t1 + "' e '" + t2 + "'.");
        return "ERRO";
    }

    int getErros()       const { return erros; }
    int getEscopoAtual() const { return escopoAtual; }

    // Imprime a tabela (para depuracao / arguicao)
    void imprimir() const {
        cout << "\n--- TABELA SEMANTICA ---\n";
        cout << "NOME\t\tTIPO\t\tCATEGORIA\tESCOPO\n";
        cout << string(60, '-') << "\n";
        for (auto& s : tabela) {
            cout << s.nome << "\t\t"
                 << s.tipo << "\t\t"
                 << s.categoria << "\t\t"
                 << s.escopo << "\n";
        }
        cout << string(60, '-') << "\n";
    }
};

#endif
