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
    std::string prefixo;                 // Trecho de texto/prefixo armazenado no nó
    bool fimPalavra;                     // Se finaliza uma palavra inserida
    std::map<char, NoPatricia*> filhos;  // Filhos indexados pelo 1º caractere do prefixo
    int id;                              // Identificador para visualização Graphviz

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
 * @brief Implementação da Árvore Patricia.
 */
class PatriciaTree {
private:
    NoPatricia* raiz;
    int contadorNos;
    size_t totalPalavras;

    // Retorna o tamanho do prefixo comum entre duas strings
    static size_t tamanhoPrefixoComum(const std::string& a, const std::string& b) {
        size_t tam = 0;
        size_t limite = std::min(a.length(), b.length());
        while (tam < limite && a[tam] == b[tam]) {
            tam++;
        }
        return tam;
    }

    // Inserção com divisão de nó (Split) quando há divergência de caracteres
    void inserirAux(NoPatricia* noAtual, const std::string& palavra) {
        if (palavra.empty()) return;

        char primCaractere = palavra[0];
        auto it = noAtual->filhos.find(primCaractere);

        // Caso 1: Não existe aresta iniciando com este caractere
        if (it == noAtual->filhos.end()) {
            noAtual->filhos[primCaractere] = new NoPatricia(palavra, true, ++contadorNos);
            totalPalavras++;
            return;
        }

        NoPatricia* filho = it->second;
        size_t tamComum = tamanhoPrefixoComum(palavra, filho->prefixo);

        // Caso 2: A palavra coincide exatamente com o prefixo do filho
        if (tamComum == palavra.length() && tamComum == filho->prefixo.length()) {
            if (!filho->fimPalavra) {
                filho->fimPalavra = true;
                totalPalavras++;
            }
            return;
        }

        // Caso 3: O prefixo do filho é prefixo próprio da palavra (continua recursão)
        if (tamComum == filho->prefixo.length()) {
            std::string restoPalavra = palavra.substr(tamComum);
            inserirAux(filho, restoPalavra);
            return;
        }

        // Caso 4: Divergência parcial -> Divisão de Nó (Split)
        std::string parteComum = filho->prefixo.substr(0, tamComum);
        std::string restoFilho = filho->prefixo.substr(tamComum);

        NoPatricia* noDivisao = new NoPatricia(parteComum, false, ++contadorNos);
        
        // Ajusta o filho antigo para conter apenas o restante e vira filho do nó intermediário
        filho->prefixo = restoFilho;
        noDivisao->filhos[restoFilho[0]] = filho;

        // Atualiza a tabela do pai para apontar para o novo nó intermediário
        noAtual->filhos[primCaractere] = noDivisao;

        // Se a palavra inteira foi consumida na parte comum
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

    // Fusão (Merge) de nós internos com 1 único filho após remoção
    void fundirSePossivel(NoPatricia* no) {
        if (!no->fimPalavra && no->filhos.size() == 1 && no != raiz) {
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

    void exportarDOTAux(NoPatricia* no, std::stringstream& ss) const {
        if (!no) return;

        std::string rotulo = (no == raiz) ? "RAIZ" : "\"" + no->prefixo + "\"";

        if (no == raiz) {
            ss << "    node_" << no->id << " [label=\"RAIZ\", shape=circle, style=filled, fillcolor=\"#E0E0E0\"];\n";
        } else if (no->fimPalavra) {
            ss << "    node_" << no->id << " [label=" << rotulo << ", shape=doublecircle, style=filled, fillcolor=\"#A5D6A7\", color=\"#2E7D32\", penwidth=2.0];\n";
        } else {
            ss << "    node_" << no->id << " [label=" << rotulo << ", shape=ellipse, style=filled, fillcolor=\"#BBDEFB\", color=\"#1565C0\"];\n";
        }

        for (const auto& par : no->filhos) {
            NoPatricia* filho = par.second;
            ss << "    node_" << no->id << " -> node_" << filho->id 
               << " [label=\" [" << filho->prefixo[0] << "]\", fontcolor=\"#C2185B\", penwidth=1.5];\n";
            exportarDOTAux(filho, ss);
        }
    }

public:
    PatriciaTree() : raiz(new NoPatricia("", false, 0)), contadorNos(0), totalPalavras(0) {}

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

    std::string exportDOT(const std::string& titulo = "Patricia Tree (Radix Compacta)") const {
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

#endif // PATRICIA_HPP
