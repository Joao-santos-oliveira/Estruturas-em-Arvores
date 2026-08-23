#ifndef TREAP_HPP
#define TREAP_HPP

#include <iostream>
#include <sstream>
#include <random>
#include <string>

/**
 * @brief Nó da Árvore Treap (BST + Max-Heap).
 */
template <typename T>
struct NoTreap {
    T chave;
    int prioridade;
    NoTreap* esq;
    NoTreap* dir;
    int id;

    NoTreap(const T& k, int p, int idNo = 0)
        : chave(k), prioridade(p), esq(nullptr), dir(nullptr), id(idNo) {}
};

/**
 * @brief Árvore Treap (Randomized Search Tree).
 */
template <typename T>
class Treap {
private:
    NoTreap<T>* raiz;
    int contadorNos;
    size_t totalElementos;
    unsigned long long totalRotacoes;
    std::mt19937 gerador;
    std::uniform_int_distribution<int> distrib;

    // Rotação simples à direita
    NoTreap<T>* rotacionarDir(NoTreap<T>* y) {
        NoTreap<T>* x = y->esq;
        y->esq = x->dir;
        x->dir = y;
        totalRotacoes++;
        return x;
    }

    // Rotação simples à esquerda
    NoTreap<T>* rotacionarEsq(NoTreap<T>* x) {
        NoTreap<T>* y = x->dir;
        x->dir = y->esq;
        y->esq = x;
        totalRotacoes++;
        return y;
    }

    NoTreap<T>* inserirAux(NoTreap<T>* no, const T& chave, int prioridade, bool& inserido) {
        if (!no) {
            inserido = true;
            totalElementos++;
            return new NoTreap<T>(chave, prioridade, ++contadorNos);
        }

        if (chave == no->chave) {
            inserido = false;
            return no;
        }

        if (chave < no->chave) {
            no->esq = inserirAux(no->esq, chave, prioridade, inserido);
            // Corrige propriedade de Max-Heap se a prioridade do filho for maior que a do pai
            if (no->esq && no->esq->prioridade > no->prioridade) {
                no = rotacionarDir(no);
            }
        } else {
            no->dir = inserirAux(no->dir, chave, prioridade, inserido);
            // Corrige propriedade de Max-Heap se a prioridade do filho for maior que a do pai
            if (no->dir && no->dir->prioridade > no->prioridade) {
                no = rotacionarEsq(no);
            }
        }

        return no;
    }

    NoTreap<T>* removerAux(NoTreap<T>* no, const T& chave, bool& removido) {
        if (!no) {
            removido = false;
            return nullptr;
        }

        if (chave < no->chave) {
            no->esq = removerAux(no->esq, chave, removido);
        } else if (chave > no->chave) {
            no->dir = removerAux(no->dir, chave, removido);
        } else {
            removido = true;

            // Caso 1: Nó folha
            if (!no->esq && !no->dir) {
                delete no;
                totalElementos--;
                return nullptr;
            }
            // Caso 2: Somente filho direito
            else if (!no->esq) {
                no = rotacionarEsq(no);
                no->esq = removerAux(no->esq, chave, removido);
            }
            // Caso 3: Somente filho esquerdo
            else if (!no->dir) {
                no = rotacionarDir(no);
                no->dir = removerAux(no->dir, chave, removido);
            }
            // Caso 4: Ambos os filhos -> rotaciona o filho de maior prioridade para cima
            else {
                if (no->esq->prioridade > no->dir->prioridade) {
                    no = rotacionarDir(no);
                    no->dir = removerAux(no->dir, chave, removido);
                } else {
                    no = rotacionarEsq(no);
                    no->esq = removerAux(no->esq, chave, removido);
                }
            }
        }

        return no;
    }

    bool buscarAux(NoTreap<T>* no, const T& chave) const {
        if (!no) return false;
        if (no->chave == chave) return true;
        if (chave < no->chave) return buscarAux(no->esq, chave);
        return buscarAux(no->dir, chave);
    }

    void destruirRecursivo(NoTreap<T>* no) {
        if (!no) return;
        destruirRecursivo(no->esq);
        destruirRecursivo(no->dir);
        delete no;
    }

    void exportarDOTAux(NoTreap<T>* no, std::stringstream& ss) const {
        if (!no) return;

        std::string corFundo = (no == raiz) ? "#C8E6C9" : "#DCEDC8";
        std::string corBorda = (no == raiz) ? "#2E7D32" : "#558B2F";

        ss << "    node_" << no->id << " [label=\"" << no->chave << " | p=" << no->prioridade 
           << "\", shape=record, style=filled, fillcolor=\"" << corFundo << "\", color=\"" << corBorda << "\", penwidth=2.0];\n";

        if (no->esq) {
            ss << "    node_" << no->id << " -> node_" << no->esq->id << " [label=\"Esq\"];\n";
            exportarDOTAux(no->esq, ss);
        } else {
            ss << "    null_l_" << no->id << " [shape=point, width=0.1];\n";
            ss << "    node_" << no->id << " -> null_l_" << no->id << " [style=dashed];\n";
        }

        if (no->dir) {
            ss << "    node_" << no->id << " -> node_" << no->dir->id << " [label=\"Dir\"];\n";
            exportarDOTAux(no->dir, ss);
        } else {
            ss << "    null_r_" << no->id << " [shape=point, width=0.1];\n";
            ss << "    node_" << no->id << " -> null_r_" << no->id << " [style=dashed];\n";
        }
    }

public:
    Treap(unsigned int semente = 42) 
        : raiz(nullptr), contadorNos(0), totalElementos(0), totalRotacoes(0),
          gerador(semente), distrib(1, 1000000) {}

    ~Treap() {
        destruirRecursivo(raiz);
    }

    Treap(const Treap&) = delete;
    Treap& operator=(const Treap&) = delete;

    bool insert(const T& chave) {
        return insertWithPriority(chave, distrib(gerador));
    }

    bool insertWithPriority(const T& chave, int prioridade) {
        bool inserido = false;
        raiz = inserirAux(raiz, chave, prioridade, inserido);
        return inserido;
    }

    bool search(const T& chave) const {
        return buscarAux(raiz, chave);
    }

    bool remove(const T& chave) {
        bool removido = false;
        raiz = removerAux(raiz, chave, removido);
        return removido;
    }

    size_t size() const {
        return totalElementos;
    }

    unsigned long long getRotationCount() const {
        return totalRotacoes;
    }

    void resetMetrics() {
        totalRotacoes = 0;
    }

    std::string exportDOT(const std::string& titulo = "Treap (BST + Heap)") const {
        std::stringstream ss;
        ss << "digraph \"" << titulo << "\" {\n";
        ss << "    rankdir=TB;\n";
        ss << "    node [fontsize=12, fontname=\"Arial\"];\n";
        ss << "    edge [fontsize=10, fontname=\"Arial\"];\n";
        ss << "    labelloc=\"t\";\n";
        ss << "    label=\"" << titulo << "\";\n";

        if (raiz) {
            exportarDOTAux(raiz, ss);
        } else {
            ss << "    empty [label=\"Vazia\", shape=none];\n";
        }

        ss << "}\n";
        return ss.str();
    }
};

#endif // TREAP_HPP
