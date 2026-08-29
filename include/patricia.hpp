#ifndef PATRICIA_HPP
#define PATRICIA_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

/**
 * @brief Nó individual da Árvore Patricia (Radix Tree Compacta).
 */
struct NoPatricia {
    std::string prefixo;                 // Rótulo da string comprimida
    bool fimPalavra;                     // Indica se uma palavra válida termina aqui
    std::map<char, NoPatricia*> filhos;  // Filhos indexados pelo 1º caractere do prefixo
    int id;                              // Identificador único para visualização

    NoPatricia(const std::string& pref = "", bool terminal = false, int idNo = 0)
        : prefixo(pref), fimPalavra(terminal), id(idNo) {}

    ~NoPatricia() {
        for (auto& par : filhos) {
            delete par.second;
        }
        filhos.clear();
    }
};

/**
 * @brief Implementação da Árvore Patricia com métricas e serialização JSON.
 */
class PatriciaTree {
private:
    NoPatricia* raiz;
    int contadorNos;
    size_t totalPalavras;
    unsigned long long totalSplits;
    unsigned long long totalMerges;

    static size_t tamanhoPrefixoComum(const std::string& a, const std::string& b) {
        size_t tam = 0;
        size_t limite = std::min(a.length(), b.length());
        while (tam < limite && a[tam] == b[tam]) {
            tam++;
        }
        return tam;
    }

    void inserirAux(NoPatricia* noAtual, const std::string& palavra) {
        if (palavra.empty()) return;

        char primCaractere = palavra[0];
        auto it = noAtual->filhos.find(primCaractere);

        // Caso 1: Nenhuma aresta inicia com este caractere
        if (it == noAtual->filhos.end()) {
            noAtual->filhos[primCaractere] = new NoPatricia(palavra, true, ++contadorNos);
            totalPalavras++;
            return;
        }

        NoPatricia* filho = it->second;
        size_t tamComum = tamanhoPrefixoComum(palavra, filho->prefixo);

        // Caso 2: Coincidência exata
        if (tamComum == palavra.length() && tamComum == filho->prefixo.length()) {
            if (!filho->fimPalavra) {
                filho->fimPalavra = true;
                totalPalavras++;
            }
            return;
        }

        // Caso 3: Prefixo do filho é prefixo próprio da palavra
        if (tamComum == filho->prefixo.length()) {
            std::string restoPalavra = palavra.substr(tamComum);
            inserirAux(filho, restoPalavra);
            return;
        }

        // Caso 4: Divergência parcial -> Divisão de Nó (Split)
        totalSplits++;
        std::string parteComum = filho->prefixo.substr(0, tamComum);
        std::string restoFilho = filho->prefixo.substr(tamComum);

        NoPatricia* noDivisao = new NoPatricia(parteComum, false, ++contadorNos);
        
        filho->prefixo = restoFilho;
        noDivisao->filhos[restoFilho[0]] = filho;

        noAtual->filhos[primCaractere] = noDivisao;

        if (tamComum == palavra.length()) {
            noDivisao->fimPalavra = true;
            totalPalavras++;
        } else {
            std::string restoNova = palavra.substr(tamComum);
            noDivisao->filhos[restoNova[0]] = new NoPatricia(restoNova, true, ++contadorNos);
            totalPalavras++;
        }
    }

    bool buscarAux(NoPatricia* noAtual, const std::string& palavra) const {
        if (palavra.empty()) return noAtual->fimPalavra;

        char primCaractere = palavra[0];
        auto it = noAtual->filhos.find(primCaractere);
        if (it == noAtual->filhos.end()) {
            return false;
        }

        NoPatricia* filho = it->second;
        size_t tamComum = tamanhoPrefixoComum(palavra, filho->prefixo);

        if (tamComum < filho->prefixo.length()) {
            return false;
        }

        return buscarAux(filho, palavra.substr(tamComum));
    }

