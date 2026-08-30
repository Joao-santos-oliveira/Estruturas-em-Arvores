#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>

#include "trie.hpp"
#include "patricia.hpp"
#include "splay_tree.hpp"
#include "treap.hpp"
#include "kd_tree.hpp"
#include "bst.hpp"
#include "avl_tree.hpp"

using namespace std::chrono;

struct ResultadoExecucao {
    std::string estrutura;
    std::string dataset;
    std::string operacao;
    size_t tamanhoN;
    double tempoTotalSegundos;
    double tempoMedioNanossegundos;
    unsigned long long metricasAuxiliares; // Rotações, comparações, splits, podas
    size_t contagemNos;
    std::string detalheExtra;
};

void gravarCSV(const ResultadoExecucao& r) {
    std::string caminhoCSV = "benchmarks/dados_comparativos.csv";
    bool existe = std::ifstream(caminhoCSV).good();

    std::ofstream arq(caminhoCSV, std::ios::app);
    if (!arq.is_open()) return;

    if (!existe) {
        arq << "Estrutura,Dataset,Operacao,TamanhoN,TempoTotal_s,TempoMedio_ns,Metrica_Operacoes,Contagem_Nos,Detalhes\n";
    }

    arq << r.estrutura << ","
        << r.dataset << ","
        << r.operacao << ","
        << r.tamanhoN << ","
        << std::fixed << std::setprecision(6) << r.tempoTotalSegundos << ","
        << std::fixed << std::setprecision(2) << r.tempoMedioNanossegundos << ","
        << r.metricasAuxiliares << ","
        << r.contagemNos << ","
        << "\"" << r.detalheExtra << "\"\n";
    arq.close();
}

void imprimirCabecalhoTabela() {
    std::cout << "\n+-----------------+---------------------+---------------------+-----------+-----------------+-----------------+---------------+--------------+" << std::endl;
    std::cout << "| Estrutura       | Dataset             | Operacao            | Tamanho N | Tempo Total (s) | Media (ns/op)   | Op. Internas  | Total de Nos |" << std::endl;
    std::cout << "+-----------------+---------------------+---------------------+-----------+-----------------+-----------------+---------------+--------------+" << std::endl;
}

void imprimirLinhaTabela(const ResultadoExecucao& r) {
    std::cout << "| " << std::left << std::setw(15) << r.estrutura
              << " | " << std::setw(19) << r.dataset
              << " | " << std::setw(19) << r.operacao
              << " | " << std::right << std::setw(9) << r.tamanhoN
              << " | " << std::setw(15) << std::fixed << std::setprecision(6) << r.tempoTotalSegundos
              << " | " << std::setw(15) << std::fixed << std::setprecision(2) << r.tempoMedioNanossegundos
              << " | " << std::setw(13) << r.metricasAuxiliares
              << " | " << std::setw(12) << r.contagemNos << " |" << std::endl;
}

void imprimirRodapeTabela() {
    std::cout << "+-----------------+---------------------+---------------------+-----------+-----------------+-----------------+---------------+--------------+" << std::endl;
}

void imprimirResumoTempoTotal(const std::vector<ResultadoExecucao>& resultados) {
    double somaSegundos = 0.0;
    for (const auto& r : resultados) {
        somaSegundos += r.tempoTotalSegundos;
    }
    std::cout << "\n>>> TEMPO TOTAL ACUMULADO DO CICLO: " 
              << std::fixed << std::setprecision(6) << somaSegundos << " s"
              << " (" << std::fixed << std::setprecision(2) << (somaSegundos * 1000.0) << " ms)"
              << " <<<" << std::endl;
}

std::vector<std::string> carregarLinhas(const std::string& caminho) {
    std::vector<std::string> linhas;
    std::ifstream arq(caminho);
    if (!arq.is_open()) return linhas;
    std::string l;
    while (std::getline(arq, l)) {
        if (!l.empty()) linhas.push_back(l);
    }
    return linhas;
}

std::vector<int> carregarInteiros(const std::string& caminho) {
    std::vector<int> vals;
    std::ifstream arq(caminho);
    if (!arq.is_open()) return vals;
    int v;
    while (arq >> v) vals.push_back(v);
    return vals;
}

// =============================================================================
// CICLOS COMPLETOS DE OPERAÇÕES POR ESTRUTURA
// =============================================================================

