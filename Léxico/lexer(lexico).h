#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <fstream>
#include "tags(lexico).h"

using namespace std;

// Classes de suporte 
class Token { public: int tag; Token(int t) : tag(t) {} virtual ~Token() {} };
class Num : public Token { public: int value; Num(int v) : Token(NUM), value(v) {} };
class Real : public Token { public: float value; Real(float v) : Token(REAL_VAL), value(v) {} };
class String : public Token { public: string text; String(string s) : Token(STR), text(s) {} };
class Word : public Token { public: string lexeme; Word(string s, int t) : Token(t), lexeme(s) {} };

class Lexer {
private:
    char peek;
    map<string, Word*> words;
    istream& input;
    int line = 1;
    int col = 0;

    void reserve(Word* w) { words[w->lexeme] = w; }
    
    char read() {
        char c = input.get();
        col++;
        if (c == '\n') { line++; col = 0; }
        return c;
    }

public:
    Lexer(istream& in) : peek(' '), input(in) { 
        // Reservas de palavras-chave
        reserve(new Word("algoritmo", ALGORITMO)); reserve(new Word("inicio", INICIO));
        reserve(new Word("fim", FIM));             reserve(new Word("se", SE));
        reserve(new Word("entao", ENTAO));         reserve(new Word("senao", SENAO));
        reserve(new Word("enquanto", ENQUANTO));   reserve(new Word("faca", FACA));
        reserve(new Word("para", PARA));           reserve(new Word("de", DE));
        reserve(new Word("ate", ATE));             reserve(new Word("inteiro", INTEIRO));
        reserve(new Word("real", REAL));           reserve(new Word("caractere", CARACTERE));
        reserve(new Word("logico", LOGICO));       reserve(new Word("escreva", ESCREVA));
        reserve(new Word("leia", LEIA));           reserve(new Word("verdadeiro", VERDADEIRO));
        reserve(new Word("falso", FALSO));
    }

    map<string, Word*> getSymbolTable() { return words; }

    Token* scan() {
        // Ignora espa�os em branco
        while (isspace(peek)) peek = read();

        if (input.eof() || peek == (char)-1) return NULL;

        // --- IN�CIO DO TRATAMENTO DE COMENT�RIOS E DIVIS�O ---
        if (peek == '/') {
            peek = read();
            
            // 1. Coment�rio de Linha �nica (//)
            if (peek == '/') {
                while (peek != '\n' && !input.eof() && peek != (char)-1) {
                    peek = read();
                }
                return scan(); // Recurs�o: busca o pr�ximo token v�lido
            } 
            // 2. Coment�rio de M�ltiplas Linhas (/* ... */)
            else if (peek == '*') {
                peek = read();
                bool closed = false;
                while (!input.eof() && peek != (char)-1) {
                    if (peek == '*') {
                        peek = read();
                        if (peek == '/') {
                            closed = true;
                            peek = read(); // Consome a '/' final
                            break;
                        }
                    } else {
                        peek = read();
                    }
                }
                
                // Tratamento de erro l�xico para coment�rio n�o fechado
                if (!closed) {
                    cerr << "\n>>> ERRO LEXICO [Linha " << line << "]: Comentario de multiplas linhas nao fechado." << endl;
                }
                return scan(); // Recurs�o: busca o pr�ximo token v�lido
            } 
            // 3. Operador de Divis�o (Apenas '/')
            else {
                return new Token('/'); 
            }
        }
        // --- FIM DO TRATAMENTO DE COMENT�RIOS ---

        // Lookahead para Operadores Compostos
        if (peek == '<') {
            peek = read();
            if (peek == '-') { peek = read(); return new Word("<-", ATRIBUICAO); }
            if (peek == '=') { peek = read(); return new Word("<=", MENOR_IGUAL); }
            if (peek == '>') { peek = read(); return new Word("<>", DIFERENTE); }
            return new Token('<');
        }
        if (peek == '>') {
            peek = read();
            if (peek == '=') { peek = read(); return new Word(">=", MAIOR_IGUAL); }
            return new Token('>');
        }

        // Strings
        if (peek == '"') {
            string s = "";
            while (true) {
                peek = read();
                if (peek == '"' || input.eof()) break;
                s += peek;
            }
            peek = read();
            return new String(s);
        }

        // N�meros
        if (isdigit(peek)) {
            float v = 0;
            do { v = v * 10 + (peek - '0'); peek = read(); } while (isdigit(peek));
            if (peek != '.') return new Num((int)v);
            float d = 10; peek = read();
            while (isdigit(peek)) { v += (peek - '0') / d; d *= 10; peek = read(); }
            return new Real(v);
        }

        // Identificadores e Palavras Reservadas
        if (isalpha(peek)) {
            string s = "";
            do { s += peek; peek = read(); } while (isalnum(peek));
            if (words.count(s)) return words[s];
            return words[s] = new Word(s, ID);
        }

        // Erros l�xicos de s�mbolos n�o suportados
        if (peek == '$' || peek == '#' || peek == '@') {
            cerr << "\n>>> ERRO LEXICO [Linha " << line << ", Coluna " << col << "]: Simbolo '" << peek << "' nao reconhecido." << endl;
            peek = read();
            return scan();
        }

        // Qualquer outro caractere (operadores simples como +, -, *, etc)
        Token* t = new Token(peek); peek = ' '; return t;
    }
};
#endif
