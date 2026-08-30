#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "trie.hpp"
#include "patricia.hpp"
#include "splay_tree.hpp"
#include "treap.hpp"
#include "kd_tree.hpp"
#include "bst.hpp"
#include "avl_tree.hpp"

using namespace std::chrono;

const int NUM_REPETICOES = 10;

struct EstatisticaExperimento {
    std::string categoria;
    std::string estrutura;
    std::string cenario;
    std::string operacao;
    size_t tamanhoN;
    double mediaTempoSegundos;
    double desvioPadraoSegundos;
    double minTempoSegundos;
    double maxTempoSegundos;
    double mediaTempoMedioPorOpNs;
    unsigned long long mediaOpInternas; // Rotações, splits, comparações, podas
    size_t contagemNos;
    size_t memoriaEstimadaBytes;
    std::string detalheExtra;
};

std::vector<EstatisticaExperimento> estatisticas;

// Estimativa de consumo de memória em bytes por nó
size_t estimarMemoriaBytes(const std::string& estrutura, size_t totalNos, size_t totalElementos) {
    if (estrutura == "Trie") {
        // NoTrie: bool fimPalavra (1) + padding + id (4) + map<char, NoTrie*> (~48 bytes base + 32 por filho)
        return totalNos * (sizeof(bool) + sizeof(int) + 48) + totalNos * 24;
    } else if (estrutura == "Patricia") {
        // NoPatricia: string prefixo (~32) + bool fimPalavra (1) + id (4) + map (~48)
        return totalNos * (sizeof(std::string) + sizeof(bool) + sizeof(int) + 48) + totalNos * 24;
    } else if (estrutura == "SplayTree") {
        // NoSplay: T chave (4) + 2 ponteiros (16) + id (4) = ~24 bytes
        return totalElementos * 24;
    } else if (estrutura == "Treap") {
        // NoTreap: T chave (4) + prioridade (4) + 2 ponteiros (16) + id (4) = ~28 bytes
        return totalElementos * 32;
    } else if (estrutura == "AVLTree") {
        // NoAVL: T chave (4) + altura (4) + 2 ponteiros (16) + id (4) = ~28 bytes
        return totalElementos * 32;
    } else if (estrutura == "BST") {
        // NoBST: T chave (4) + 2 ponteiros (16) + id (4) = ~24 bytes
        return totalElementos * 24;
    } else if (estrutura == "KDTree_2D") {
        // NoKD: Ponto<2> (16) + eixo (4) + 2 ponteiros (16) + id (4) = ~40 bytes
        return totalElementos * 48;
    }
    return totalElementos * 32;
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

// Remove corridas inválidas (t < 0 ou outlier acima de 10x a mediana)
std::vector<double> filtrarOutliers(std::vector<double> tempos) {
    if (tempos.size() <= 2) {
        // Eliminar apenas negativos
        tempos.erase(std::remove_if(tempos.begin(), tempos.end(), [](double t){ return t < 0.0; }), tempos.end());
        return tempos;
    }
    // Calcular mediana
    std::vector<double> sorted = tempos;
    std::sort(sorted.begin(), sorted.end());
    double mediana = sorted[sorted.size()/2];
    // Manter apenas corridas dentro de [0, max(1ms, mediana*10)]
    double limiar = std::max(0.001, mediana * 10.0);
    tempos.erase(std::remove_if(tempos.begin(), tempos.end(),
        [limiar](double t){ return t < 0.0 || t > limiar; }), tempos.end());
    return tempos.empty() ? sorted : tempos;  // fallback: retornar todas se todas forem filtradas
}

void calcularEstatisticas(const std::vector<double>& tempos_raw, double& media, double& desvio, double& minT, double& maxT) {
    if (tempos_raw.empty()) { media = desvio = minT = maxT = 0; return; }
    auto tempos = filtrarOutliers(tempos_raw);
    if (tempos.empty()) tempos = tempos_raw;
    double soma = std::accumulate(tempos.begin(), tempos.end(), 0.0);
    media = soma / tempos.size();
    minT = *std::min_element(tempos.begin(), tempos.end());
    maxT = *std::max_element(tempos.begin(), tempos.end());

    double somaQuad = 0.0;
    for (double t : tempos) {
        double diff = t - media;
        somaQuad += diff * diff;
    }
    desvio = std::sqrt(somaQuad / tempos.size());
}

void salvarCSV(const std::string& caminho) {
    std::ofstream arq(caminho);
    if (!arq.is_open()) return;

    arq << "Categoria,Estrutura,Cenario,Operacao,TamanhoN,MediaTempo_s,DesvioPadrao_s,MinTempo_s,MaxTempo_s,MediaTempoPorOp_ns,MediaOpInternas,ContagemNos,MemoriaBytes,MemoriaKB,Detalhes\n";
    for (const auto& e : estatisticas) {
        arq << e.categoria << ","
            << e.estrutura << ","
            << e.cenario << ","
            << e.operacao << ","
            << e.tamanhoN << ","
            << std::fixed << std::setprecision(8) << e.mediaTempoSegundos << ","
            << std::fixed << std::setprecision(8) << e.desvioPadraoSegundos << ","
            << std::fixed << std::setprecision(8) << e.minTempoSegundos << ","
            << std::fixed << std::setprecision(8) << e.maxTempoSegundos << ","
            << std::fixed << std::setprecision(2) << e.mediaTempoMedioPorOpNs << ","
            << e.mediaOpInternas << ","
            << e.contagemNos << ","
            << e.memoriaEstimadaBytes << ","
            << std::fixed << std::setprecision(2) << (e.memoriaEstimadaBytes / 1024.0) << ","
            << "\"" << e.detalheExtra << "\"\n";
    }
    arq.close();
}

void salvarJSON(const std::string& caminho) {
    std::ofstream arq(caminho);
    if (!arq.is_open()) return;

    arq << "{\n  \"experimentos\": [\n";
    for (size_t i = 0; i < estatisticas.size(); ++i) {
        const auto& e = estatisticas[i];
        arq << "    {\n"
            << "      \"categoria\": \"" << e.categoria << "\",\n"
            << "      \"estrutura\": \"" << e.estrutura << "\",\n"
            << "      \"cenario\": \"" << e.cenario << "\",\n"
            << "      \"operacao\": \"" << e.operacao << "\",\n"
            << "      \"tamanhoN\": " << e.tamanhoN << ",\n"
            << "      \"mediaTempo_s\": " << std::fixed << std::setprecision(8) << e.mediaTempoSegundos << ",\n"
            << "      \"desvioPadrao_s\": " << std::fixed << std::setprecision(8) << e.desvioPadraoSegundos << ",\n"
            << "      \"minTempo_s\": " << std::fixed << std::setprecision(8) << e.minTempoSegundos << ",\n"
            << "      \"maxTempo_s\": " << std::fixed << std::setprecision(8) << e.maxTempoSegundos << ",\n"
            << "      \"mediaTempoPorOp_ns\": " << std::fixed << std::setprecision(2) << e.mediaTempoMedioPorOpNs << ",\n"
            << "      \"mediaOpInternas\": " << e.mediaOpInternas << ",\n"
            << "      \"contagemNos\": " << e.contagemNos << ",\n"
            << "      \"memoriaBytes\": " << e.memoriaEstimadaBytes << ",\n"
            << "      \"memoriaKB\": " << std::fixed << std::setprecision(2) << (e.memoriaEstimadaBytes / 1024.0) << ",\n"
            << "      \"detalhes\": \"" << e.detalheExtra << "\"\n"
            << "    }" << (i + 1 < estatisticas.size() ? "," : "") << "\n";
    }
    arq << "  ]\n}\n";
    arq.close();
}

// =============================================================================
// ROTINAS DE BENCHMARK COM 10 REPETIÇÕES
// =============================================================================

void executarExperimentosStrings() {
    std::cout << "[1/3] Executando Bateria Científica: Strings (Trie vs. Patricia) - 10 Repetições..." << std::endl;
    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};
    std::vector<std::pair<std::string, std::string>> cenarios = {
        {"prefixos_densos_", "Prefixos_Densos"},
        {"prefixos_dispersos_", "Prefixos_Dispersos"}
    };

    for (const auto& c : cenarios) {
        for (size_t n : tamanhos) {
            std::string caminho = "benchmarks/datasets/1_strings/" + c.first + std::to_string(n) + ".txt";
            auto palavras = carregarLinhas(caminho);
            if (palavras.empty()) continue;

            // --- TRIE ---
            std::vector<double> temposInsTrie(NUM_REPETICOES), temposBuscaTrie(NUM_REPETICOES);
            size_t nosTrie = 0;
            for (int r = 0; r < NUM_REPETICOES; ++r) {
                Trie trie;
                auto t0 = high_resolution_clock::now();
                for (size_t i = 0; i < n; ++i) trie.insert(palavras[i]);
                auto t1 = high_resolution_clock::now();
                temposInsTrie[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                nosTrie = trie.getNodeCount();

                t0 = high_resolution_clock::now();
                for (size_t i = 0; i < n; ++i) trie.search(palavras[i]);
                t1 = high_resolution_clock::now();
                temposBuscaTrie[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
            }

            double medIns, desvIns, minIns, maxIns;
            calcularEstatisticas(temposInsTrie, medIns, desvIns, minIns, maxIns);
            estatisticas.push_back({"Strings", "Trie", c.second, "Insercao", n, medIns, desvIns, minIns, maxIns, (medIns * 1e9) / n, 0, nosTrie, estimarMemoriaBytes("Trie", nosTrie, n), "Construcao"});

            double medB, desvB, minB, maxB;
            calcularEstatisticas(temposBuscaTrie, medB, desvB, minB, maxB);
            estatisticas.push_back({"Strings", "Trie", c.second, "Busca", n, medB, desvB, minB, maxB, (medB * 1e9) / n, 0, nosTrie, estimarMemoriaBytes("Trie", nosTrie, n), "100% Sucesso"});

            // --- PATRICIA ---
            std::vector<double> temposInsPat(NUM_REPETICOES), temposBuscaPat(NUM_REPETICOES);
            size_t nosPat = 0;
            unsigned long long splitsPat = 0;
            for (int r = 0; r < NUM_REPETICOES; ++r) {
                PatriciaTree pat;
                auto t0 = high_resolution_clock::now();
                for (size_t i = 0; i < n; ++i) pat.insert(palavras[i]);
                auto t1 = high_resolution_clock::now();
                temposInsPat[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                nosPat = pat.getNodeCount();
                splitsPat = pat.getSplitCount();

                t0 = high_resolution_clock::now();
                for (size_t i = 0; i < n; ++i) pat.search(palavras[i]);
                t1 = high_resolution_clock::now();
                temposBuscaPat[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
            }

            calcularEstatisticas(temposInsPat, medIns, desvIns, minIns, maxIns);
            estatisticas.push_back({"Strings", "Patricia", c.second, "Insercao", n, medIns, desvIns, minIns, maxIns, (medIns * 1e9) / n, splitsPat, nosPat, estimarMemoriaBytes("Patricia", nosPat, n), "Splits: " + std::to_string(splitsPat)});

            calcularEstatisticas(temposBuscaPat, medB, desvB, minB, maxB);
            estatisticas.push_back({"Strings", "Patricia", c.second, "Busca", n, medB, desvB, minB, maxB, (medB * 1e9) / n, 0, nosPat, estimarMemoriaBytes("Patricia", nosPat, n), "100% Sucesso"});

            std::cout << "  -> " << c.second << " N = " << std::setw(6) << n 
                      << " | Trie: " << std::fixed << std::setprecision(5) << temposInsTrie[0] << "s (" << nosTrie << " nos)"
                      << " | Patricia: " << std::fixed << std::setprecision(5) << temposInsPat[0] << "s (" << nosPat << " nos)" << std::endl;
        }
    }
}

void executarExperimentosNumericos() {
    std::cout << "\n[2/3] Executando Bateria Científica: Numéricos (Splay, Treap, AVL, BST) - 10 Repetições..." << std::endl;
    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};
    std::vector<std::pair<std::string, std::string>> cenarios = {
        {"aleatorio_uniforme_", "Aleatorio_Uniforme"},
        {"ordenado_crescente_", "Ordenado_Crescente"},
        {"ordenado_decrescente_", "Ordenado_Decrescente"},
        {"quase_ordenado_", "Quase_Ordenado"}
    };

    for (const auto& c : cenarios) {
        for (size_t n : tamanhos) {
            std::string caminho = "benchmarks/datasets/2_numericos/" + c.first + std::to_string(n) + ".txt";
            auto chaves = carregarInteiros(caminho);
            if (chaves.empty()) continue;

            // 1. TREAP
            std::vector<double> temposTreap(NUM_REPETICOES);
            unsigned long long rotTreap = 0;
            for (int r = 0; r < NUM_REPETICOES; ++r) {
                Treap<int> treap(42 + r);
                auto t0 = high_resolution_clock::now();
                for (int v : chaves) treap.insert(v);
                auto t1 = high_resolution_clock::now();
                temposTreap[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                rotTreap = treap.getRotationCount();
            }
            double medT, desvT, minT, maxT;
            calcularEstatisticas(temposTreap, medT, desvT, minT, maxT);
            estatisticas.push_back({"Numericos", "Treap", c.second, "Insercao", n, medT, desvT, minT, maxT, (medT * 1e9) / n, rotTreap, n, estimarMemoriaBytes("Treap", n, n), "Rotacoes: " + std::to_string(rotTreap)});

            // 2. AVL
            std::vector<double> temposAVL(NUM_REPETICOES);
            unsigned long long rotAVL = 0;
            for (int r = 0; r < NUM_REPETICOES; ++r) {
                AVLTree<int> avl;
                auto t0 = high_resolution_clock::now();
                for (int v : chaves) avl.insert(v);
                auto t1 = high_resolution_clock::now();
                temposAVL[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                rotAVL = avl.getRotationCount();
            }
            calcularEstatisticas(temposAVL, medT, desvT, minT, maxT);
            estatisticas.push_back({"Numericos", "AVLTree", c.second, "Insercao", n, medT, desvT, minT, maxT, (medT * 1e9) / n, rotAVL, n, estimarMemoriaBytes("AVLTree", n, n), "Rotacoes: " + std::to_string(rotAVL)});

            // 3. SPLAY
            std::vector<double> temposSplay(NUM_REPETICOES);
            unsigned long long rotSplay = 0;
            for (int r = 0; r < NUM_REPETICOES; ++r) {
                SplayTree<int> splay;
                auto t0 = high_resolution_clock::now();
                for (int v : chaves) splay.insert(v);
                auto t1 = high_resolution_clock::now();
                temposSplay[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                rotSplay = splay.getRotationCount();
            }
            calcularEstatisticas(temposSplay, medT, desvT, minT, maxT);
            estatisticas.push_back({"Numericos", "SplayTree", c.second, "Insercao", n, medT, desvT, minT, maxT, (medT * 1e9) / n, rotSplay, n, estimarMemoriaBytes("SplayTree", n, n), "Rotacoes: " + std::to_string(rotSplay)});

            // 4. BST (Apenas para n <= 10000 em ordenados, para evitar estouro de pilha)
            if (c.second.find("Ordenado") != std::string::npos && n > 10000) {
                // omitido
            } else {
                std::vector<double> temposBST(NUM_REPETICOES);
                unsigned long long compBST = 0;
                for (int r = 0; r < NUM_REPETICOES; ++r) {
                    BST<int> bst;
                    auto t0 = high_resolution_clock::now();
                    for (int v : chaves) bst.insert(v);
                    auto t1 = high_resolution_clock::now();
                    temposBST[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                    compBST = bst.getComparisonCount();
                }
                calcularEstatisticas(temposBST, medT, desvT, minT, maxT);
                estatisticas.push_back({"Numericos", "BST", c.second, "Insercao", n, medT, desvT, minT, maxT, (medT * 1e9) / n, compBST, n, estimarMemoriaBytes("BST", n, n), "Comparacoes: " + std::to_string(compBST)});
            }

            std::cout << "  -> " << c.second << " N = " << std::setw(6) << n 
                      << " | Treap: " << std::fixed << std::setprecision(5) << medT << "s"
                      << " | AVL: " << std::fixed << std::setprecision(5) << temposAVL[0] << "s"
                      << " | Splay: " << std::fixed << std::setprecision(5) << temposSplay[0] << "s" << std::endl;
        }
    }

    // Localidade Temporal Zipf 80-20 e 90-10 (10 repetições de 100.000 buscas em N=50.000)
    // As buscas são geradas como ÍNDICES das chaves realmente inseridas para garantir 100% de hits
    // e exercitar corretamente o autoajuste dinâmico da Splay Tree.
    std::cout << "  [LOCALIDADE TEMPORAL] Avaliando consultas Zipf 80-20 e 90-10 (10 repetições)..." << std::endl;
    auto chaves50k = carregarInteiros("benchmarks/datasets/2_numericos/aleatorio_uniforme_50000.txt");
    auto idxZipf80 = carregarInteiros("benchmarks/datasets/2_numericos/consultas_zipf_80_20_100k.txt");
    auto idxZipf90 = carregarInteiros("benchmarks/datasets/2_numericos/consultas_zipf_90_10_100k.txt");

    // Converter índices Zipf em chaves reais (índice % N para garantir bounds)
    // O arquivo Zipf contém valores em [1..N-1], usamos como índices dentro de chaves50k
    std::vector<int> buscasZipf80, buscasZipf90;
    for (int idx : idxZipf80) buscasZipf80.push_back(chaves50k[std::abs(idx) % chaves50k.size()]);
    for (int idx : idxZipf90) buscasZipf90.push_back(chaves50k[std::abs(idx) % chaves50k.size()]);

    auto avaliarZipf = [&](const std::string& nomeZipf, const std::vector<int>& buscas) {
        std::vector<double> tSplay(NUM_REPETICOES), tAVL(NUM_REPETICOES), tTreap(NUM_REPETICOES);
        unsigned long long rotSplayZipf = 0;

        for (int r = 0; r < NUM_REPETICOES; ++r) {
            SplayTree<int> splay;
            AVLTree<int> avl;
            Treap<int> treap(42 + r);

            for (int v : chaves50k) { splay.insert(v); avl.insert(v); treap.insert(v); }

            splay.resetMetrics();
            auto t0 = high_resolution_clock::now();
            for (int b : buscas) splay.search(b);
            auto t1 = high_resolution_clock::now();
            tSplay[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
            rotSplayZipf = splay.getRotationCount();

            t0 = high_resolution_clock::now();
            for (int b : buscas) avl.search(b);
            t1 = high_resolution_clock::now();
            tAVL[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;

            t0 = high_resolution_clock::now();
            for (int b : buscas) treap.search(b);
            t1 = high_resolution_clock::now();
            tTreap[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
        }

        double med, desv, minT, maxT;
        calcularEstatisticas(tSplay, med, desv, minT, maxT);
        estatisticas.push_back({"Localidade_Temporal", "SplayTree", nomeZipf, "Busca_100k", 100000, med, desv, minT, maxT, (med * 1e9) / 100000, rotSplayZipf, 50000, estimarMemoriaBytes("SplayTree", 50000, 50000), "Autoajuste Zipf 100% hits"});

        calcularEstatisticas(tAVL, med, desv, minT, maxT);
        estatisticas.push_back({"Localidade_Temporal", "AVLTree", nomeZipf, "Busca_100k", 100000, med, desv, minT, maxT, (med * 1e9) / 100000, 0, 50000, estimarMemoriaBytes("AVLTree", 50000, 50000), "Balanceamento Fixo 100% hits"});

        calcularEstatisticas(tTreap, med, desv, minT, maxT);
        estatisticas.push_back({"Localidade_Temporal", "Treap", nomeZipf, "Busca_100k", 100000, med, desv, minT, maxT, (med * 1e9) / 100000, 0, 50000, estimarMemoriaBytes("Treap", 50000, 50000), "Probabilistico 100% hits"});
    };

    avaliarZipf("Zipf_80_20", buscasZipf80);
    avaliarZipf("Zipf_90_10", buscasZipf90);
}

void executarExperimentosEspaciais() {
    std::cout << "[3/3] KD-Tree 2D (10 repeticoes)..." << std::endl;
    std::vector<size_t> tamanhos = {100, 1000, 10000, 50000, 100000};

    // Carrega alvos e caixas
    std::vector<Ponto<2>> alvosNN;
    std::ifstream arqNN("benchmarks/datasets/3_espaciais/consultas_pontos_alvo_nn.txt");
    double x, y;
    while (arqNN >> x >> y) alvosNN.push_back(Ponto<2>({x, y}));
    arqNN.close();

    struct Caixa { Ponto<2> minPt; Ponto<2> maxPt; };
    std::vector<Caixa> caixas;
    std::ifstream arqR("benchmarks/datasets/3_espaciais/consultas_caixas_range.txt");
    double x1, y1, x2, y2;
    while (arqR >> x1 >> y1 >> x2 >> y2) caixas.push_back({Ponto<2>({x1, y1}), Ponto<2>({x2, y2})});
    arqR.close();

    std::vector<std::pair<std::string, std::string>> cenarios = {
        {"uniformes_2d_", "Uniformes_2D"},
        {"clusters_2d_", "Clusters_2D"}
    };

    for (const auto& c : cenarios) {
        for (size_t n : tamanhos) {
            std::string caminho = "benchmarks/datasets/3_espaciais/" + c.first + std::to_string(n) + ".txt";
            std::ifstream arq(caminho);
            std::vector<Ponto<2>> pts;
            while (arq >> x >> y) pts.push_back(Ponto<2>({x, y}));
            arq.close();

            std::vector<double> temposConstrucao(NUM_REPETICOES), temposNN(NUM_REPETICOES), temposRange(NUM_REPETICOES);
            unsigned long long calcDistNN = 0, podasRange = 0;

            for (int r = 0; r < NUM_REPETICOES; ++r) {
                KDTree<2> kd;
                auto t0 = high_resolution_clock::now();
                for (const auto& pt : pts) kd.insert(pt);
                auto t1 = high_resolution_clock::now();
                temposConstrucao[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;

                kd.resetMetrics();
                t0 = high_resolution_clock::now();
                for (const auto& alvo : alvosNN) kd.nearestNeighbor(alvo);
                t1 = high_resolution_clock::now();
                temposNN[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                calcDistNN = kd.getDistanceComputations();

                kd.resetMetrics();
                t0 = high_resolution_clock::now();
                for (const auto& cx : caixas) kd.rangeSearch(cx.minPt, cx.maxPt);
                t1 = high_resolution_clock::now();
                temposRange[r] = duration_cast<nanoseconds>(t1 - t0).count() / 1e9;
                podasRange = kd.getPruningCount();
            }

            double med, desv, minT, maxT;
            calcularEstatisticas(temposConstrucao, med, desv, minT, maxT);
            estatisticas.push_back({"Espacial", "KDTree_2D", c.second, "Construcao", n, med, desv, minT, maxT, (med * 1e9) / n, 0, n, estimarMemoriaBytes("KDTree_2D", n, n), "Particao espacial"});

            calcularEstatisticas(temposNN, med, desv, minT, maxT);
            estatisticas.push_back({"Espacial", "KDTree_2D", c.second, "Nearest_Neighbor_2kx", n, med, desv, minT, maxT, (med * 1e9) / alvosNN.size(), calcDistNN, n, estimarMemoriaBytes("KDTree_2D", n, n), "Distancias calculadas: " + std::to_string(calcDistNN)});

            calcularEstatisticas(temposRange, med, desv, minT, maxT);
            estatisticas.push_back({"Espacial", "KDTree_2D", c.second, "Range_Search_2kx", n, med, desv, minT, maxT, (med * 1e9) / caixas.size(), podasRange, n, estimarMemoriaBytes("KDTree_2D", n, n), "Podas de caixas: " + std::to_string(podasRange)});

            std::cout << "  -> " << c.second << " N = " << std::setw(6) << n 
                      << " | 2k NN: " << std::fixed << std::setprecision(5) << temposNN[0] << "s"
                      << " (Distancias calc: " << (calcDistNN / alvosNN.size()) << "/consulta)"
                      << " | 2k Range: " << std::fixed << std::setprecision(5) << temposRange[0] << "s" << std::endl;
        }
    }
}

int main() {
    std::cout << "Executando benchmarks experimentais (10 repeticoes)..." << std::endl;

    executarExperimentosStrings();
    executarExperimentosNumericos();
    executarExperimentosEspaciais();

    salvarCSV("benchmarks/dados_estatisticos_10execucoes.csv");
    salvarJSON("benchmarks/dados_estatisticos_10execucoes.json");

    std::cout << "\nResultados salvos:" << std::endl;
    std::cout << "  - benchmarks/dados_estatisticos_10execucoes.csv" << std::endl;
    std::cout << "  - benchmarks/dados_estatisticos_10execucoes.json" << std::endl;
    return 0;
}
