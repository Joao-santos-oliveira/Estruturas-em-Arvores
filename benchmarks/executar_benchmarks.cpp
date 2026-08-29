#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "trie.hpp"
#include "patricia.hpp"
#include "splay_tree.hpp"
#include "treap.hpp"
#include "kd_tree.hpp"
#include "bst.hpp"
#include "avl_tree.hpp"

using namespace std::chrono;

struct RegistroBenchmark {
    std::string categoria;
    std::string estrutura;
    std::string cenario;
    std::string operacao;
    size_t tamanhoN;
    double tempoMicrossegundos;
    double tempoMedioPorOpNanossegundos;
    unsigned long long metricaOperacoes; // Rotações, comparações, podas ou nós
    size_t contagemNos;
};

std::vector<RegistroBenchmark> resultados;

void salvarResultadosCSV(const std::string& caminhoCSV) {
    std::ofstream arq(caminhoCSV);
    if (!arq.is_open()) {
        std::cerr << "Erro ao salvar CSV: " << caminhoCSV << std::endl;
        return;
    }

    arq << "Categoria,Estrutura,Cenario,Operacao,TamanhoN,Tempo_us,TempoMedio_ns,Metrica_Operacoes,Contagem_Nos\n";
    for (const auto& r : resultados) {
        arq << r.categoria << ","
            << r.estrutura << ","
            << r.cenario << ","
            << r.operacao << ","
            << r.tamanhoN << ","
            << std::fixed << std::setprecision(2) << r.tempoMicrossegundos << ","
            << std::fixed << std::setprecision(2) << r.tempoMedioPorOpNanossegundos << ","
            << r.metricaOperacoes << ","
            << r.contagemNos << "\n";
    }
    arq.close();
    std::cout << "\n[BENCHMARK] Todos os dados experimentais salvos com sucesso em: " << caminhoCSV << std::endl;
}

std::vector<std::string> carregarArquivoLinhas(const std::string& caminho) {
    std::vector<std::string> linhas;
    std::ifstream arq(caminho);
    if (!arq.is_open()) return linhas;
    std::string l;
    while (std::getline(arq, l)) {
        if (!l.empty()) linhas.push_back(l);
    }
    return linhas;
}

std::vector<int> carregarArquivoInteiros(const std::string& caminho) {
    std::vector<int> vals;
    std::ifstream arq(caminho);
    if (!arq.is_open()) return vals;
    int v;
    while (arq >> v) vals.push_back(v);
    return vals;
}

