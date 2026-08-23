#ifndef BST_HPP
#define BST_HPP

#include <iostream>
#include <sstream>
#include <string>

template <typename T>
struct NoBST {
    T chave;
    NoBST* esq;
    NoBST* dir;
    int id;

    NoBST(const T& k, int idNo = 0)
        : chave(k), esq(nullptr), dir(nullptr), id(idNo) {}
};

template <typename T>
class BST {
private:
    NoBST<T>* raiz;
    int contadorNos;
    size_t totalElementos;
    unsigned long long totalComparacoes;

    NoBST<T>* inserirAux(NoBST<T>* no, const T& chave, bool& inserido) {
        if (!no) {
            inserido = true;
            totalElementos++;
            return new NoBST<T>(chave, ++contadorNos);
        }

        totalComparacoes++;
        if (chave == no->chave) {
            inserido = false;
            return no;
        }

        if (chave < no->chave) {
            no->esq = inserirAux(no->esq, chave, inserido);
        } else {
            no->dir = inserirAux(no->dir, chave, inserido);
        }
        return no;
    }

    NoBST<T>* encontrarMinimo(NoBST<T>* no) const {
        while (no && no->esq) no = no->esq;
        return no;
    }

    NoBST<T>* removerAux(NoBST<T>* no, const T& chave, bool& removido) {
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
            if (!no->esq) {
                NoBST<T>* temp = no->dir;
                delete no;
                totalElementos--;
                return temp;
            } else if (!no->dir) {
                NoBST<T>* temp = no->esq;
                delete no;
                totalElementos--;
                return temp;
            }

            NoBST<T>* sucessor = encontrarMinimo(no->dir);
            no->chave = sucessor->chave;
            no->dir = removerAux(no->dir, sucessor->chave, removido);
        }
        return no;
    }

    bool buscarAux(NoBST<T>* no, const T& chave) {
        if (!no) return false;
        totalComparacoes++;
        if (no->chave == chave) return true;
        if (chave < no->chave) return buscarAux(no->esq, chave);
        return buscarAux(no->dir, chave);
    }

    void destruirRecursivo(NoBST<T>* no) {
        if (!no) return;
        destruirRecursivo(no->esq);
        destruirRecursivo(no->dir);
        delete no;
    }

public:
    BST() : raiz(nullptr), contadorNos(0), totalElementos(0), totalComparacoes(0) {}

    ~BST() {
        destruirRecursivo(raiz);
    }

    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    bool insert(const T& chave) {
        bool inserido = false;
        raiz = inserirAux(raiz, chave, inserido);
        return inserido;
    }

    bool search(const T& chave) {
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

    unsigned long long getComparisonCount() const {
        return totalComparacoes;
    }

    void resetMetrics() {
        totalComparacoes = 0;
    }
};

#endif // BST_HPP
