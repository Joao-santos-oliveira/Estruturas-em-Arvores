<div align="center">

![C++](https://img.shields.io/badge/C++-17-00599C?style=flat&logo=cplusplus)
![Python](https://img.shields.io/badge/Python-3.8+-3776AB?style=flat&logo=python)
![Web](https://img.shields.io/badge/Web-HTML%20%7C%20JS%20%7C%20CSS-E34F26?style=flat&logo=html5)
![Build](https://img.shields.io/badge/build-make-brightgreen?style=flat)
</div>

<div align="center">

# Estruturas em Árvores Especializadas

**Trabalho Prático Individual I — Algoritmos e Estruturas de Dados II (AEDS II)**

Modelagem, Implementação em C++ e Análise Comparativa Experimental de Árvores Não Convencionais.
</div>

---

## O que é este projeto?

Este repositório contém o estudo, implementação utilizando *templates* em C++17 e a avaliação experimental de cinco estruturas em árvores não convencionais, além de duas estruturas de referência (*baselines*). 

As estruturas implementadas são:
1. **Árvore Trie** (*Prefix Tree*)
2. **Árvore Patricia** (*Radix Tree compacta*)
3. **Árvore Splay** (*Self-Adjusting Binary Search Tree*)
4. **Árvore Treap** (*Tree + Heap*)
5. **KD-Tree** (*k-Dimensional Tree, 2D*)
6. **Baselines:** Árvore Binária de Busca Padrão (**BST**) e Árvore **AVL**

O foco principal do projeto é a comparação de desempenho (custo computacional, contagem de nós e rotações) sob diferentes distribuições de dados e cenários extremos (ex: pior caso com inserções ordenadas). Além dos testes automatizados, o projeto dispõe de uma interface web interativa para visualização em tempo real das operações de inserção, busca e balanceamento de cada uma das árvores.

---

## Estrutura do Projeto

```text
.
├── benchmarks/                  # Scripts e códigos para experimentação
│   ├── datasets/                # Conjuntos de dados gerados
│   ├── executar_benchmarks.cpp  # Painel interativo de testes
│   └── executar_experimentos_cientificos.cpp # Bateria de 10 execuções
├── docs/                        # Interface Web Interativa (HTML/JS/CSS)
├── include/                     # Implementações em C++ das estruturas (.hpp)
├── relatorio/                   # Relatório técnico e diretório de gráficos (SVG)
├── tests/                       # Testes unitários (test_trees.cpp)
└── Makefile                     # Regras de compilação
```

---

## Interface Web e Visualização

O projeto possui uma aplicação web estática (`docs/`) pronta para ser servida localmente ou no GitHub Pages. Ela permite:
- Visualização das 7 estruturas implementadas (com início em árvore vazia).
- Visualização dupla para KD-Tree (Árvore Hierárquica + Plano Cartesiano 2D).
- Cartões de nós contendo chave e prioridade (para Treap).
- Animação em 3 estados de rastreamento visual das operações (inserção, busca e balanceamento).

Para rodar localmente e abrir a visualização:
```bash
make web
```
*(O comando iniciará um servidor Python na porta 8000. Acesse http://localhost:8000)*

---

## Compilação, Execução e Testes

O projeto foi configurado para ser construído de forma simplificada através do `Makefile`, garantindo compatibilidade total com ambientes Linux.

### Pré-requisitos

Certifique-se de possuir o compilador C++ e o Python 3 instalados no seu ambiente:

```bash
sudo apt update
sudo apt install build-essential python3 python3-pip
```

Para a geração dos gráficos comparativos (`make plot`), instale as bibliotecas necessárias:

```bash
pip3 install matplotlib numpy
```

### Comandos Disponíveis

A automação é garantida pelas seguintes regras no `Makefile`:

| Comando | Função |
|---|---|
| `make clean` | Remove arquivos gerados em compilações anteriores |
| `make all` | Compila os binários utilizando o compilador `g++` com a flag `-O3` |
| `make test` | Executa os testes unitários garantindo a corretude estrutural das árvores |
| `make datasets` | Gera as massas de dados nas distribuições necessárias (100 a 100.000 registros) |
| `make bench` | Executa o painel interativo de testes individuais no terminal |
| `make bench-cientifico`| Roda 10 repetições dos benchmarks, filtrando outliers e salvando em JSON/CSV |
| `make plot` | Processa os dados de benchmark e gera as figuras vetoriais (SVG) para o relatório |

---

## Experimentos e Análise

Para gerar todo o conjunto de dados para o relatório, do zero, a sequência recomendada é:

1. `make datasets` (cria os arquivos base)
2. `make bench-cientifico` (roda a avaliação de desempenho automatizada)
3. `make plot` (gera os arquivos SVG)

As figuras produzidas analisam cenários de interesse prático, como o tempo de busca e economia de memória da **Patricia em relação à Trie**, bem como as penalidades impostas à **AVL** pelas constantes rotações, e a capacidade de adaptação da **Splay** às chaves mais acessadas.

---

## Autor

<table>
  <tr>
    <td align="center">
      <img src="https://github.com/Joao-santos-oliveira.png" width="100px"><br>
      <b>João Gabriel</b><br>
      <a href="https://github.com/Joao-santos-oliveira"><img src="https://img.shields.io/github/followers/Joao-santos-oliveira?label=Seguidores&style=social"></a>
    </td>
  </tr>
</table>
