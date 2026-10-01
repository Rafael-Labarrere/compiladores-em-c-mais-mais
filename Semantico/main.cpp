#include <iostream>
#include <fstream>
#include "tags.h"
#include "lexer.h"
#include "parser.h"

using namespace std;

int main() {
    string caminho;
    cout << "### ANALISADOR SINTATICO (Portugol) ###\nArquivo: ";
    getline(cin, caminho);

    ifstream arquivo(caminho.c_str());
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo '" << caminho << "'!" << endl;
        return 1;
    }

    Lexer lexer(arquivo);
    Parser parser(lexer);

    parser.parse(); // inicia a análise sintática

    arquivo.close();
    return 0;
}