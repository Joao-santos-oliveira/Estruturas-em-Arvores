#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP

#include <iostream>
#include <algorithm>
#include <sstream>

template <typename T>
struct NoAVL {
    T chave;
    int altura;
    NoAVL* esq;
    NoAVL* dir;
    int id;

    NoAVL(const T& k, int idNo = 0)
        : chave(k), altura(1), esq(nullptr), dir(nullptr), id(idNo) {}
};

template <typename T>
class AVLTree {
private:
    NoAVL<T>* raiz;
    int contadorNos;
    size_t totalElementos;
    unsigned long long totalRotacoes;

    int getAltura(NoAVL<T>* no) const {
        return no ? no->altura : 0;
    }

    int getFatorBalanceamento(NoAVL<T>* no) const {
        return no ? getAltura(no->esq) - getAltura(no->dir) : 0;
    }

    void atualizarAltura(NoAVL<T>* no) {
        if (no) {
            no->altura = 1 + std::max(getAltura(no->esq), getAltura(no->dir));
        }
    }

    NoAVL<T>* rotacionarDir(NoAVL<T>* y) {
        NoAVL<T>* x = y->esq;
        NoAVL<T>* temp = x->dir;

        x->dir = y;
        y->esq = temp;

        atualizarAltura(y);
        atualizarAltura(x);

        totalRotacoes++;
        return x;
    }

    NoAVL<T>* rotacionarEsq(NoAVL<T>* x) {
        NoAVL<T>* y = x->dir;
        NoAVL<T>* temp = y->esq;

        y->esq = x;
        x->dir = temp;

        atualizarAltura(x);
        atualizarAltura(y);

        totalRotacoes++;
        return y;
    }

    NoAVL<T>* inserirAux(NoAVL<T>* no, const T& chave, bool& inserido) {
        if (!no) {
            inserido = true;
            totalElementos++;
            return new NoAVL<T>(chave, ++contadorNos);
        }

        if (chave == no->chave) {
            inserido = false;
            return no;
        }

        if (chave < no->chave) {
            no->esq = inserirAux(no->esq, chave, inserido);
        } else {
            no->dir = inserirAux(no->dir, chave, inserido);
        }

        atualizarAltura(no);
        int fb = getFatorBalanceamento(no);

        if (fb > 1 && chave < no->esq->chave) {
            return rotacionarDir(no);
        }
        if (fb < -1 && chave > no->dir->chave) {
            return rotacionarEsq(no);
        }
        if (fb > 1 && chave > no->esq->chave) {
            no->esq = rotacionarEsq(no->esq);
            return rotacionarDir(no);
        }
        if (fb < -1 && chave < no->dir->chave) {
            no->dir = rotacionarDir(no->dir);
            return rotacionarEsq(no);
        }

        return no;
    }

    NoAVL<T>* encontrarMinimo(NoAVL<T>* no) const {
        while (no && no->esq) no = no->esq;
        return no;
    }

    NoAVL<T>* removerAux(NoAVL<T>* no, const T& chave, bool& removido) {
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
            if (!no->esq || !no->dir) {
                NoAVL<T>* temp = no->esq ? no->esq : no->dir;
                if (!temp) {
                    temp = no;
                    no = nullptr;
                } else {
                    *no = *temp;
                }
                delete temp;
                totalElementos--;
            } else {
                NoAVL<T>* sucessor = encontrarMinimo(no->dir);
                no->chave = sucessor->chave;
                no->dir = removerAux(no->dir, sucessor->chave, removido);
            }
        }

        if (!no) return nullptr;

        atualizarAltura(no);
        int fb = getFatorBalanceamento(no);

        if (fb > 1 && getFatorBalanceamento(no->esq) >= 0) {
            return rotacionarDir(no);
        }
        if (fb > 1 && getFatorBalanceamento(no->esq) < 0) {
            no->esq = rotacionarEsq(no->esq);
            return rotacionarDir(no);
        }
        if (fb < -1 && getFatorBalanceamento(no->dir) <= 0) {
            return rotacionarEsq(no);
        }
        if (fb < -1 && getFatorBalanceamento(no->dir) > 0) {
            no->dir = rotacionarDir(no->dir);
            return rotacionarEsq(no);
        }

        return no;
    }

    bool buscarAux(NoAVL<T>* no, const T& chave) const {
        if (!no) return false;
        if (no->chave == chave) return true;
        if (chave < no->chave) return buscarAux(no->esq, chave);
        return buscarAux(no->dir, chave);
    }

    void destruirRecursivo(NoAVL<T>* no) {
        if (!no) return;
        destruirRecursivo(no->esq);
        destruirRecursivo(no->dir);
        delete no;
    }

    void exportarJSONAux(NoAVL<T>* no, std::stringstream& ss) const {
        if (!no) {
            ss << "null";
            return;
        }
        ss << "{";
        ss << "\"id\":" << no->id << ",";
        ss << "\"chave\":" << no->chave << ",";
        ss << "\"altura\":" << no->altura << ",";
        ss << "\"fb\":" << getFatorBalanceamento(no) << ",";
        ss << "\"esq\":";
        exportarJSONAux(no->esq, ss);
        ss << ",\"dir\":";
        exportarJSONAux(no->dir, ss);
        ss << "}";
    }

public:
    AVLTree() : raiz(nullptr), contadorNos(0), totalElementos(0), totalRotacoes(0) {}

    ~AVLTree() {
        destruirRecursivo(raiz);
    }

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(const T& chave) {
        bool inserido = false;
        raiz = inserirAux(raiz, chave, inserido);
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

    int height() const {
        return getAltura(raiz);
    }

    unsigned long long getRotationCount() const {
        return totalRotacoes;
    }

    void resetMetrics() {
        totalRotacoes = 0;
    }

    std::string exportarJSON() const {
        std::stringstream ss;
        ss << "{\"tipo\":\"AVLTree\",\"totalElementos\":" << totalElementos 
           << ",\"altura\":" << height()
           << ",\"rotacoes\":" << totalRotacoes << ",\"arvore\":";
        exportarJSONAux(raiz, ss);
        ss << "}";
        return ss.str();
    }
};

#endif // AVL_TREE_HPP