std::vector<ResultadoExecucao> executarCicloTrie(const std::vector<std::string>& dados, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    Trie trie;
    std::vector<std::string> sub(dados.begin(), dados.begin() + std::min(n, dados.size()));

    // 1. Inserção
    auto t0 = high_resolution_clock::now();
    for (const auto& p : sub) trie.insert(p);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Trie", nomeDataset, "Insercao", sub.size(), segIns, (segIns * 1e9) / sub.size(), 0, trie.getNodeCount(), "Construcao"});

    // 2. Busca (Sucesso)
    t0 = high_resolution_clock::now();
    for (const auto& p : sub) trie.search(p);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Trie", nomeDataset, "Busca_Sucesso", sub.size(), segBusca, (segBusca * 1e9) / sub.size(), 0, trie.getNodeCount(), "100% Sucesso"});

    // 3. Busca (Falha)
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < sub.size(); ++i) trie.search("xyz" + sub[i]);
    t1 = high_resolution_clock::now();
    double segFalha = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Trie", nomeDataset, "Busca_Falha", sub.size(), segFalha, (segFalha * 1e9) / sub.size(), 0, trie.getNodeCount(), "Rejeicao rapida"});

    // 4. Autocomplete (1.000 prefixos)
    size_t nAuto = std::min((size_t)1000, sub.size());
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < nAuto; ++i) {
        if (sub[i].length() >= 3) trie.autocomplete(sub[i].substr(0, 3));
    }
    t1 = high_resolution_clock::now();
    double segAuto = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Trie", nomeDataset, "Autocomplete_1kx", nAuto, segAuto, (segAuto * 1e9) / nAuto, 0, trie.getNodeCount(), "Prefix Matching"});

    // 5. Remoção (50% dos nós)
    size_t remCount = sub.size() / 2;
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < remCount; ++i) trie.remove(sub[i]);
    t1 = high_resolution_clock::now();
    double segRem = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Trie", nomeDataset, "Remocao_50pct", remCount, segRem, (segRem * 1e9) / remCount, 0, trie.getNodeCount(), "Poda de orfaos"});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

std::vector<ResultadoExecucao> executarCicloPatricia(const std::vector<std::string>& dados, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    PatriciaTree pat;
    std::vector<std::string> sub(dados.begin(), dados.begin() + std::min(n, dados.size()));

    // 1. Inserção
    auto t0 = high_resolution_clock::now();
    for (const auto& p : sub) pat.insert(p);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Patricia", nomeDataset, "Insercao", sub.size(), segIns, (segIns * 1e9) / sub.size(), pat.getSplitCount(), pat.getNodeCount(), "Splits: " + std::to_string(pat.getSplitCount())});

    // 2. Busca (Sucesso)
    t0 = high_resolution_clock::now();
    for (const auto& p : sub) pat.search(p);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Patricia", nomeDataset, "Busca_Sucesso", sub.size(), segBusca, (segBusca * 1e9) / sub.size(), 0, pat.getNodeCount(), "100% Sucesso"});

    // 3. Busca (Falha)
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < sub.size(); ++i) pat.search("xyz" + sub[i]);
    t1 = high_resolution_clock::now();
    double segFalha = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Patricia", nomeDataset, "Busca_Falha", sub.size(), segFalha, (segFalha * 1e9) / sub.size(), 0, pat.getNodeCount(), "Rejeicao rapida"});

    // 4. Remoção (50% com Merge)
    size_t remCount = sub.size() / 2;
    pat.resetMetrics();
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < remCount; ++i) pat.remove(sub[i]);
    t1 = high_resolution_clock::now();
    double segRem = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Patricia", nomeDataset, "Remocao_50pct", remCount, segRem, (segRem * 1e9) / remCount, pat.getMergeCount(), pat.getNodeCount(), "Merges: " + std::to_string(pat.getMergeCount())});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