    void fundirSePossivel(NoPatricia* no) {
        if (!no->fimPalavra && no->filhos.size() == 1 && no != raiz) {
            totalMerges++;
            auto it = no->filhos.begin();
            NoPatricia* unicoFilho = it->second;

            no->prefixo += unicoFilho->prefixo;
            no->fimPalavra = unicoFilho->fimPalavra;
            no->filhos = std::move(unicoFilho->filhos);

            unicoFilho->filhos.clear();
            delete unicoFilho;
        }
    }

    bool removerAux(NoPatricia* pai, NoPatricia* atual, const std::string& palavra, char caractereAresta) {
        if (!atual) return false;

        size_t tamComum = tamanhoPrefixoComum(palavra, atual->prefixo);

        if (atual != raiz && tamComum < atual->prefixo.length()) {
            return false;
        }

        std::string restoPalavra = (atual == raiz) ? palavra : palavra.substr(tamComum);

        if (restoPalavra.empty()) {
            if (!atual->fimPalavra) return false;

            atual->fimPalavra = false;
            totalPalavras--;

            if (atual->filhos.empty() && atual != raiz) {
                delete atual;
                pai->filhos.erase(caractereAresta);
                fundirSePossivel(pai);
                return true;
            }

            if (atual->filhos.size() == 1 && atual != raiz) {
                fundirSePossivel(atual);
            }

            return true;
        }

        char proximoChar = restoPalavra[0];
        auto it = atual->filhos.find(proximoChar);
        if (it == atual->filhos.end()) {
            return false;
        }

        bool removido = removerAux(atual, it->second, restoPalavra, proximoChar);

        if (removido && atual != raiz) {
            fundirSePossivel(atual);
        }

        return removido;
    }

    void exportarJSONAux(NoPatricia* no, std::stringstream& ss) const {
        if (!no) return;
        ss << "{";
        ss << "\"id\":" << no->id << ",";
        ss << "\"prefixo\":\"" << (no == raiz ? "RAIZ" : no->prefixo) << "\",";
        ss << "\"fimPalavra\":" << (no->fimPalavra ? "true" : "false") << ",";
        ss << "\"filhos\":[";
        bool primeiro = true;
        for (const auto& par : no->filhos) {
            if (!primeiro) ss << ",";
            exportarJSONAux(par.second, ss);
            primeiro = false;
        }
        ss << "]}";
    }

    size_t contarNosAux(NoPatricia* no) const {
        if (!no) return 0;
        size_t total = 1;
        for (const auto& par : no->filhos) {
            total += contarNosAux(par.second);
        }
        return total;
    }

public:
    PatriciaTree() 
        : raiz(new NoPatricia("", false, 0)), contadorNos(0), totalPalavras(0),
          totalSplits(0), totalMerges(0) {}

    ~PatriciaTree() {
        delete raiz;
    }

    PatriciaTree(const PatriciaTree&) = delete;
    PatriciaTree& operator=(const PatriciaTree&) = delete;

    void insert(const std::string& palavra) {
        if (palavra.empty()) return;
        inserirAux(raiz, palavra);
    }

    bool search(const std::string& palavra) const {
        if (palavra.empty()) return false;
        return buscarAux(raiz, palavra);
    }

    bool remove(const std::string& palavra) {
        if (palavra.empty()) return false;
        return removerAux(nullptr, raiz, palavra, '\0');
    }

    size_t size() const {
        return totalPalavras;
    }

    size_t getNodeCount() const {
        return contarNosAux(raiz);
    }

    unsigned long long getSplitCount() const {
        return totalSplits;
    }

    unsigned long long getMergeCount() const {
        return totalMerges;
    }

    void resetMetrics() {
        totalSplits = 0;
        totalMerges = 0;
    }

    std::string exportarJSON() const {
        std::stringstream ss;
        ss << "{\"tipo\":\"Patricia\",\"totalPalavras\":" << totalPalavras 
           << ",\"totalNos\":" << getNodeCount() 
           << ",\"splits\":" << totalSplits 
           << ",\"merges\":" << totalMerges << ",\"arvore\":";
        exportarJSONAux(raiz, ss);
        ss << "}";
        return ss.str();
    }
};

#endif // PATRICIA_HPP