// -----------------------------------------------------------------------------
// 1. BENCHMARK: STRINGS (TRIE vs. PATRICIA)
// -----------------------------------------------------------------------------
void benchmarkStrings() {
    std::cout << "\n==================================================" << std::endl;
    std::cout << " [1/3] EXECUTANDO BENCHMARK: TRIE vs. PATRICIA " << std::endl;
    std::cout << "==================================================" << std::endl;

    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};

    // 1.1 Prefixos Densos (Caso Real de Dicionário)
    for (size_t n : tamanhos) {
        std::string caminho = "benchmarks/datasets/1_strings/prefixos_densos_" + std::to_string(n) + ".txt";
        auto palavras = carregarArquivoLinhas(caminho);
        if (palavras.empty()) continue;

        // --- TRIE ---
        Trie trie;
        auto t0 = high_resolution_clock::now();
        for (const auto& p : palavras) trie.insert(p);
        auto t1 = high_resolution_clock::now();
        double tInsTrie = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Strings", "Trie", "Prefixos_Densos", "Insercao", n, tInsTrie, (tInsTrie * 1000.0) / n, 0, trie.getNodeCount()});

        t0 = high_resolution_clock::now();
        for (const auto& p : palavras) trie.search(p);
        t1 = high_resolution_clock::now();
        double tBuscaTrie = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Strings", "Trie", "Prefixos_Densos", "Busca_Sucesso", n, tBuscaTrie, (tBuscaTrie * 1000.0) / n, 0, trie.getNodeCount()});

        // --- PATRICIA ---
        PatriciaTree pat;
        t0 = high_resolution_clock::now();
        for (const auto& p : palavras) pat.insert(p);
        t1 = high_resolution_clock::now();
        double tInsPat = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Strings", "Patricia", "Prefixos_Densos", "Insercao", n, tInsPat, (tInsPat * 1000.0) / n, pat.getSplitCount(), pat.getNodeCount()});

        t0 = high_resolution_clock::now();
        for (const auto& p : palavras) pat.search(p);
        t1 = high_resolution_clock::now();
        double tBuscaPat = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Strings", "Patricia", "Prefixos_Densos", "Busca_Sucesso", n, tBuscaPat, (tBuscaPat * 1000.0) / n, 0, pat.getNodeCount()});

        double taxaCompactacao = (1.0 - (double)pat.getNodeCount() / (double)trie.getNodeCount()) * 100.0;
        std::cout << "  -> N = " << std::setw(6) << n 
                  << " | Trie Nos: " << std::setw(7) << trie.getNodeCount()
                  << " | Patricia Nos: " << std::setw(7) << pat.getNodeCount()
                  << " (" << std::fixed << std::setprecision(1) << taxaCompactacao << "% menos nos!)" << std::endl;
    }

    // 1.2 Pior Caso: Cadeia Linear Longa
    auto cadeiaPior = carregarArquivoLinhas("benchmarks/datasets/1_strings/pior_caso_cadeia_longa.txt");
    if (!cadeiaPior.empty()) {
        Trie triePior;
        PatriciaTree patPior;
        for (const auto& p : cadeiaPior) {
            triePior.insert(p);
            patPior.insert(p);
        }
        resultados.push_back({"Strings", "Trie", "Cadeia_Longa_120chars", "Insercao", cadeiaPior.size(), 0, 0, 0, triePior.getNodeCount()});
        resultados.push_back({"Strings", "Patricia", "Cadeia_Longa_120chars", "Insercao", cadeiaPior.size(), 0, 0, patPior.getSplitCount(), patPior.getNodeCount()});
        std::cout << "  [PIOR CASO STRINGS] String 120 chars -> Trie nos: " << triePior.getNodeCount() 
                  << " | Patricia nos: " << patPior.getNodeCount() << std::endl;
    }
}

