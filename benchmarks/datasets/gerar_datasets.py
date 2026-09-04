#!/usr/bin/env python3
# Gerador de datasets para experimentos com estruturas em arvores (AEDS II)
# Gera matriz de dados para N in {100, 1000, 10000, 50000, 100000}:
# 1. Strings (Prefixos densos e dispersos)
# 2. Numericos (Ordenados, aleatorios, quase ordenados e Zipf)
# 3. Espaciais 2D (Uniformes e clusters gaussianos)

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

def gerar_strings():
    print("[1/3] Gerando datasets de strings...")
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

    banco_densas = set()
    while len(banco_densas) < 100000:
        p = random.choice(prefixos_reais)
        s = random.choice(sufixos_reais)
        meio = "".join(random.choices(string.ascii_lowercase, k=random.randint(1, 4)))
        banco_densas.add(p + meio + s)
    lista_densas = list(banco_densas)

    banco_dispersas = set()
    while len(banco_dispersas) < 100000:
        tam = random.randint(6, 16)
        banco_dispersas.add("".join(random.choices(string.ascii_lowercase, k=tam)))
    lista_dispersas = list(banco_dispersas)

    for n in TAMANHOS_N:
        with open(os.path.join(DIR_STRINGS, f"prefixos_densos_{n}.txt"), "w") as f:
            f.write("\n".join(lista_densas[:n]) + "\n")

        with open(os.path.join(DIR_STRINGS, f"prefixos_dispersos_{n}.txt"), "w") as f:
            f.write("\n".join(lista_dispersas[:n]) + "\n")

    # Cadeia longa para caso linear
    with open(os.path.join(DIR_STRINGS, "pior_caso_cadeia_longa.txt"), "w") as f:
        f.write("a" * 120 + "\n")

    # Consultas de busca
    with open(os.path.join(DIR_STRINGS, "consultas_strings_busca.txt"), "w") as f:
        amostra_presente = random.sample(lista_densas[:50000], 5000)
        amostra_ausente = ["".join(random.choices(string.ascii_uppercase, k=10)) for _ in range(5000)]
        f.write("\n".join(amostra_presente + amostra_ausente) + "\n")

def gerar_numericos():
    print("[2/3] Gerando datasets numericos...")
    random.seed(42)

    for n in TAMANHOS_N:
        # Ordenado crescente
        with open(os.path.join(DIR_NUMERICOS, f"ordenado_crescente_{n}.txt"), "w") as f:
            f.write("\n".join(str(i) for i in range(1, n + 1)) + "\n")

        # Ordenado decrescente
        with open(os.path.join(DIR_NUMERICOS, f"ordenado_decrescente_{n}.txt"), "w") as f:
            f.write("\n".join(str(i) for i in range(n, 0, -1)) + "\n")

        # Aleatorio uniforme
        vals_aleatorios = list(range(1, n + 1))
        random.shuffle(vals_aleatorios)
        with open(os.path.join(DIR_NUMERICOS, f"aleatorio_uniforme_{n}.txt"), "w") as f:
            f.write("\n".join(str(x) for x in vals_aleatorios) + "\n")

        # Quase ordenado (95% ordenado)
        vals_quase = list(range(1, n + 1))
        trocas = max(1, int(n * 0.05))
        for _ in range(trocas):
            i1 = random.randint(0, n - 1)
            i2 = random.randint(0, n - 1)
            vals_quase[i1], vals_quase[i2] = vals_quase[i2], vals_quase[i1]

        with open(os.path.join(DIR_NUMERICOS, f"quase_ordenado_{n}.txt"), "w") as f:
            f.write("\n".join(str(x) for x in vals_quase) + "\n")

    # Consultas Zipf
    def gerar_zipf_chaves(n_consultas, max_chave, s):
        pesos = [1.0 / (i ** s) for i in range(1, max_chave + 1)]
        soma = sum(pesos)
        probs = [p / soma for p in pesos]
        return random.choices(range(1, max_chave + 1), weights=probs, k=n_consultas)

    with open(os.path.join(DIR_NUMERICOS, "consultas_zipf_80_20_100k.txt"), "w") as f:
        chaves = gerar_zipf_chaves(100000, 50000, s=1.1)
        f.write("\n".join(str(x) for x in chaves) + "\n")

    with open(os.path.join(DIR_NUMERICOS, "consultas_zipf_90_10_100k.txt"), "w") as f:
        chaves = gerar_zipf_chaves(100000, 50000, s=1.5)
        f.write("\n".join(str(x) for x in chaves) + "\n")

def gerar_espaciais():
    print("[3/3] Gerando datasets espaciais 2D...")
    random.seed(42)

    for n in TAMANHOS_N:
        # Uniforme 2D [-1000, 1000]
        with open(os.path.join(DIR_ESPACIAIS, f"uniformes_2d_{n}.txt"), "w") as f:
            for _ in range(n):
                x = round(random.uniform(-1000.0, 1000.0), 4)
                y = round(random.uniform(-1000.0, 1000.0), 4)
                f.write(f"{x} {y}\n")

        # Clusters Gaussianos 2D
        centros = [(-500.0, -500.0), (500.0, 500.0), (-500.0, 500.0), (500.0, -500.0), (0.0, 0.0)]
        with open(os.path.join(DIR_ESPACIAIS, f"clusters_2d_{n}.txt"), "w") as f:
            for _ in range(n):
                cx, cy = random.choice(centros)
                x = round(random.gauss(cx, 80.0), 4)
                y = round(random.gauss(cy, 80.0), 4)
                f.write(f"{x} {y}\n")

    # Alvos para 1-NN
    with open(os.path.join(DIR_ESPACIAIS, "consultas_pontos_alvo_nn.txt"), "w") as f:
        for _ in range(2000):
            x = round(random.uniform(-1000.0, 1000.0), 4)
            y = round(random.uniform(-1000.0, 1000.0), 4)
            f.write(f"{x} {y}\n")

    # Caixas delimitadoras para Range Search
    with open(os.path.join(DIR_ESPACIAIS, "consultas_caixas_range.txt"), "w") as f:
        for _ in range(2000):
            cx = random.uniform(-800.0, 800.0)
            cy = random.uniform(-800.0, 800.0)
            dx = random.uniform(50.0, 200.0)
            dy = random.uniform(50.0, 200.0)
            x1, x2 = round(cx - dx / 2, 4), round(cx + dx / 2, 4)
            y1, y2 = round(cy - dy / 2, 4), round(cy + dy / 2, 4)
            f.write(f"{x1} {y1} {x2} {y2}\n")

if __name__ == "__main__":
    criar_pastas()
    gerar_strings()
    gerar_numericos()
    gerar_espaciais()
    print("Geracao de datasets concluida com sucesso.")