std::vector<ResultadoExecucao> executarCicloSplay(const std::vector<int>& dados, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    SplayTree<int> splay;
    std::vector<int> sub(dados.begin(), dados.begin() + std::min(n, dados.size()));

    // 1. Inserção
    auto t0 = high_resolution_clock::now();
    for (int v : sub) splay.insert(v);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"SplayTree", nomeDataset, "Insercao", sub.size(), segIns, (segIns * 1e9) / sub.size(), splay.getRotationCount(), splay.size(), "Rotacoes: " + std::to_string(splay.getRotationCount())});

    // 2. Busca com Autoajuste
    splay.resetMetrics();
    t0 = high_resolution_clock::now();
    for (int v : sub) splay.search(v);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"SplayTree", nomeDataset, "Busca_Autoajuste", sub.size(), segBusca, (segBusca * 1e9) / sub.size(), splay.getRotationCount(), splay.size(), "Rotacoes busca: " + std::to_string(splay.getRotationCount())});

    // 3. Busca (Falha)
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < sub.size(); ++i) splay.search(-1 * (int)(i + 1));
    t1 = high_resolution_clock::now();
    double segFalha = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"SplayTree", nomeDataset, "Busca_Falha", sub.size(), segFalha, (segFalha * 1e9) / sub.size(), 0, splay.size(), "Chaves ausentes"});

    // 4. Remoção (50%)
    size_t remCount = sub.size() / 2;
    splay.resetMetrics();
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < remCount; ++i) splay.remove(sub[i]);
    t1 = high_resolution_clock::now();
    double segRem = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"SplayTree", nomeDataset, "Remocao_50pct", remCount, segRem, (segRem * 1e9) / remCount, splay.getRotationCount(), splay.size(), "Split & Join"});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

std::vector<ResultadoExecucao> executarCicloTreap(const std::vector<int>& dados, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    Treap<int> treap(42);
    std::vector<int> sub(dados.begin(), dados.begin() + std::min(n, dados.size()));

    // 1. Inserção
    auto t0 = high_resolution_clock::now();
    for (int v : sub) treap.insert(v);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Treap", nomeDataset, "Insercao", sub.size(), segIns, (segIns * 1e9) / sub.size(), treap.getRotationCount(), treap.size(), "Rotacoes Heap: " + std::to_string(treap.getRotationCount())});

    // 2. Busca
    t0 = high_resolution_clock::now();
    for (int v : sub) treap.search(v);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Treap", nomeDataset, "Busca_Sucesso", sub.size(), segBusca, (segBusca * 1e9) / sub.size(), 0, treap.size(), "Altura: " + std::to_string(treap.height())});

    // 3. Busca (Falha)
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < sub.size(); ++i) treap.search(-1 * (int)(i + 1));
    t1 = high_resolution_clock::now();
    double segFalha = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Treap", nomeDataset, "Busca_Falha", sub.size(), segFalha, (segFalha * 1e9) / sub.size(), 0, treap.size(), "Chaves ausentes"});

    // 4. Remoção (50%)
    size_t remCount = sub.size() / 2;
    treap.resetMetrics();
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < remCount; ++i) treap.remove(sub[i]);
    t1 = high_resolution_clock::now();
    double segRem = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"Treap", nomeDataset, "Remocao_50pct", remCount, segRem, (segRem * 1e9) / remCount, treap.getRotationCount(), treap.size(), "Rotacoes descida"});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

std::vector<ResultadoExecucao> executarCicloAVL(const std::vector<int>& dados, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    AVLTree<int> avl;
    std::vector<int> sub(dados.begin(), dados.begin() + std::min(n, dados.size()));

    // 1. Inserção
    auto t0 = high_resolution_clock::now();
    for (int v : sub) avl.insert(v);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"AVLTree", nomeDataset, "Insercao", sub.size(), segIns, (segIns * 1e9) / sub.size(), avl.getRotationCount(), avl.size(), "Rotacoes: " + std::to_string(avl.getRotationCount())});

    // 2. Busca
    t0 = high_resolution_clock::now();
    for (int v : sub) avl.search(v);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"AVLTree", nomeDataset, "Busca_Sucesso", sub.size(), segBusca, (segBusca * 1e9) / sub.size(), 0, avl.size(), "Altura: " + std::to_string(avl.height())});

    // 3. Busca (Falha)
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < sub.size(); ++i) avl.search(-1 * (int)(i + 1));
    t1 = high_resolution_clock::now();
    double segFalha = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"AVLTree", nomeDataset, "Busca_Falha", sub.size(), segFalha, (segFalha * 1e9) / sub.size(), 0, avl.size(), "Chaves ausentes"});

    // 4. Remoção (50%)
    size_t remCount = sub.size() / 2;
    avl.resetMetrics();
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < remCount; ++i) avl.remove(sub[i]);
    t1 = high_resolution_clock::now();
    double segRem = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"AVLTree", nomeDataset, "Remocao_50pct", remCount, segRem, (segRem * 1e9) / remCount, avl.getRotationCount(), avl.size(), "Rotacoes remocao"});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

