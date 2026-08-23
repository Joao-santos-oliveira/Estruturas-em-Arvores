#ifndef SPLAY_TREE_HPP
#define SPLAY_TREE_HPP

#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief Nó da Árvore Splay.
 */
template <typename T>
struct NoSplay {
    T chave;
    NoSplay* esq;
    NoSplay* dir;
    int id;

    NoSplay(const T& k, int idNo = 0)
        : chave(k), esq(nullptr), dir(nullptr), id(idNo) {}
};

/**
 * @brief Árvore Splay (BST Autoajustável).
 */
template <typename T>
class SplayTree {
private:
    NoSplay<T>* raiz;
    int contadorNos;
    size_t totalElementos;
    unsigned long long totalRotacoes;

    // Rotação simples à direita
    NoSplay<T>* rotacionarDir(NoSplay<T>* p) {
        NoSplay<T>* q = p->esq;
        p->esq = q->dir;
        q->dir = p;
        totalRotacoes++;
        return q;
    }

    // Rotação simples à esquerda
    NoSplay<T>* rotacionarEsq(NoSplay<T>* p) {
        NoSplay<T>* q = p->dir;
        p->dir = q->esq;
        q->esq = p;
        totalRotacoes++;
        return q;
    }

    /**
     * @brief Operação central Splay: move o nó acessado para a raiz
     * aplicando os passos Zig, Zig-Zig e Zig-Zag.
     */
    NoSplay<T>* splay(NoSplay<T>* no, const T& chave) {
        if (!no || no->chave == chave) return no;

        // Chave reside na subárvore esquerda
        if (chave < no->chave) {
            if (!no->esq) return no; // Chave ausente

            // Caso Zig-Zig (Esq-Esq): rotaciona primeiro o pai, depois o filho
            if (chave < no->esq->chave) {
                no->esq->esq = splay(no->esq->esq, chave);
                no = rotacionarDir(no);
            }
            // Caso Zig-Zag (Esq-Dir): rotaciona o filho à esq, depois o pai à dir
            else if (chave > no->esq->chave) {
                no->esq->dir = splay(no->esq->dir, chave);
                if (no->esq->dir) {
                    no->esq = rotacionarEsq(no->esq);
                }
            }

            return (no->esq == nullptr) ? no : rotacionarDir(no);
        }
        // Chave reside na subárvore direita
        else {
            if (!no->dir) return no; // Chave ausente

            // Caso Zig-Zag (Dir-Esq)
            if (chave < no->dir->chave) {
                no->dir->esq = splay(no->dir->esq, chave);
                if (no->dir->esq) {
                    no->dir = rotacionarDir(no->dir);
                }
            }
            // Caso Zig-Zig (Dir-Dir)
            else if (chave > no->dir->chave) {
                no->dir->dir = splay(no->dir->dir, chave);
                no = rotacionarEsq(no);
            }

            return (no->dir == nullptr) ? no : rotacionarEsq(no);
        }
    }

    void destruirRecursivo(NoSplay<T>* no) {
        if (!no) return;
        destruirRecursivo(no->esq);
        destruirRecursivo(no->dir);
        delete no;
    }

    void exportarDOTAux(NoSplay<T>* no, std::stringstream& ss) const {
        if (!no) return;

        std::string corFundo = (no == raiz) ? "#FFE082" : "#E1BEE7";
        std::string corBorda = (no == raiz) ? "#FF8F00" : "#8E24AA";

        ss << "    node_" << no->id << " [label=\"" << no->chave << "\", shape=circle, style=filled, fillcolor=\"" 
           << corFundo << "\", color=\"" << corBorda << "\", penwidth=2.0];\n";

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
    SplayTree() : raiz(nullptr), contadorNos(0), totalElementos(0), totalRotacoes(0) {}

    ~SplayTree() {
        destruirRecursivo(raiz);
    }

    SplayTree(const SplayTree&) = delete;
    SplayTree& operator=(const SplayTree&) = delete;

    void insert(const T& chave) {
        if (!raiz) {
            raiz = new NoSplay<T>(chave, ++contadorNos);
            totalElementos++;
            return;
        }

        raiz = splay(raiz, chave);

        if (raiz->chave == chave) {
            return; // Elemento já existente
        }

        NoSplay<T>* novoNo = new NoSplay<T>(chave, ++contadorNos);

        if (chave < raiz->chave) {
            novoNo->dir = raiz;
            novoNo->esq = raiz->esq;
            raiz->esq = nullptr;
        } else {
            novoNo->esq = raiz;
            novoNo->dir = raiz->dir;
            raiz->dir = nullptr;
        }

        raiz = novoNo;
        totalElementos++;
    }

    bool search(const T& chave) {
        if (!raiz) return false;
        raiz = splay(raiz, chave);
        return (raiz && raiz->chave == chave);
    }

    bool remove(const T& chave) {
        if (!raiz) return false;

        raiz = splay(raiz, chave);

        if (raiz->chave != chave) {
            return false;
        }

        NoSplay<T>* noParaRemover = raiz;

        if (!raiz->esq) {
            raiz = raiz->dir;
        } else {
            NoSplay<T>* subEsq = raiz->esq;
            NoSplay<T>* subDir = raiz->dir;

            subEsq = splay(subEsq, chave);
            subEsq->dir = subDir;
            raiz = subEsq;
        }

        delete noParaRemover;
        totalElementos--;
        return true;
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

    const NoSplay<T>* getRoot() const {
        return raiz;
    }

    std::string exportDOT(const std::string& titulo = "Splay Tree") const {
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

#endif // SPLAY_TREE_HPP
