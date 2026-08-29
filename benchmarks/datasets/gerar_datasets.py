#!/usr/bin/env python3
"""
=============================================================================
GERADOR DE DATASETS EXPERIMENTAIS PARA ÁRVORES AVANÇADAS (AEDS II)
=============================================================================
Gera a matriz completa de dados para avaliação experimental:
1. Strings (Prefixos Densos, Dispersos, Pior Caso) para Trie vs. Patricia
2. Numéricos (Crescente, Decrescente, Aleatório, Zipf 80-20/90-10) para Splay, Treap, AVL, BST
3. Espaciais 2D/3D (Uniformes, Clusters Gaussianos, Consultas NN e Range) para KD-Tree

Tamanhos de entrada: N in {100, 1000, 10000, 50000, 100000}
=============================================================================
"""

import os
import random
import string
import math

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DIR_STRINGS = os.path.join(BASE_DIR, "1_strings")
DIR_NUMERICOS = os.path.join(BASE_DIR, "2_numericos")
DIR_ESPACIAIS = os.path.join(BASE_DIR, "3_espaciais")

TAMANHOS_N = [100, 1000, 10000, 50000, 100000]

def criar_pastas():
    for d in [DIR_STRINGS, DIR_NUMERICOS, DIR_ESPACIAIS]:
        os.makedirs(d, exist_ok=True)

# =============================================================================
# 1. CATEGORIA: STRINGS (TRIE vs. PATRICIA TREE)
# =============================================================================
def gerar_strings():
    print("[1/3] Gerando Datasets de Strings (Trie vs. Patricia)...")
    random.seed(42)

    prefixos_reais = [
        "comput", "desenvolv", "estrutur", "algorit", "trans", "inter",
        "super", "micro", "macro", "multi", "infor", "tele", "foto", "auto",
        "hiper", "proto", "termo", "neuro", "bio", "geo", "cripto", "sub"
    ]
    sufixos_reais = [
        "acao", "ador", "amento", "avel", "idade", "ismo", "ista",
        "mente", "izar", "eza", "oso", "eiro", "ario", "ante", "encia", "ivel"
    ]

    # Gera banco de palavras com prefixos densos
    banco_densas = set()
    while len(banco_densas) < 100000:
        p = random.choice(prefixos_reais)
        s = random.choice(sufixos_reais)
        meio = "".join(random.choices(string.ascii_lowercase, k=random.randint(1, 4)))
        banco_densas.add(p + meio + s)
    lista_densas = list(banco_densas)

    # Gera banco de strings dispersas/aleatórias (baixa sobreposição)
    banco_dispersas = set()
    while len(banco_dispersas) < 100000:
        tam = random.randint(6, 16)
        s = "".join(random.choices(string.ascii_lowercase, k=tam))
        banco_dispersas.add(s)
    lista_dispersas = list(banco_dispersas)

    # Grava por tamanho N
    for n in TAMANHOS_N:
        # Prefixos densos
        arq_densas = os.path.join(DIR_STRINGS, f"prefixos_densos_{n}.txt")
        with open(arq_densas, "w", encoding="utf-8") as f:
            for i in range(n):
                f.write(lista_densas[i] + "\n")

        # Prefixos dispersos
        arq_dispersas = os.path.join(DIR_STRINGS, f"prefixos_dispersos_{n}.txt")
        with open(arq_dispersas, "w", encoding="utf-8") as f:
            for i in range(n):
                f.write(lista_dispersas[i] + "\n")

    # Pior caso: cadeia linear longa (palavra única com 120 caracteres)
    arq_pior = os.path.join(DIR_STRINGS, "pior_caso_cadeia_longa.txt")
    with open(arq_pior, "w", encoding="utf-8") as f:
        f.write("a" * 120 + "\n")
        f.write("a" * 119 + "b\n")

    # Consultas de busca (10.000 presentes + 10.000 ausentes)
    arq_consultas = os.path.join(DIR_STRINGS, "consultas_strings_busca.txt")
    with open(arq_consultas, "w", encoding="utf-8") as f:
        # 10.000 presentes
        for i in range(10000):
            f.write(f"PRESENTE {lista_densas[i]}\n")
        # 10.000 ausentes
        for _ in range(10000):
            ausente = "xyz" + "".join(random.choices(string.ascii_lowercase, k=8))
            f.write(f"AUSENTE {ausente}\n")

    print(f"  -> Concluído: {len(TAMANHOS_N)*2 + 2} arquivos de strings gerados.")