std::vector<ResultadoExecucao> executarCicloBST(const std::vector<int>& dados, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    BST<int> bst;
    std::vector<int> sub(dados.begin(), dados.begin() + std::min(n, dados.size()));

    // 1. Inserção
    auto t0 = high_resolution_clock::now();
    for (int v : sub) bst.insert(v);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"BST", nomeDataset, "Insercao", sub.size(), segIns, (segIns * 1e9) / sub.size(), bst.getComparisonCount(), bst.size(), "Comparacoes: " + std::to_string(bst.getComparisonCount())});

    // 2. Busca
    bst.resetMetrics();
    t0 = high_resolution_clock::now();
    for (int v : sub) bst.search(v);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"BST", nomeDataset, "Busca_Sucesso", sub.size(), segBusca, (segBusca * 1e9) / sub.size(), bst.getComparisonCount(), bst.size(), "Altura: " + std::to_string(bst.height())});

    // 3. Remoção (50%)
    size_t remCount = sub.size() / 2;
    t0 = high_resolution_clock::now();
    for (size_t i = 0; i < remCount; ++i) bst.remove(sub[i]);
    t1 = high_resolution_clock::now();
    double segRem = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"BST", nomeDataset, "Remocao_50pct", remCount, segRem, (segRem * 1e9) / remCount, 0, bst.size(), "Remocao padrao"});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

std::vector<ResultadoExecucao> executarCicloKDTree2D(const std::string& caminhoArquivo, const std::string& nomeDataset, size_t n) {
    std::vector<ResultadoExecucao> res;
    KDTree<2> kd;
    std::ifstream arq(caminhoArquivo);
    std::vector<Ponto<2>> pts;
    double x, y;
    while (arq >> x >> y && pts.size() < n) {
        pts.push_back(Ponto<2>({x, y}));
    }
    arq.close();

    // 1. Construção
    auto t0 = high_resolution_clock::now();
    for (const auto& pt : pts) kd.insert(pt);
    auto t1 = high_resolution_clock::now();
    double segIns = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"KDTree_2D", nomeDataset, "Construcao", pts.size(), segIns, (segIns * 1e9) / pts.size(), 0, kd.size(), "Cortes X e Y"});

    // 2. Busca Exata
    t0 = high_resolution_clock::now();
    for (const auto& pt : pts) kd.search(pt);
    t1 = high_resolution_clock::now();
    double segBusca = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"KDTree_2D", nomeDataset, "Busca_Exata", pts.size(), segBusca, (segBusca * 1e9) / pts.size(), 0, kd.size(), "100% Sucesso"});

    // 3. 1000x Nearest Neighbor (1-NN)
    kd.resetMetrics();
    t0 = high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        kd.nearestNeighbor(Ponto<2>({(double)(i % 500), (double)((i * 3) % 500)}));
    }
    t1 = high_resolution_clock::now();
    double segNN = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"KDTree_2D", nomeDataset, "Nearest_Neigh_1kx", 1000, segNN, (segNN * 1e9) / 1000.0, kd.getDistanceComputations(), kd.size(), "Distancias: " + std::to_string(kd.getDistanceComputations())});

    // 4. 1000x Range Search
    kd.resetMetrics();
    t0 = high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        kd.rangeSearch(Ponto<2>({-100.0, -100.0}), Ponto<2>({100.0, 100.0}));
    }
    t1 = high_resolution_clock::now();
    double segRange = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    res.push_back({"KDTree_2D", nomeDataset, "Range_Search_1kx", 1000, segRange, (segRange * 1e9) / 1000.0, kd.getPruningCount(), kd.size(), "Podas: " + std::to_string(kd.getPruningCount())});

    for (const auto& r : res) gravarCSV(r);
    return res;
}

// =============================================================================
// MENUS E INTERFACE INTERATIVA COM SELEÇÃO DE DATASET
// =============================================================================

void menuEscolherTamanho(size_t& n) {
    std::cout << "\nEscolha o tamanho de entrada (N):" << std::endl;
    std::cout << "1. N = 100" << std::endl;
    std::cout << "2. N = 1.000" << std::endl;
    std::cout << "3. N = 10.000" << std::endl;
    std::cout << "4. N = 50.000" << std::endl;
    std::cout << "5. N = 100.000" << std::endl;
    std::cout << "Opcao: ";
    int op;
    if (std::cin >> op) {
        if (op == 1) n = 100;
        else if (op == 2) n = 1000;
        else if (op == 3) n = 10000;
        else if (op == 4) n = 50000;
        else if (op == 5) n = 100000;
        else n = 1000;
    }
}