// -----------------------------------------------------------------------------
// 2. BENCHMARK: NUMÉRICOS (SPLAY, TREAP, AVL, BST)
// -----------------------------------------------------------------------------
void benchmarkNumericos() {
    std::cout << "\n==================================================" << std::endl;
    std::cout << " [2/3] EXECUTANDO BENCHMARK: SPLAY vs. TREAP vs. AVL vs. BST " << std::endl;
    std::cout << "==================================================" << std::endl;

    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};

    // 2.1 Chaves Ordenadas (Pior caso clássico da BST)
    for (size_t n : tamanhos) {
        std::string caminho = "benchmarks/datasets/2_numericos/ordenado_crescente_" + std::to_string(n) + ".txt";
        auto chaves = carregarArquivoInteiros(caminho);
        if (chaves.empty()) continue;

        // Treap
        Treap<int> treap(42);
        auto t0 = high_resolution_clock::now();
        for (int v : chaves) treap.insert(v);
        auto t1 = high_resolution_clock::now();
        double tTreap = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Numericos", "Treap", "Ordenado_Crescente", "Insercao", n, tTreap, (tTreap * 1000.0) / n, treap.getRotationCount(), treap.size()});

        // AVL
        AVLTree<int> avl;
        t0 = high_resolution_clock::now();
        for (int v : chaves) avl.insert(v);
        t1 = high_resolution_clock::now();
        double tAVL = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Numericos", "AVL", "Ordenado_Crescente", "Insercao", n, tAVL, (tAVL * 1000.0) / n, avl.getRotationCount(), avl.size()});

        // Splay
        SplayTree<int> splay;
        t0 = high_resolution_clock::now();
        for (int v : chaves) splay.insert(v);
        t1 = high_resolution_clock::now();
        double tSplay = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Numericos", "Splay", "Ordenado_Crescente", "Insercao", n, tSplay, (tSplay * 1000.0) / n, splay.getRotationCount(), splay.size()});

        // BST (Apenas até N=10000 para evitar stack overflow por lista linear de profundidade 50k+)
        if (n <= 10000) {
            BST<int> bst;
            t0 = high_resolution_clock::now();
            for (int v : chaves) bst.insert(v);
            t1 = high_resolution_clock::now();
            double tBST = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
            resultados.push_back({"Numericos", "BST", "Ordenado_Crescente", "Insercao", n, tBST, (tBST * 1000.0) / n, bst.getComparisonCount(), bst.size()});
            std::cout << "  -> Ordenado N = " << std::setw(6) << n 
                      << " | Treap: " << std::setw(8) << tTreap << " us"
                      << " | AVL: " << std::setw(8) << tAVL << " us"
                      << " | BST (Degenerada): " << std::setw(10) << tBST << " us" << std::endl;
        } else {
            std::cout << "  -> Ordenado N = " << std::setw(6) << n 
                      << " | Treap: " << std::setw(8) << tTreap << " us"
                      << " | AVL: " << std::setw(8) << tAVL << " us"
                      << " | Splay: " << std::setw(8) << tSplay << " us (BST omitida para N>10k por degeneracao)" << std::endl;
        }
    }

    // 2.2 Localidade Temporal de Referência (Zipf 80-20 e 90-10)
    auto chaves50k = carregarArquivoInteiros("benchmarks/datasets/2_numericos/aleatorio_uniforme_50000.txt");
    auto buscasZipf80 = carregarArquivoInteiros("benchmarks/datasets/2_numericos/consultas_zipf_80_20_100k.txt");
    auto buscasZipf90 = carregarArquivoInteiros("benchmarks/datasets/2_numericos/consultas_zipf_90_10_100k.txt");

    if (!chaves50k.empty() && !buscasZipf80.empty()) {
        std::cout << "\n  [LOCALIDADE TEMPORAL] Carregando N = 50.000 elementos..." << std::endl;
        SplayTree<int> splay;
        AVLTree<int> avl;
        Treap<int> treap(42);

        for (int v : chaves50k) {
            splay.insert(v);
            avl.insert(v);
            treap.insert(v);
        }

        // Teste Zipf 80-20 (100.000 buscas)
        splay.resetMetrics();
        auto t0 = high_resolution_clock::now();
        for (int b : buscasZipf80) splay.search(b);
        auto t1 = high_resolution_clock::now();
        double tBuscaSplay80 = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Localidade_Temporal", "Splay", "Zipf_80_20", "Busca_100k", 100000, tBuscaSplay80, (tBuscaSplay80 * 1000.0) / 100000, splay.getRotationCount(), splay.size()});

        avl.resetMetrics();
        t0 = high_resolution_clock::now();
        for (int b : buscasZipf80) avl.search(b);
        t1 = high_resolution_clock::now();
        double tBuscaAVL80 = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Localidade_Temporal", "AVL", "Zipf_80_20", "Busca_100k", 100000, tBuscaAVL80, (tBuscaAVL80 * 1000.0) / 100000, 0, avl.size()});

        t0 = high_resolution_clock::now();
        for (int b : buscasZipf80) treap.search(b);
        t1 = high_resolution_clock::now();
        double tBuscaTreap80 = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Localidade_Temporal", "Treap", "Zipf_80_20", "Busca_100k", 100000, tBuscaTreap80, (tBuscaTreap80 * 1000.0) / 100000, 0, treap.size()});

        std::cout << "  -> Zipf 80-20 (100.000 buscas em N=50k):" << std::endl;
        std::cout << "     Splay: " << tBuscaSplay80 << " us (rotacoes autoajuste: " << splay.getRotationCount() << ")" << std::endl;
        std::cout << "     AVL:   " << tBuscaAVL80 << " us" << std::endl;
        std::cout << "     Treap: " << tBuscaTreap80 << " us" << std::endl;
    }
}

