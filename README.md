# Estruturas em Árvores Avançadas (AEDS II)

Trabalho Prático Individual I de **Algoritmos e Estruturas de Dados II**.

## 📌 Estruturas Implementadas
1. **Árvore Trie** (*Prefix Tree*)
2. **Árvore Patricia** (*Compact Radix Tree*)
3. **Árvore Splay** (*Self-Adjusting Binary Search Tree*)
4. **Árvore Treap** (*Randomized BST + Heap*)
5. **KD-Tree** (*k-dimensional Tree*)
6. *(Baselines de Comparação)* **BST Padrão** e **Árvore AVL**

---

## 📁 Estrutura do Repositório

```text
├── include/                  # Cabeçalhos das estruturas (.hpp)
│   ├── trie.hpp
│   ├── patricia.hpp
│   ├── splay_tree.hpp
│   ├── treap.hpp
│   ├── kd_tree.hpp
│   ├── bst.hpp
│   └── avl_tree.hpp
├── src/                      # Código-fonte e ponto de entrada interativo
│   └── main.cpp
├── visualizer/               # Rastreamento de estados e exportação Graphviz (.dot/PNG)
│   ├── dot_exporter.hpp
│   └── generate_traces.cpp
├── benchmarks/               # Framework de experimentos e análise experimental
│   ├── benchmark_runner.cpp
│   ├── datasets/
│   └── plot_results.py
├── tests/                    # Testes unitários e casos de borda
│   └── test_trees.cpp
├── relatorio/                # Artigo/Relatório Técnico (8 a 12 páginas)
│   ├── main.tex
│   ├── secoes/
│   ├── figuras/
│   └── referencias.bib
├── assets/                   # Diagramas e saídas visuais
├── Makefile                  # Script de compilação Make
└── CMakeLists.txt            # Configuração CMake
```

---

## 🛠️ Como Compilar e Executar

### Pré-requisitos
- Compilador C++ com suporte a C++17 (`g++` ou `clang++`)
- `make` ou `cmake`
- `python3` com `matplotlib` (para geração de gráficos)
- `graphviz` (`dot`) (para renderização visual de árvores)

### Comandos de Compilação (via Makefile)
```bash
# Compilar todos os executáveis
make all

# Executar testes unitários
make test

# Gerar rastreamentos visuais das árvores (Graphviz)
make traces

# Executar experimentos de benchmark e gerar gráficos
make bench
```