void menuComparativo() {
    std::cout << "\n==================================================" << std::endl;
    std::cout << " PAINEL DE BENCHMARKING INTERATIVO E COMPARATIVO " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "1. Executar Estrutura Individual (com selecao de Dataset)" << std::endl;
    std::cout << "2. Comparativo: Trie vs. Patricia Tree (Strings e Memoria)" << std::endl;
    std::cout << "3. Comparativo: Treap vs. AVL vs. BST vs. Splay (Chaves Ordenadas / Aleatorias)" << std::endl;
    std::cout << "4. Comparativo: Splay vs. AVL vs. Treap (Localidade Zipf 80-20)" << std::endl;
    std::cout << "5. Teste de Desempenho e Poda Espacial da KD-Tree" << std::endl;
    std::cout << "6. Executar Bateria Completa de Todos os Experimentos (Modo Geral)" << std::endl;
    std::cout << "0. Sair" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Escolha uma opcao: ";
}

void executarIndividual() {
    std::cout << "\nEscolha a Estrutura:" << std::endl;
    std::cout << "1. Trie (Prefix Tree)" << std::endl;
    std::cout << "2. Arvore Patricia (Radix Compacta)" << std::endl;
    std::cout << "3. Arvore Splay (BST Autoajustavel)" << std::endl;
    std::cout << "4. Arvore Treap (BST + Heap)" << std::endl;
    std::cout << "5. Arvore AVL (Balanceada Estrita - Baseline)" << std::endl;
    std::cout << "6. BST Padrao (Nao Balanceada - Baseline)" << std::endl;
    std::cout << "7. KD-Tree (Espacial 2D)" << std::endl;
    std::cout << "Opcao: ";
    int est; std::cin >> est;
    size_t n = 10000; menuEscolherTamanho(n);

    std::vector<ResultadoExecucao> linhas;

    // Se for Trie ou Patricia (Strings)
    if (est == 1 || est == 2) {
        std::cout << "\nEscolha o Dataset de Strings:" << std::endl;
        std::cout << "1. Prefixos Densos (Dicionario Real / Alta Sobreposicao)" << std::endl;
        std::cout << "2. Prefixos Dispersos (Strings Aleatorias / Baixa Sobreposicao)" << std::endl;
        std::cout << "Opcao: ";
        int opD = 1; std::cin >> opD;
        std::string arqNome = (opD == 2) ? "prefixos_dispersos_" : "prefixos_densos_";
        std::string nomeD = (opD == 2) ? "Prefixos_Dispersos" : "Prefixos_Densos";
        std::string caminho = "benchmarks/datasets/1_strings/" + arqNome + std::to_string(n) + ".txt";

        auto dados = carregarLinhas(caminho);
        if (dados.empty()) {
            std::cout << "[ERRO] Dataset nao encontrado: " << caminho << std::endl;
            return;
        }

        if (est == 1) linhas = executarCicloTrie(dados, nomeD, n);
        else linhas = executarCicloPatricia(dados, nomeD, n);
    }
    // Se for Splay, Treap, AVL ou BST (Numéricos)
    else if (est >= 3 && est <= 6) {
        std::cout << "\nEscolha o Dataset Numerico:" << std::endl;
        std::cout << "1. Aleatorio Uniforme (Caso Medio)" << std::endl;
        std::cout << "2. Ordenado Crescente (1 .. N - Pior caso BST)" << std::endl;
        std::cout << "3. Ordenado Decrescente (N .. 1)" << std::endl;
        std::cout << "4. Quase Ordenado (95% ordenado)" << std::endl;
        std::cout << "Opcao: ";
        int opD = 1; std::cin >> opD;

        std::string prefixoArq = "aleatorio_uniforme_";
        std::string nomeD = "Aleatorio";
        if (opD == 2) { prefixoArq = "ordenado_crescente_"; nomeD = "Ordenado_Cresc"; }
        else if (opD == 3) { prefixoArq = "ordenado_decrescente_"; nomeD = "Ordenado_Decresc"; }
        else if (opD == 4) { prefixoArq = "quase_ordenado_"; nomeD = "Quase_Ordenado"; }

        std::string caminho = "benchmarks/datasets/2_numericos/" + prefixoArq + std::to_string(n) + ".txt";
        auto dados = carregarInteiros(caminho);
        if (dados.empty()) {
            std::cout << "[ERRO] Dataset nao encontrado: " << caminho << std::endl;
            return;
        }

        if (est == 3) linhas = executarCicloSplay(dados, nomeD, n);
        else if (est == 4) linhas = executarCicloTreap(dados, nomeD, n);
        else if (est == 5) linhas = executarCicloAVL(dados, nomeD, n);
        else if (est == 6) {
            if (opD >= 2 && opD <= 3 && n > 10000) {
                std::cout << "\n[AVISO] A BST padrao com N > 10.000 em dados ordenados atinge profundidade O(N) e pode causar Stack Overflow. Limitando a N=10.000 para seguranca." << std::endl;
                n = 10000;
                caminho = "benchmarks/datasets/2_numericos/" + prefixoArq + "10000.txt";
                dados = carregarInteiros(caminho);
            }
            linhas = executarCicloBST(dados, nomeD, n);
        }
    }
    // Se for KD-Tree (Espaciais)
    else if (est == 7) {
        std::cout << "\nEscolha o Dataset Espacial:" << std::endl;
        std::cout << "1. Distribuicao Uniforme no Plano 2D" << std::endl;
        std::cout << "2. Clusters Gaussianos (Cidades/Sistemas GIS)" << std::endl;
        std::cout << "Opcao: ";
        int opD = 1; std::cin >> opD;

        std::string arq = (opD == 2) ? "clusters_2d_" : "uniformes_2d_";
        std::string nomeD = (opD == 2) ? "Clusters_2D" : "Uniformes_2D";
        std::string caminho = "benchmarks/datasets/3_espaciais/" + arq + std::to_string(n) + ".txt";

        linhas = executarCicloKDTree2D(caminho, nomeD, n);
    }

    imprimirCabecalhoTabela();
    for (const auto& l : linhas) imprimirLinhaTabela(l);
    imprimirRodapeTabela();
    imprimirResumoTempoTotal(linhas);
}

