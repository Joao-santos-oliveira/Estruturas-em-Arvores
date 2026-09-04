# Modelagem, Implementação e Análise Comparativa de Estruturas em Árvores Especializadas

Trabalho Prático Individual I — Algoritmos e Estruturas de Dados II (AEDS II)

---

## 1. Visão Geral

Este repositório contém o estudo, implementação em C++17 e avaliação experimental de cinco estruturas em árvores não convencionais, além de duas estruturas de referência (*baselines*):

1. **Árvore Trie** (*Prefix Tree*)
2. **Árvore Patricia** (*Radix Tree compacta*)
3. **Árvore Splay** (*Self-Adjusting Binary Search Tree*)
4. **Árvore Treap** (*Tree + Heap*)
5. **KD-Tree** (*k-Dimensional Tree, 2D*)
6. **Baselines:** Árvore Binária de Busca Padrão (**BST**) e Árvore **AVL**

---

## 2. Estrutura do Repositório

```text
Estruturas-em-Arvores/
├── include/                     # Implementações em C++17 das estruturas
│   ├── trie.hpp                 # Trie com busca por prefixo, autocomplete e exportação JSON
│   ├── patricia.hpp             # Patricia Tree com compactação de arestas (split/merge)
│   ├── splay_tree.hpp           # Splay Tree com passos Zig, Zig-Zig e Zig-Zag
│   ├── treap.hpp                # Treap com rotações e prioridades de Max-Heap
│   ├── kd_tree.hpp              # KD-Tree com Nearest Neighbor e Range Search
│   ├── bst.hpp                  # Baseline: Árvore Binária de Busca
│   └── avl_tree.hpp             # Baseline: Árvore AVL
├── tests/
│   └── test_trees.cpp           # Testes unitários e verificação de casos de borda
├── benchmarks/
│   ├── datasets/                # Conjuntos de dados gerados (N = 100 a 100.000)
│   │   ├── 1_strings/           # Prefixos densos e dispersos
│   │   ├── 2_numericos/         # Ordenados, aleatórios, quase-ordenados e Zipf
│   │   ├── 3_espaciais/         # Distribuições uniformes 2D e clusters gaussianos
│   │   └── gerar_datasets.py    # Script de geração dos datasets
│   ├── executar_benchmarks.cpp  # Painel interativo para testes individuais e ciclos de operações
│   ├── executar_experimentos_cientificos.cpp # Bateria com 10 repetições por configuração
│   └── gerar_graficos_cientificos.py        # Geração dos gráficos comparativos em SVG
├── relatorio/
│   ├── secoes/                  # Seções em LaTeX do relatório técnico
│   └── figuras/                 # Figuras vetoriais (SVG) para o relatório
└── Makefile                     # Regras de compilação e automação
```

---

## 3. Compilação e Execução

O projeto utiliza `g++` com suporte a C++17 e otimização `-O3`.

### 3.1. Compilar os binários
```bash
make all
```

### 3.2. Executar testes unitários
Valida as operações de inserção, busca, remoção, rotações, cortes espaciais e propriedades estruturais:
```bash
make test
```

### 3.3. Gerar os datasets de teste
Gera os arquivos de dados em `benchmarks/datasets/`:
```bash
make datasets
```

### 3.4. Executar o painel interativo de benchmarks
Permite selecionar a estrutura, o tipo de dado e o tamanho de entrada para teste no terminal:
```bash
make bench
```

### 3.5. Executar os experimentos com repetições (10 repetições)
Executa a matriz completa de experimentos com 10 repetições por configuração, salvando médias, desvios-padrão e métricas em CSV e JSON:
```bash
make bench-cientifico
```

### 3.6. Gerar os gráficos comparativos (SVG)
Gera as figuras vetoriais em `relatorio/figuras/`:
```bash
make plot
```

As figuras produzidas incluem:
- `fig1_trie_patricia_insercao.svg`: Tempo de inserção de Trie vs. Patricia em prefixos densos e dispersos.
- `fig2_trie_patricia_nos.svg`: Contagem de nós alocados (compressão de arestas da Patricia).
- `fig3_pior_caso_ordenado.svg`: Pior caso com chaves ordenadas (BST vs. AVL, Treap e Splay).
- `fig4_insercao_4_distribuicoes.svg`: AVL vs. Treap vs. Splay sob 4 distribuições de entrada.
- `fig6_rotacoes_acumuladas.svg`: Custo de reorganização estrutural (rotações acumuladas).
- `fig7_kdtree_consultas.svg`: Tempo de 1-NN e Range Query na KD-Tree (uniforme vs. clusters).

### 3.7. Executar a Interface Web Interativa (GitHub Pages)
Inicia o servidor local para visualizar a interface web estática localizada em `docs/`:
```bash
make web
```
Ou abra diretamente o arquivo `docs/index.html` em qualquer navegador moderno. Quando publicado no GitHub, basta ativar o **GitHub Pages** apontando para a pasta `/docs` da branch principal.

### 3.8. Limpar binários e dados temporários
```bash
make clean
```
