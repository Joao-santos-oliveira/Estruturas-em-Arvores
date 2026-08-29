#ifndef TREAP_HPP
#define TREAP_HPP

#include <iostream>
#include <sstream>
#include <random>
#include <string>

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

template <typename T>
class Treap {
private:
    NoTreap<T>* raiz;
    int contadorNos;
    size_t totalElementos;
    unsigned long long totalRotacoes;
    std::mt19937 gerador;
    std::uniform_int_distribution<int> distrib;

    NoTreap<T>* rotacionarDir(NoTreap<T>* y) {
        NoTreap<T>* x = y->esq;
        y->esq = x->dir;
        x->dir = y;
        totalRotacoes++;
        return x;
    }

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
            if (no->esq && no->esq->prioridade > no->prioridade) {
                no = rotacionarDir(no);
            }
        } else {
            no->dir = inserirAux(no->dir, chave, prioridade, inserido);
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

            if (!no->esq && !no->dir) {
                delete no;
                totalElementos--;
                return nullptr;
            } else if (!no->esq) {
                no = rotacionarEsq(no);
                no->esq = removerAux(no->esq, chave, removido);
            } else if (!no->dir) {
                no = rotacionarDir(no);
                no->dir = removerAux(no->dir, chave, removido);
            } else {
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

    void exportarJSONAux(NoTreap<T>* no, std::stringstream& ss) const {
        if (!no) {
            ss << "null";
            return;
        }
        ss << "{";
        ss << "\"id\":" << no->id << ",";
        ss << "\"chave\":" << no->chave << ",";
        ss << "\"prioridade\":" << no->prioridade << ",";
        ss << "\"esq\":";
        exportarJSONAux(no->esq, ss);
        ss << ",\"dir\":";
        exportarJSONAux(no->dir, ss);
        ss << "}";
    }

    int calcularAlturaAux(NoTreap<T>* no) const {
        if (!no) return 0;
        return 1 + std::max(calcularAlturaAux(no->esq), calcularAlturaAux(no->dir));
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

    int height() const {
        return calcularAlturaAux(raiz);
    }

    unsigned long long getRotationCount() const {
        return totalRotacoes;
    }

    void resetMetrics() {
        totalRotacoes = 0;
    }

    std::string exportarJSON() const {
        std::stringstream ss;
        ss << "{\"tipo\":\"Treap\",\"totalElementos\":" << totalElementos 
           << ",\"altura\":" << height()
           << ",\"rotacoes\":" << totalRotacoes
           << ",\"arvore\":";
        exportarJSONAux(raiz, ss);
        ss << "}";
        return ss.str();
    }
};

#endif // TREAP_HPP