void rodarComparativoTrieVsPatricia() {
    size_t n = 10000;
    menuEscolherTamanho(n);

    std::cout << "\nEscolha o tipo de dataset de strings:" << std::endl;
    std::cout << "1. Prefixos Densos (Caso Real de Dicionario / Alta Sobreposicao)" << std::endl;
    std::cout << "2. Prefixos Dispersos (Strings Aleatorias / Baixa Sobreposicao)" << std::endl;
    std::cout << "Opcao: ";
    int opDataset = 1;
    std::cin >> opDataset;

    std::string arqNome = (opDataset == 2) ? "prefixos_dispersos_" : "prefixos_densos_";
    std::string nomeD = (opDataset == 2) ? "Prefixos_Dispersos" : "Prefixos_Densos";
    std::string caminho = "benchmarks/datasets/1_strings/" + arqNome + std::to_string(n) + ".txt";

    auto dados = carregarLinhas(caminho);
    if (dados.empty()) {
        std::cout << "[ERRO] Arquivo de dataset nao encontrado: " << caminho << std::endl;
        return;
    }

    std::cout << "\n[EXECUTANDO] Comparando Trie vs. Patricia com N = " << n << " (" << nomeD << ")..." << std::endl;
    auto linhasTrie = executarCicloTrie(dados, nomeD, n);
    auto linhasPat = executarCicloPatricia(dados, nomeD, n);

    imprimirCabecalhoTabela();
    for (const auto& l : linhasTrie) imprimirLinhaTabela(l);
    for (const auto& l : linhasPat) imprimirLinhaTabela(l);
    imprimirRodapeTabela();

    size_t nosTrie = linhasTrie[0].contagemNos;
    size_t nosPat = linhasPat[0].contagemNos;
    double economiaNos = (1.0 - (double)nosPat / (double)nosTrie) * 100.0;

    std::cout << "\n>>> ANALISE COMPARATIVA CONCLUIDA <<<" << std::endl;
    std::cout << "• Economia de Memoria: A Patricia usou " << std::fixed << std::setprecision(1) 
              << economiaNos << "% MENOS nos que a Trie! (" << nosPat << " nos vs " << nosTrie << " nos)" << std::endl;
}

