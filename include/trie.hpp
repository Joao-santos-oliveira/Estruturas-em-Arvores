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
    int id; // Identificador para visualização

    NoTrie(int idNo = 0) : fimPalavra(false), id(idNo) {}

    ~NoTrie() {
        for (auto& par : filhos) {
            delete par.second;
        }
        filhos.clear();
    }
};

/**
 * @brief Implementação da Árvore Trie de Prefixos.
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
                return false;
            }
            no->fimPalavra = false;
            totalPalavras--;
            removido = true;
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

    void coletarPalavras(NoTrie* no, std::string prefixoAtual, std::vector<std::string>& lista) const {
        if (!no) return;
        if (no->fimPalavra) lista.push_back(prefixoAtual);
        for (const auto& par : no->filhos) {
            coletarPalavras(par.second, prefixoAtual + par.first, lista);
        }
    }

    void exportarJSONAux(NoTrie* no, char charAresta, std::stringstream& ss) const {
        if (!no) return;
        ss << "{";
        ss << "\"id\":" << no->id << ",";
        if (charAresta != '\0') {
            ss << "\"caractere\":\"" << charAresta << "\",";
        } else {
            ss << "\"caractere\":\"RAIZ\",";
        }
        ss << "\"fimPalavra\":" << (no->fimPalavra ? "true" : "false") << ",";
        ss << "\"filhos\":[";
        bool primeiro = true;
        for (const auto& par : no->filhos) {
            if (!primeiro) ss << ",";
            exportarJSONAux(par.second, par.first, ss);
            primeiro = false;
        }
        ss << "]}";
    }

    size_t contarNosAux(NoTrie* no) const {
        if (!no) return 0;
        size_t total = 1;
        for (const auto& par : no->filhos) {
            total += contarNosAux(par.second);
        }
        return total;
    }

public:
    Trie() : raiz(new NoTrie(0)), contadorNos(0), totalPalavras(0) {}

    ~Trie() {
        delete raiz;
    }

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

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

    bool remove(const std::string& palavra) {
        if (palavra.empty()) return false;
        bool removido = false;
        removerAux(raiz, palavra, 0, removido);
        return removido;
    }

    std::vector<std::string> autocomplete(const std::string& prefixo) const {
        std::vector<std::string> lista;
        NoTrie* atual = raiz;

        for (char c : prefixo) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) return lista;
            atual = it->second;
        }

        coletarPalavras(atual, prefixo, lista);
        return lista;
    }

    size_t size() const {
        return totalPalavras;
    }

    size_t getNodeCount() const {
        return contarNosAux(raiz);
    }

    std::string exportarJSON() const {
        std::stringstream ss;
        ss << "{\"tipo\":\"Trie\",\"totalPalavras\":" << totalPalavras 
           << ",\"totalNos\":" << getNodeCount() << ",\"arvore\":";
        exportarJSONAux(raiz, '\0', ss);
        ss << "}";
        return ss.str();
    }
};

#endif // TRIE_HPP
