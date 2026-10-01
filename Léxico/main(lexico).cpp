#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "tags(lexico).h"
#include "lexer(lexico).h"

using namespace std;

int main() {
    string caminho;
    cout << "### ANALISADOR LEXICO ###\nArquivo: ";
    getline(cin, caminho);

    ifstream arquivo(caminho.c_str());
    if (!arquivo.is_open()) { 
        cerr << "Erro ao abrir arquivo!" << endl; 
        return 1; 
    }

    Lexer lexer(arquivo);
    cout << "\n--- RELATORIO SEQUENCIAL DE TOKENS ---\n" << endl;

    while(true) {
        Token* t = lexer.scan();
        if (t == NULL) break;

        if(t->tag == NUM) cout << "<LITERAL_INT, " << static_cast<Num*>(t)->value << "> \n";
        else if(t->tag == REAL_VAL) cout << "<LITERAL_REAL, " << static_cast<Real*>(t)->value << "> \n";
        else if(t->tag == STR) cout << "<LITERAL_STR, \"" << static_cast<String*>(t)->text << "\"> \n";
        else if(t->tag == ID) cout << "<ID, \"" << static_cast<Word*>(t)->lexeme << "\"> \n";
        else if(t->tag >= ALGORITMO && t->tag <= FALSO) cout << "<RESERVADA, \"" << static_cast<Word*>(t)->lexeme << "\"> \n";
        
        // --- SEPARA��O DOS OPERADORES COMPOSTOS ---
        else if(t->tag == ATRIBUICAO)  cout << "<OP_ATRIBUICAO, '<-'> \n";
        else if(t->tag == MENOR_IGUAL) cout << "<OP_RELACIONAL, '<=' (menor ou igual)> \n";
        else if(t->tag == MAIOR_IGUAL) cout << "<OP_RELACIONAL, '>=' (maior ou igual)> \n";
        else if(t->tag == DIFERENTE)   cout << "<OP_RELACIONAL, '<>' (diferente)> \n";
        
        // Trata os operadores simples de rela��o '<' e '>' 
        else if(t->tag == '<') cout << "<OP_RELACIONAL, '<' (menor que)> \n";
        else if(t->tag == '>') cout << "<OP_RELACIONAL, '>' (maior que)> \n";
        
        // Outros delimitadores e operadores matem�ticos
        else cout << "<OP_OU_DELIM, '" << (char)t->tag << "'> \n";
        
        if (t->tag == ';') cout << endl;
    }

    // --- TABELA DE SIMBOLOS ---
    cout << "\n\n--- TABELA DE SIMBOLOS ---" << endl;
    cout << left << setw(20) << "LEXEMA" << " | " << "TIPO/CATEGORIA" << endl;
    cout << "---------------------------------------------------" << endl;
    
    map<string, Word*> tabela = lexer.getSymbolTable();
    map<string, Word*>::iterator it;
    
    for (it = tabela.begin(); it != tabela.end(); ++it) {
        
        string categoria;
        if (it->second->tag == ID) {
            categoria = "Identificador";
        } else {
            categoria = "Palavra Reservada";
        }

        cout << left << setw(20) << it->first << " | " << categoria << endl;
    }
    cout << "---------------------------------------------------" << endl;

    arquivo.close();
    return 0;
}
