#ifndef SPLAY_TREE_HPP
#define SPLAY_TREE_HPP

#include <iostream>
#include <sstream>
#include <string>

template <typename T>
struct NoSplay {
    T chave;
    NoSplay* esq;
    NoSplay* dir;
    int id;

    NoSplay(const T& k, int idNo = 0)
        : chave(k), esq(nullptr), dir(nullptr), id(idNo) {}
};

template <typename T>
class SplayTree {
private:
    NoSplay<T>* raiz;
    int contadorNos;
    size_t totalElementos;
    unsigned long long totalRotacoes;
    unsigned long long totalZigZig;
    unsigned long long totalZigZag;
    unsigned long long totalZig;

    NoSplay<T>* rotacionarDir(NoSplay<T>* p) {
        NoSplay<T>* q = p->esq;
        p->esq = q->dir;
        q->dir = p;
        totalRotacoes++;
        return q;
    }

    NoSplay<T>* rotacionarEsq(NoSplay<T>* p) {
        NoSplay<T>* q = p->dir;
        p->dir = q->esq;
        q->esq = p;
        totalRotacoes++;
        return q;
    }

    NoSplay<T>* splay(NoSplay<T>* no, const T& chave) {
        if (!no || no->chave == chave) return no;

        if (chave < no->chave) {
            if (!no->esq) return no;

            // Caso Zig-Zig (Esq-Esq)
            if (chave < no->esq->chave) {
                totalZigZig++;
                no->esq->esq = splay(no->esq->esq, chave);
                no = rotacionarDir(no);
            }
            // Caso Zig-Zag (Esq-Dir)
            else if (chave > no->esq->chave) {
                totalZigZag++;
                no->esq->dir = splay(no->esq->dir, chave);
                if (no->esq->dir) {
                    no->esq = rotacionarEsq(no->esq);
                }
            }

            if (no->esq == nullptr) return no;
            totalZig++;
            return rotacionarDir(no);
        } else {
            if (!no->dir) return no;

            // Caso Zig-Zag (Dir-Esq)
            if (chave < no->dir->chave) {
                totalZigZag++;
                no->dir->esq = splay(no->dir->esq, chave);
                if (no->dir->esq) {
                    no->dir = rotacionarDir(no->dir);
                }
            }
            // Caso Zig-Zig (Dir-Dir)
            else if (chave > no->dir->chave) {
                totalZigZig++;
                no->dir->dir = splay(no->dir->dir, chave);
                no = rotacionarEsq(no);
            }

            if (no->dir == nullptr) return no;
            totalZig++;
            return rotacionarEsq(no);
        }
    }

    void destruirRecursivo(NoSplay<T>* no) {
        if (!no) return;
        destruirRecursivo(no->esq);
        destruirRecursivo(no->dir);
        delete no;
    }

    void exportarJSONAux(NoSplay<T>* no, std::stringstream& ss) const {
        if (!no) {
            ss << "null";
            return;
        }
        ss << "{";
        ss << "\"id\":" << no->id << ",";
        ss << "\"chave\":" << no->chave << ",";
        ss << "\"esq\":";
        exportarJSONAux(no->esq, ss);
        ss << ",\"dir\":";
        exportarJSONAux(no->dir, ss);
        ss << "}";
    }

    int calcularAlturaAux(NoSplay<T>* no) const {
        if (!no) return 0;
        return 1 + std::max(calcularAlturaAux(no->esq), calcularAlturaAux(no->dir));
    }

public:
    SplayTree() 
        : raiz(nullptr), contadorNos(0), totalElementos(0), 
          totalRotacoes(0), totalZigZig(0), totalZigZag(0), totalZig(0) {}

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

        if (raiz->chave == chave) return;

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

        if (raiz->chave != chave) return false;

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

    int height() const {
        return calcularAlturaAux(raiz);
    }

    unsigned long long getRotationCount() const {
        return totalRotacoes;
    }

    void resetMetrics() {
        totalRotacoes = 0;
        totalZigZig = 0;
        totalZigZag = 0;
        totalZig = 0;
    }

    const NoSplay<T>* getRoot() const {
        return raiz;
    }

    std::string exportarJSON() const {
        std::stringstream ss;
        ss << "{\"tipo\":\"SplayTree\",\"totalElementos\":" << totalElementos 
           << ",\"altura\":" << height()
           << ",\"rotacoes\":" << totalRotacoes
           << ",\"zigZig\":" << totalZigZig
           << ",\"zigZag\":" << totalZigZag
           << ",\"arvore\":";
        exportarJSONAux(raiz, ss);
        ss << "}";
        return ss.str();
    }
};

#endif // SPLAY_TREE_HPP
