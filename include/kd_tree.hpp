#ifndef KD_TREE_HPP
#define KD_TREE_HPP

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <limits>
#include <array>

/**
 * @brief Estrutura representando um Ponto em K dimensões.
 */
template <size_t K, typename TipoCoord = double>
struct Ponto {
    std::array<TipoCoord, K> coords;

    Ponto() {
        coords.fill(0);
    }

    Ponto(std::initializer_list<TipoCoord> lista) {
        size_t i = 0;
        for (auto v : lista) {
            if (i < K) coords[i++] = v;
        }
        while (i < K) coords[i++] = 0;
    }

    TipoCoord operator[](size_t idx) const {
        return coords[idx];
    }

    TipoCoord& operator[](size_t idx) {
        return coords[idx];
    }

    bool operator==(const Ponto<K, TipoCoord>& outro) const {
        for (size_t i = 0; i < K; ++i) {
            if (coords[i] != outro.coords[i]) return false;
        }
        return true;
    }

    double distanciaQuadrada(const Ponto<K, TipoCoord>& outro) const {
        double soma = 0.0;
        for (size_t i = 0; i < K; ++i) {
            double dif = static_cast<double>(coords[i]) - static_cast<double>(outro.coords[i]);
            soma += dif * dif;
        }
        return soma;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "(";
        for (size_t i = 0; i < K; ++i) {
            ss << coords[i] << (i + 1 < K ? ", " : "");
        }
        ss << ")";
        return ss.str();
    }
};

/**
 * @brief Nó da KD-Tree.
 */
template <size_t K, typename TipoCoord = double>
struct NoKD {
    Ponto<K, TipoCoord> ponto;
    int eixo; // Eixo de corte (0 = X, 1 = Y, ...)
    NoKD* esq;
    NoKD* dir;
    int id;

    NoKD(const Ponto<K, TipoCoord>& pt, int eixoCorte, int idNo = 0)
        : ponto(pt), eixo(eixoCorte), esq(nullptr), dir(nullptr), id(idNo) {}
};

/**
 * @brief KD-Tree para indexação espacial e busca multidimensional.
 */
template <size_t K, typename TipoCoord = double>
class KDTree {
private:
    NoKD<K, TipoCoord>* raiz;
    int contadorNos;
    size_t totalPontos;
    mutable unsigned long long calculosDistancia;

    NoKD<K, TipoCoord>* inserirAux(NoKD<K, TipoCoord>* no, const Ponto<K, TipoCoord>& pt, int prof, bool& inserido) {
        if (!no) {
            inserido = true;
            totalPontos++;
            return new NoKD<K, TipoCoord>(pt, prof % K, ++contadorNos);
        }

        if (no->ponto == pt) {
            inserido = false;
            return no;
        }

        int eixo = prof % K;
        if (pt[eixo] < no->ponto[eixo]) {
            no->esq = inserirAux(no->esq, pt, prof + 1, inserido);
        } else {
            no->dir = inserirAux(no->dir, pt, prof + 1, inserido);
        }

        return no;
    }

    bool buscarAux(NoKD<K, TipoCoord>* no, const Ponto<K, TipoCoord>& pt, int prof) const {
        if (!no) return false;
        if (no->ponto == pt) return true;

        int eixo = prof % K;
        if (pt[eixo] < no->ponto[eixo]) {
            return buscarAux(no->esq, pt, prof + 1);
        } else {
            return buscarAux(no->dir, pt, prof + 1);
        }
    }

    void buscaIntervaloAux(NoKD<K, TipoCoord>* no, 
                           const Ponto<K, TipoCoord>& minPt, 
                           const Ponto<K, TipoCoord>& maxPt,
                           std::vector<Ponto<K, TipoCoord>>& resultado) const {
        if (!no) return;

        bool dentro = true;
        for (size_t i = 0; i < K; ++i) {
            if (no->ponto[i] < minPt[i] || no->ponto[i] > maxPt[i]) {
                dentro = false;
                break;
            }
        }
        if (dentro) {
            resultado.push_back(no->ponto);
        }

        int eixo = no->eixo;
        if (minPt[eixo] <= no->ponto[eixo]) {
            buscaIntervaloAux(no->esq, minPt, maxPt, resultado);
        }
        if (maxPt[eixo] >= no->ponto[eixo]) {
            buscaIntervaloAux(no->dir, minPt, maxPt, resultado);
        }
    }