void rodarComparativoOrdenados() {
    size_t n = 10000;
    menuEscolherTamanho(n);

    std::cout << "\nEscolha o Dataset para Comparativo:" << std::endl;
    std::cout << "1. Ordenado Crescente (1 .. N - Pior Caso da BST)" << std::endl;
    std::cout << "2. Ordenado Decrescente (N .. 1)" << std::endl;
    std::cout << "3. Aleatorio Uniforme (Caso Medio)" << std::endl;
    std::cout << "Opcao: ";
    int opD = 1; std::cin >> opD;

    std::string prefixoArq = "ordenado_crescente_";
    std::string nomeD = "Ordenado_Cresc";
    if (opD == 2) { prefixoArq = "ordenado_decrescente_"; nomeD = "Ordenado_Decresc"; }
    else if (opD == 3) { prefixoArq = "aleatorio_uniforme_"; nomeD = "Aleatorio"; }

    std::string caminho = "benchmarks/datasets/2_numericos/" + prefixoArq + std::to_string(n) + ".txt";
    auto dados = carregarInteiros(caminho);
    if (dados.empty()) {
        std::cout << "[ERRO] Dataset nao encontrado: " << caminho << std::endl;
        return;
    }

    std::cout << "\n[EXECUTANDO] Comparando Treap vs. AVL vs. Splay vs. BST com N = " << n << " (" << nomeD << ")..." << std::endl;
    auto lTreap = executarCicloTreap(dados, nomeD, n);
    auto lAVL = executarCicloAVL(dados, nomeD, n);
    auto lSplay = executarCicloSplay(dados, nomeD, n);

    imprimirCabecalhoTabela();
    for (const auto& l : lTreap) imprimirLinhaTabela(l);
    for (const auto& l : lAVL) imprimirLinhaTabela(l);
    for (const auto& l : lSplay) imprimirLinhaTabela(l);

    if (n <= 10000) {
        auto lBST = executarCicloBST(dados, nomeD, n);
        for (const auto& l : lBST) imprimirLinhaTabela(l);
        imprimirRodapeTabela();
        std::cout << "\n>>> ANALISE: Em dados ordenados, a BST degenera para lista linear (O(N^2)), enquanto a Treap e AVL mantem O(log N)!" << std::endl;
    } else {
        imprimirRodapeTabela();
        std::cout << "\n(Aviso: BST padrao omitida para N > 10.000 para evitar travamento por profundidade O(N))." << std::endl;
    }
}

void rodarComparativoZipf() {
    std::cout << "\n[EXECUTANDO] Carregando N = 50.000 chaves e 100.000 buscas com distribuicao Zipfiana 80-20..." << std::endl;
    auto chaves = carregarInteiros("benchmarks/datasets/2_numericos/aleatorio_uniforme_50000.txt");
    auto buscas = carregarInteiros("benchmarks/datasets/2_numericos/consultas_zipf_80_20_100k.txt");

    if (chaves.empty() || buscas.empty()) {
        std::cout << "[ERRO] Datasets necessarios nao encontrados." << std::endl;
        return;
    }

    SplayTree<int> splay;
    AVLTree<int> avl;
    Treap<int> treap(42);

    for (int v : chaves) {
        splay.insert(v);
        avl.insert(v);
        treap.insert(v);
    }

    splay.resetMetrics();
    auto t0 = high_resolution_clock::now();
    for (int b : buscas) splay.search(b);
    auto t1 = high_resolution_clock::now();
    double segSplay = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    ResultadoExecucao rSplay = {"SplayTree", "Zipf_80_20", "Busca_100k", buscas.size(), segSplay, (segSplay * 1e9) / buscas.size(), splay.getRotationCount(), splay.size(), "Autoajuste"};

    avl.resetMetrics();
    t0 = high_resolution_clock::now();
    for (int b : buscas) avl.search(b);
    t1 = high_resolution_clock::now();
    double segAVL = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    ResultadoExecucao rAVL = {"AVLTree", "Zipf_80_20", "Busca_100k", buscas.size(), segAVL, (segAVL * 1e9) / buscas.size(), 0, avl.size(), "Balanceamento Fixo"};

    t0 = high_resolution_clock::now();
    for (int b : buscas) treap.search(b);
    t1 = high_resolution_clock::now();
    double segTreap = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
    ResultadoExecucao rTreap = {"Treap", "Zipf_80_20", "Busca_100k", buscas.size(), segTreap, (segTreap * 1e9) / buscas.size(), 0, treap.size(), "Probabilistico"};

    gravarCSV(rSplay); gravarCSV(rAVL); gravarCSV(rTreap);

    imprimirCabecalhoTabela();
    imprimirLinhaTabela(rSplay);
    imprimirLinhaTabela(rAVL);
    imprimirLinhaTabela(rTreap);
    imprimirRodapeTabela();
}