// -----------------------------------------------------------------------------
// 3. BENCHMARK: ESPACIAIS MULTIDIMENSIONAIS (KD-TREE 2D e 3D)
// -----------------------------------------------------------------------------
void benchmarkEspaciais() {
    std::cout << "\n==================================================" << std::endl;
    std::cout << " [3/3] EXECUTANDO BENCHMARK: KD-TREE (2D e 3D) " << std::endl;
    std::cout << "==================================================" << std::endl;

    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};

    // Carrega 2.000 alvos para Nearest Neighbor e 2.000 caixas de Range Search
    std::vector<Ponto<2>> alvosNN;
    std::ifstream arqNN("benchmarks/datasets/3_espaciais/consultas_pontos_alvo_nn.txt");
    double x, y;
    while (arqNN >> x >> y) alvosNN.push_back(Ponto<2>({x, y}));
    arqNN.close();

    struct CaixaRange { Ponto<2> minPt; Ponto<2> maxPt; };
    std::vector<CaixaRange> caixas;
    std::ifstream arqRange("benchmarks/datasets/3_espaciais/consultas_caixas_range.txt");
    double x1, y1, x2, y2;
    while (arqRange >> x1 >> y1 >> x2 >> y2) {
        caixas.push_back({Ponto<2>({x1, y1}), Ponto<2>({x2, y2})});
    }
    arqRange.close();

    for (size_t n : tamanhos) {
        std::string caminho2D = "benchmarks/datasets/3_espaciais/uniformes_2d_" + std::to_string(n) + ".txt";
        std::ifstream arq(caminho2D);
        if (!arq.is_open()) continue;

        KDTree<2> kd2;
        std::vector<Ponto<2>> pts;
        while (arq >> x >> y) pts.push_back(Ponto<2>({x, y}));
        arq.close();

        // Inserção / Construção
        auto t0 = high_resolution_clock::now();
        for (const auto& pt : pts) kd2.insert(pt);
        auto t1 = high_resolution_clock::now();
        double tIns = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Espacial", "KDTree_2D", "Uniformes", "Construcao", n, tIns, (tIns * 1000.0) / n, 0, kd2.size()});

        // 2.000 Consultas Nearest Neighbor (1-NN)
        kd2.resetMetrics();
        t0 = high_resolution_clock::now();
        for (const auto& alvo : alvosNN) {
            kd2.nearestNeighbor(alvo);
        }
        t1 = high_resolution_clock::now();
        double tNN = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        double mediaDistPorConsulta = (double)kd2.getDistanceComputations() / (double)alvosNN.size();
        resultados.push_back({"Espacial", "KDTree_2D", "Uniformes", "Nearest_Neighbor_2000x", n, tNN, (tNN * 1000.0) / alvosNN.size(), kd2.getDistanceComputations(), kd2.size()});

        // 2.000 Consultas Range Search
        t0 = high_resolution_clock::now();
        for (const auto& c : caixas) {
            kd2.rangeSearch(c.minPt, c.maxPt);
        }
        t1 = high_resolution_clock::now();
        double tRange = duration_cast<nanoseconds>(t1 - t0).count() / 1000.0;
        resultados.push_back({"Espacial", "KDTree_2D", "Uniformes", "Range_Search_2000x", n, tRange, (tRange * 1000.0) / caixas.size(), kd2.getPruningCount(), kd2.size()});

        std::cout << "  -> N = " << std::setw(6) << n 
                  << " | 2000x 1-NN: " << std::setw(8) << tNN << " us"
                  << " (Distancias calculadas: " << std::setw(4) << (int)mediaDistPorConsulta << "/ponto)"
                  << " | 2000x Range: " << std::setw(8) << tRange << " us" << std::endl;
    }
}

int main() {
    std::cout << "=================================================================" << std::endl;
    std::cout << " EXECUTOR GERAL DE BENCHMARKS COMPARATIVOS (AEDS II) " << std::endl;
    std::cout << "=================================================================" << std::endl;

    benchmarkStrings();
    benchmarkNumericos();
    benchmarkEspaciais();

    salvarResultadosCSV("benchmarks/dados_comparativos.csv");

    std::cout << "\n=================================================================" << std::endl;
    std::cout << ">>> TODOS OS BENCHMARKS FORAM FINALIZADOS COM SUCESSO! <<<" << std::endl;
    std::cout << "=================================================================" << std::endl;
    return 0;
}
