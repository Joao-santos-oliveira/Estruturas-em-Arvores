#ifndef TRIE_HPP
#define TRIE_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

/**
 * @brief Nó individual da Árvore Trie de Prefixos.
 */
struct NoTrie {
    bool fimPalavra;
    std::map<char, NoTrie*> filhos;
    int id; // Identificador para visualização Graphviz

    NoTrie(int idNo = 0) : fimPalavra(false), id(idNo) {}

    ~NoTrie() {
        for (auto& par : filhos) {
            delete par.second;
        }
        filhos.clear();
    }
};

/**
 * @brief Implementação da Árvore Trie.
 */
class Trie {
private:
    NoTrie* raiz;
    int contadorNos;
    size_t totalPalavras;

    // Auxiliar recursivo para remoção de palavra e poda de nós órfãos
    bool removerAux(NoTrie* no, const std::string& palavra, size_t profundidade, bool& removido) {
        if (!no) return false;

        if (profundidade == palavra.length()) {
            if (!no->fimPalavra) {
                removido = false;
                return false; // Palavra não existia
            }
            no->fimPalavra = false;
            totalPalavras--;
            removido = true;

            // Retorna true se este nó deve ser desalocado pelo pai
            return no->filhos.empty();
        }

        char c = palavra[profundidade];
        auto it = no->filhos.find(c);
        if (it == no->filhos.end()) {
            removido = false;
            return false;
        }

        bool podeApagarFilho = removerAux(it->second, palavra, profundidade + 1, removido);

        if (podeApagarFilho) {
            delete it->second;
            no->filhos.erase(it);
            return !no->fimPalavra && no->filhos.empty();
        }

        return false;
    }

    // Coleta recursivamente todas as palavras com determinado prefixo
    void coletarPalavras(NoTrie* no, std::string prefixoAtual, std::vector<std::string>& lista) const {
        if (!no) return;

        if (no->fimPalavra) {
            lista.push_back(prefixoAtual);
        }

        for (const auto& par : no->filhos) {
            coletarPalavras(par.second, prefixoAtual + par.first, lista);
        }
    }

    // Gera nós e arestas no formato Graphviz DOT
    void exportarDOTAux(NoTrie* no, std::stringstream& ss) const {
        if (!no) return;

        if (no == raiz) {
            ss << "    node_" << no->id << " [label=\"RAIZ\", shape=circle, style=filled, fillcolor=\"#E0E0E0\"];\n";
        } else if (no->fimPalavra) {
            ss << "    node_" << no->id << " [label=\"\", shape=doublecircle, style=filled, fillcolor=\"#A5D6A7\", color=\"#2E7D32\"];\n";
        } else {
            ss << "    node_" << no->id << " [label=\"\", shape=circle, style=filled, fillcolor=\"#BBDEFB\", color=\"#1565C0\"];\n";
        }

        for (const auto& par : no->filhos) {
            char c = par.first;
            NoTrie* filho = par.second;

            ss << "    node_" << no->id << " -> node_" << filho->id 
               << " [label=\"" << c << "\", fontcolor=\"#D32F2F\", penwidth=1.5];\n";

            exportarDOTAux(filho, ss);
        }
    }

public:
    Trie() : raiz(new NoTrie(0)), contadorNos(0), totalPalavras(0) {}

    ~Trie() {
        delete raiz;
    }

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    /**
     * @brief Insere uma palavra na Trie.
     */
    void insert(const std::string& palavra) {
        if (palavra.empty()) return;

        NoTrie* atual = raiz;
        for (char c : palavra) {
            if (atual->filhos.find(c) == atual->filhos.end()) {
                atual->filhos[c] = new NoTrie(++contadorNos);
            }
            atual = atual->filhos[c];
        }

        if (!atual->fimPalavra) {
            atual->fimPalavra = true;
            totalPalavras++;
        }
    }

    /**
     * @brief Busca se uma palavra existe na Trie.
     */
    bool search(const std::string& palavra) const {
        if (palavra.empty()) return false;

        NoTrie* atual = raiz;
        for (char c : palavra) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) {
                return false;
            }
            atual = it->second;
        }
        return atual != nullptr && atual->fimPalavra;
    }

    /**
     * @brief Verifica se existe alguma palavra com o prefixo dado.
     */
    bool startsWith(const std::string& prefixo) const {
        NoTrie* atual = raiz;
        for (char c : prefixo) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) {
                return false;
            }
            atual = it->second;
        }
        return atual != nullptr;
    }

    /**
     * @brief Remove uma palavra da Trie e libera nós não utilizados.
     */
    bool remove(const std::string& palavra) {
        if (palavra.empty()) return false;
        bool removido = false;
        removerAux(raiz, palavra, 0, removido);
        return removido;
    }

    /**
     * @brief Retorna todas as palavras iniciadas com o prefixo.
     */
    std::vector<std::string> autocomplete(const std::string& prefixo) const {
        std::vector<std::string> lista;
        NoTrie* atual = raiz;

        for (char c : prefixo) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) {
                return lista;
            }
            atual = it->second;
        }

        coletarPalavras(atual, prefixo, lista);
        return lista;
    }

    size_t size() const {
        return totalPalavras;
    }

    std::string exportDOT(const std::string& titulo = "Trie") const {
        std::stringstream ss;
        ss << "digraph \"" << titulo << "\" {\n";
        ss << "    rankdir=TB;\n";
        ss << "    node [fontsize=12, fontname=\"Arial\"];\n";
        ss << "    edge [fontsize=11, fontname=\"Arial\"];\n";
        ss << "    labelloc=\"t\";\n";
        ss << "    label=\"" << titulo << "\";\n";

        exportarDOTAux(raiz, ss);

        ss << "}\n";
        return ss.str();
    }
};

#endif // TRIE_HPP