    void vizinhoMaisProximoAux(NoKD<K, TipoCoord>* no,
                              const Ponto<K, TipoCoord>& alvo,
                              NoKD<K, TipoCoord>*& melhorNo,
                              double& menorDistQuad) const {
        if (!no) return;

        calculosDistancia++;
        double distAtualQuad = no->ponto.distanciaQuadrada(alvo);

        if (distAtualQuad < menorDistQuad) {
            menorDistQuad = distAtualQuad;
            melhorNo = no;
        }

        int eixo = no->eixo;
        double dif = static_cast<double>(alvo[eixo]) - static_cast<double>(no->ponto[eixo]);

        NoKD<K, TipoCoord>* primario = (dif < 0) ? no->esq : no->dir;
        NoKD<K, TipoCoord>* secundario = (dif < 0) ? no->dir : no->esq;

        vizinhoMaisProximoAux(primario, alvo, melhorNo, menorDistQuad);

        // Poda: só explora o outro lado se a distância ao hiperplano for menor que o raio atual
        if (dif * dif < menorDistQuad) {
            vizinhoMaisProximoAux(secundario, alvo, melhorNo, menorDistQuad);
        }
    }

    void destruirRecursivo(NoKD<K, TipoCoord>* no) {
        if (!no) return;
        destruirRecursivo(no->esq);
        destruirRecursivo(no->dir);
        delete no;
    }

    void exportarDOTAux(NoKD<K, TipoCoord>* no, std::stringstream& ss) const {
        if (!no) return;

        const char nomesEixos[] = {'X', 'Y', 'Z', 'W'};
        char charEixo = (no->eixo < 4) ? nomesEixos[no->eixo] : ('0' + no->eixo);

        std::string corFundo = (no == raiz) ? "#FFE0B2" : "#B2DFDB";
        std::string corBorda = (no == raiz) ? "#E65100" : "#00695C";

        ss << "    node_" << no->id << " [label=\"" << no->ponto.toString() << "\\n(eixo: " << charEixo 
           << ")\", shape=box, style=filled, fillcolor=\"" << corFundo << "\", color=\"" << corBorda << "\", penwidth=2.0];\n";

        if (no->esq) {
            ss << "    node_" << no->id << " -> node_" << no->esq->id << " [label=\"<=" << charEixo << "\"];\n";
            exportarDOTAux(no->esq, ss);
        } else {
            ss << "    null_l_" << no->id << " [shape=point, width=0.1];\n";
            ss << "    node_" << no->id << " -> null_l_" << no->id << " [style=dashed];\n";
        }

        if (no->dir) {
            ss << "    node_" << no->id << " -> node_" << no->dir->id << " [label=\">" << charEixo << "\"];\n";
            exportarDOTAux(no->dir, ss);
        } else {
            ss << "    null_r_" << no->id << " [shape=point, width=0.1];\n";
            ss << "    node_" << no->id << " -> null_r_" << no->id << " [style=dashed];\n";
        }
    }

public:
    KDTree() : raiz(nullptr), contadorNos(0), totalPontos(0), calculosDistancia(0) {}

    ~KDTree() {
        destruirRecursivo(raiz);
    }

    KDTree(const KDTree&) = delete;
    KDTree& operator=(const KDTree&) = delete;

    bool insert(const Ponto<K, TipoCoord>& pt) {
        bool inserido = false;
        raiz = inserirAux(raiz, pt, 0, inserido);
        return inserido;
    }

    bool search(const Ponto<K, TipoCoord>& pt) const {
        return buscarAux(raiz, pt, 0);
    }

    std::vector<Ponto<K, TipoCoord>> rangeSearch(const Ponto<K, TipoCoord>& minPt, const Ponto<K, TipoCoord>& maxPt) const {
        std::vector<Ponto<K, TipoCoord>> resultado;
        buscaIntervaloAux(raiz, minPt, maxPt, resultado);
        return resultado;
    }

    Ponto<K, TipoCoord> nearestNeighbor(const Ponto<K, TipoCoord>& alvo) const {
        if (!raiz) {
            throw std::runtime_error("KD-Tree vazia.");
        }
        NoKD<K, TipoCoord>* melhorNo = nullptr;
        double menorDistQuad = std::numeric_limits<double>::infinity();
        vizinhoMaisProximoAux(raiz, alvo, melhorNo, menorDistQuad);
        return melhorNo->ponto;
    }

    size_t size() const {
        return totalPontos;
    }

    unsigned long long getDistanceComputations() const {
        return calculosDistancia;
    }

    void resetMetrics() {
        calculosDistancia = 0;
    }

    std::string exportDOT(const std::string& titulo = "KD-Tree") const {
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

#endif // KD_TREE_HPP