# =============================================================================
# 2. CATEGORIA: NUMÉRICOS (SPLAY, TREAP, AVL, BST)
# =============================================================================
def gerar_numericos():
    print("[2/3] Gerando Datasets Numéricos (Splay, Treap, AVL, BST)...")
    random.seed(42)

    for n in TAMANHOS_N:
        # 1. Ordenado Crescente (1 .. N) -> Pior caso clássico da BST
        arq_cresc = os.path.join(DIR_NUMERICOS, f"ordenado_crescente_{n}.txt")
        with open(arq_cresc, "w") as f:
            for i in range(1, n + 1):
                f.write(f"{i}\n")

        # 2. Ordenado Decrescente (N .. 1)
        arq_decresc = os.path.join(DIR_NUMERICOS, f"ordenado_decrescente_{n}.txt")
        with open(arq_decresc, "w") as f:
            for i in range(n, 0, -1):
                f.write(f"{i}\n")

        # 3. Aleatório Uniforme (Permutação de 1 .. N)
        vals_aleat = list(range(1, n + 1))
        random.shuffle(vals_aleat)
        arq_aleat = os.path.join(DIR_NUMERICOS, f"aleatorio_uniforme_{n}.txt")
        with open(arq_aleat, "w") as f:
            for v in vals_aleat:
                f.write(f"{v}\n")

        # 4. Quase Ordenado (95% ordenado com 5% de swaps)
        vals_quase = list(range(1, n + 1))
        num_swaps = max(1, int(n * 0.05))
        for _ in range(num_swaps):
            i1 = random.randint(0, n - 1)
            i2 = random.randint(0, n - 1)
            vals_quase[i1], vals_quase[i2] = vals_quase[i2], vals_quase[i1]
        arq_quase = os.path.join(DIR_NUMERICOS, f"quase_ordenado_{n}.txt")
        with open(arq_quase, "w") as f:
            for v in vals_quase:
                f.write(f"{v}\n")

    # Sequências de Buscas com Localidade Temporal (Zipf 80-20 e 90-10)
    universo = list(range(1, 50001))
    
    # Zipf 80-20: 20% das chaves recebem 80% das 100.000 buscas
    corte_20 = int(len(universo) * 0.2)
    populares_80 = universo[:corte_20]
    raras_20 = universo[corte_20:]
    arq_zipf_80 = os.path.join(DIR_NUMERICOS, "consultas_zipf_80_20_100k.txt")
    with open(arq_zipf_80, "w") as f:
        for _ in range(100000):
            k = random.choice(populares_80) if random.random() < 0.8 else random.choice(raras_20)
            f.write(f"{k}\n")

    # Zipf 90-10: 10% das chaves recebem 90% das 100.000 buscas (Altíssima localidade)
    corte_10 = int(len(universo) * 0.1)
    populares_90 = universo[:corte_10]
    raras_10 = universo[corte_10:]
    arq_zipf_90 = os.path.join(DIR_NUMERICOS, "consultas_zipf_90_10_100k.txt")
    with open(arq_zipf_90, "w") as f:
        for _ in range(100000):
            k = random.choice(populares_90) if random.random() < 0.9 else random.choice(raras_10)
            f.write(f"{k}\n")

    print(f"  -> Concluído: {len(TAMANHOS_N)*4 + 2} arquivos numéricos gerados.")

# =============================================================================
# 3. CATEGORIA: ESPACIAIS MULTIDIMENSIONAIS (KD-TREE 2D/3D)
# =============================================================================
def gerar_espaciais():
    print("[3/3] Gerando Datasets Espaciais (KD-Tree 2D e 3D)...")
    random.seed(42)

    # 1. Pontos Uniformes 2D
    for n in TAMANHOS_N:
        arq_2d = os.path.join(DIR_ESPACIAIS, f"uniformes_2d_{n}.txt")
        with open(arq_2d, "w") as f:
            for _ in range(n):
                x = round(random.uniform(-1000.0, 1000.0), 3)
                y = round(random.uniform(-1000.0, 1000.0), 3)
                f.write(f"{x} {y}\n")

    # 2. Pontos em Clusters Gaussianos 2D (5 centros de alta densidade)
    centros = [(-500.0, -500.0), (500.0, 500.0), (-400.0, 600.0), (700.0, -300.0), (0.0, 0.0)]
    for n in TAMANHOS_N:
        arq_cluster = os.path.join(DIR_ESPACIAIS, f"clusters_2d_{n}.txt")
        with open(arq_cluster, "w") as f:
            for _ in range(n):
                cx, cy = random.choice(centros)
                x = round(random.gauss(cx, 80.0), 3)
                y = round(random.gauss(cy, 80.0), 3)
                f.write(f"{x} {y}\n")

    # 3. Pontos Uniformes 3D
    for n in TAMANHOS_N:
        arq_3d = os.path.join(DIR_ESPACIAIS, f"uniformes_3d_{n}.txt")
        with open(arq_3d, "w") as f:
            for _ in range(n):
                x = round(random.uniform(-1000.0, 1000.0), 3)
                y = round(random.uniform(-1000.0, 1000.0), 3)
                z = round(random.uniform(-1000.0, 1000.0), 3)
                f.write(f"{x} {y} {z}\n")

    # 4. Consultas Espaciais: 2.000 pontos alvos para Nearest Neighbor
    arq_nn = os.path.join(DIR_ESPACIAIS, "consultas_pontos_alvo_nn.txt")
    with open(arq_nn, "w") as f:
        for _ in range(2000):
            x = round(random.uniform(-1200.0, 1200.0), 3)
            y = round(random.uniform(-1200.0, 1200.0), 3)
            f.write(f"{x} {y}\n")

    # 5. Consultas Espaciais: 2.000 caixas delimitadoras para Range Search (minX, minY, maxX, maxY)
    arq_range = os.path.join(DIR_ESPACIAIS, "consultas_caixas_range.txt")
    with open(arq_range, "w") as f:
        for _ in range(2000):
            cx = random.uniform(-800.0, 800.0)
            cy = random.uniform(-800.0, 800.0)
            raio = random.uniform(20.0, 200.0)
            f.write(f"{round(cx-raio, 3)} {round(cy-raio, 3)} {round(cx+raio, 3)} {round(cy+raio, 3)}\n")

    print(f"  -> Concluído: {len(TAMANHOS_N)*3 + 2} arquivos espaciais gerados.")

if __name__ == "__main__":
    criar_pastas()
    print("=================================================================")
    print(" GERADOR COMPLETO DE DATASETS EXPERIMENTAIS (AEDS II) ")
    print("=================================================================")
    gerar_strings()
    gerar_numericos()
    gerar_espaciais()
    print("=================================================================")
    print(">>> TODOS OS DATASETS FORAM CRIADOS COM SUCESSO EM benchmarks/datasets/ <<<")
    print("=================================================================")