void rodarTesteKDTree() {
    size_t n = 10000;
    menuEscolherTamanho(n);

    std::cout << "\nEscolha a distribuicao de pontos espaciais:" << std::endl;
    std::cout << "1. Pontos com Distribuicao Uniforme no Plano 2D" << std::endl;
    std::cout << "2. Pontos Agrupados em Clusters Gaussianos (Cidades/GIS)" << std::endl;
    std::cout << "Opcao: ";
    int opD = 1;
    std::cin >> opD;

    std::string arq = (opD == 2) ? "clusters_2d_" : "uniformes_2d_";
    std::string nomeD = (opD == 2) ? "Clusters_2D" : "Uniformes_2D";
    std::string caminho = "benchmarks/datasets/3_espaciais/" + arq + std::to_string(n) + ".txt";

    std::cout << "\n[EXECUTANDO] Indexando " << n << " pontos na KD-Tree e executando consultas..." << std::endl;
    auto linhas = executarCicloKDTree2D(caminho, nomeD, n);

    imprimirCabecalhoTabela();
    for (const auto& l : linhas) imprimirLinhaTabela(l);
    imprimirRodapeTabela();
    imprimirResumoTempoTotal(linhas);
}

void rodarBateriaCompleta() {
    std::cout << "\n=================================================================" << std::endl;
    std::cout << " EXECUTANDO BATERIA COMPLETA DE BENCHMARKS (N = 100 A 100.000) " << std::endl;
    std::cout << "=================================================================" << std::endl;

    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};
    imprimirCabecalhoTabela();

    for (size_t n : tamanhos) {
        auto strDados = carregarLinhas("benchmarks/datasets/1_strings/prefixos_densos_" + std::to_string(n) + ".txt");
        if (!strDados.empty()) {
            for (const auto& l : executarCicloTrie(strDados, "Prefixos_Densos", n)) imprimirLinhaTabela(l);
            for (const auto& l : executarCicloPatricia(strDados, "Prefixos_Densos", n)) imprimirLinhaTabela(l);
        }

        auto numDados = carregarInteiros("benchmarks/datasets/2_numericos/ordenado_crescente_" + std::to_string(n) + ".txt");
        if (!numDados.empty()) {
            for (const auto& l : executarCicloTreap(numDados, "Ordenado_Cresc", n)) imprimirLinhaTabela(l);
            for (const auto& l : executarCicloAVL(numDados, "Ordenado_Cresc", n)) imprimirLinhaTabela(l);
            for (const auto& l : executarCicloSplay(numDados, "Ordenado_Cresc", n)) imprimirLinhaTabela(l);
            if (n <= 10000) {
                for (const auto& l : executarCicloBST(numDados, "Ordenado_Cresc", n)) imprimirLinhaTabela(l);
            }
        }

        std::string caminhoKD = "benchmarks/datasets/3_espaciais/uniformes_2d_" + std::to_string(n) + ".txt";
        if (std::ifstream(caminhoKD).good()) {
            for (const auto& l : executarCicloKDTree2D(caminhoKD, "Uniformes_2D", n)) imprimirLinhaTabela(l);
        }
    }

    imprimirRodapeTabela();
    std::cout << "\n>>> BATERIA COMPLETA CONCLUIDA! Dados salvos em benchmarks/dados_comparativos.csv <<<" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--all") {
        rodarBateriaCompleta();
        return 0;
    }

    int opc = -1;
    while (opc != 0) {
        menuComparativo();
        if (!(std::cin >> opc)) break;

        switch (opc) {
            case 1: executarIndividual(); break;
            case 2: rodarComparativoTrieVsPatricia(); break;
            case 3: rodarComparativoOrdenados(); break;
            case 4: rodarComparativoZipf(); break;
            case 5: rodarTesteKDTree(); break;
            case 6: rodarBateriaCompleta(); break;
            case 0: std::cout << "Encerrando painel de benchmarking." << std::endl; break;
            default: std::cout << "Opcao invalida." << std::endl; break;
        }
    }

    return 0;
}
