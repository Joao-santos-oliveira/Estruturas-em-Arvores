#!/usr/bin/env python3
# Gerador de figuras SVG para o relatório — AEDS II
# Lê benchmarks/dados_estatisticos_10execucoes.json (média de 10 execuções)
# e exporta as figuras para relatorio/figuras/
#
# Figuras geradas (6):
#   fig1 — Trie vs Patricia: tempo de inserção (densos e dispersos)
#   fig2 — Trie vs Patricia: nós alocados (densos e dispersos)
#   fig3 — Pior caso: BST vs AVL, Treap, Splay em chaves ordenadas
#   fig4 — AVL vs Treap vs Splay: inserção em 4 distribuições
#   fig6 — Rotações acumuladas na inserção (caso aleatório)
#   fig7 — KD-Tree: 1-NN e Range Query por N (uniforme vs clusters)

import os, json, math

BASE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(BASE)
FIG  = os.path.join(ROOT, "relatorio", "figuras")
os.makedirs(FIG, exist_ok=True)

with open(os.path.join(BASE, "dados_estatisticos_10execucoes.json"), encoding="utf-8") as f:
    DATA = json.load(f)["experimentos"]

# ── lookup rápido ────────────────────────────────────────────────────────────
def get(estrutura, cenario, operacao):
    rows = [d for d in DATA
            if d["estrutura"] == estrutura
            and d["cenario"] == cenario
            and d["operacao"] == operacao]
    return sorted(rows, key=lambda r: r["tamanhoN"])

def pts(rows, campo="mediaTempo_s"):
    return [(r["tamanhoN"], r[campo]) for r in rows]

def pts_err(rows, campo="mediaTempo_s"):
    """Retorna (N, media, desvio)"""
    return [(r["tamanhoN"], r[campo], r.get("desvioPadrao_s", 0)) for r in rows]

# ── paleta de cores ──────────────────────────────────────────────────────────
COR = {
    "Trie_D":   "#E11D48",  # Crimson
    "Trie_Sp":  "#F87171",  # Crimson claro
    "Pat_D":    "#2563EB",  # Azul Real
    "Pat_Sp":   "#93C5FD",  # Azul claro
    "BST":      "#DC2626",  # Vermelho
    "AVL":      "#059669",  # Esmeralda
    "Treap":    "#D97706",  # Âmbar
    "Splay":    "#7C3AED",  # Roxo
    "KD_U":     "#0891B2",  # Ciano
    "KD_C":     "#DB2777",  # Rosa
}
DASH = [None, "6,3", "3,3", "9,3"]

# =============================================================================
# MOTOR SVG DE ALTA QUALIDADE
# =============================================================================

def _svg_open(W, H):
    lines = []
    lines.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" '
                 f'width="{W}" height="{H}">')
    lines.append(f'<rect width="{W}" height="{H}" fill="#FFFFFF" rx="8" '
                 f'stroke="#CBD5E1" stroke-width="1"/>')
    lines.append("""<style>
  text { font-family: 'Segoe UI', Helvetica, Arial, sans-serif; }
  .tag  { font-size:9.5px; font-weight:700; fill:#2563EB; letter-spacing:.7px; }
  .ttl  { font-size:14.5px; font-weight:800; fill:#0F172A; }
  .sub  { font-size:10.5px; fill:#64748B; }
  .axl  { font-size:11.5px; font-weight:700; fill:#334155; }
  .tck  { font-size:9.5px;  font-weight:500; fill:#64748B;
          font-family:'Courier New',monospace; }
  .grid { stroke:#E2E8F0; stroke-width:.9; stroke-dasharray:4,4; }
  .ax   { stroke:#94A3B8; stroke-width:1.5; }
  .lbl  { font-size:9.5px; font-weight:700; }
  .lcrd { fill:#FFFFFF; stroke:#E2E8F0; stroke-width:1; }
</style>""")
    return lines

def _header(svg, pL, titulo, subtitulo, tag):
    tag_w = max(180, len(tag) * 6.5 + 20)
    svg.append(f'<rect x="{pL}" y="16" width="{tag_w}" height="16" fill="#EFF6FF" rx="3"/>')
    svg.append(f'<text x="{pL+7}" y="28" class="tag">{tag}</text>')
    svg.append(f'<text x="{pL}" y="50" class="ttl">{titulo}</text>')
    svg.append(f'<text x="{pL}" y="66" class="sub">{subtitulo}  •  Média de 10 Execuções Independentes</text>')

def _axes(svg, pL, pT, pW, pH, min_x, max_x, min_y, max_y,
          log_x, log_y, x_label, y_label, y_unit="", n_ticks_y=5):

    def mx(v):
        if log_x: return pL + (math.log10(max(1,v))-math.log10(min_x)) / \
                          (math.log10(max_x)-math.log10(min_x)) * pW
        return pL + (v - min_x)/(max_x - min_x) * pW

    def my(v):
        if log_y:
            v = max(1e-12, v)
            return pT + pH - (math.log10(v)-math.log10(min_y)) / \
                          (math.log10(max_y)-math.log10(min_y)) * pH
        return pT + pH - (v - min_y)/(max_y - min_y) * pH

    # grade Y
    if log_y:
        d0 = math.floor(math.log10(min_y))
        d1 = math.ceil(math.log10(max_y))
        ty_list = [10**d for d in range(d0, d1+1)]
    else:
        step = (max_y - min_y) / n_ticks_y
        ty_list = [min_y + i*step for i in range(n_ticks_y+1)]

    for ty in ty_list:
        py = my(ty)
        if pT-1 <= py <= pT+pH+1:
            svg.append(f'<line x1="{pL}" y1="{py:.1f}" x2="{pL+pW}" y2="{py:.1f}" class="grid"/>')
            if ty >= 1:
                label = f"{int(ty):,}" if ty >= 100 else f"{ty:.2f}"
            elif ty >= 1e-3:
                label = f"{ty:.4f}"
            else:
                label = f"{ty:.1e}"
            svg.append(f'<text x="{pL-8}" y="{py+3.5:.1f}" text-anchor="end" class="tck">{label}{y_unit}</text>')

    # grade X
    tx_list = [100, 1000, 10000, 50000, 100000] if log_x else sorted(set([min_x, max_x]))
    for tx in tx_list:
        px = mx(tx)
        if pL-1 <= px <= pL+pW+1:
            svg.append(f'<line x1="{px:.1f}" y1="{pT}" x2="{px:.1f}" y2="{pT+pH}" class="grid"/>')
            lbl = f"{tx:,}" if tx >= 1000 else str(tx)
            svg.append(f'<text x="{px:.1f}" y="{pT+pH+18}" text-anchor="middle" class="tck">{lbl}</text>')

    # eixos
    svg.append(f'<line x1="{pL}" y1="{pT+pH}" x2="{pL+pW}" y2="{pT+pH}" class="ax"/>')
    svg.append(f'<line x1="{pL}" y1="{pT}"   x2="{pL}"    y2="{pT+pH}" class="ax"/>')

    # rótulos dos eixos
    cx = pL + pW/2
    svg.append(f'<text x="{cx:.1f}" y="{pT+pH+40}" text-anchor="middle" class="axl">{x_label}</text>')
    cy = pT + pH/2
    svg.append(f'<text x="22" y="{cy:.1f}" text-anchor="middle" '
               f'transform="rotate(-90 22 {cy:.1f})" class="axl">{y_label}</text>')

    return mx, my

def _legend_card(svg, itens, x, y, cols=1):
    """itens = [(label, cor, dash)]"""
    per_col = math.ceil(len(itens)/cols)
    col_w = 185
    card_w = col_w * cols
    card_h = per_col * 20 + 12
    svg.append(f'<rect x="{x}" y="{y}" width="{card_w}" height="{card_h}" class="lcrd" rx="5"/>')
    for i, (lbl, cor, dash) in enumerate(itens):
        col = i // per_col
        row = i % per_col
        lx = x + 10 + col * col_w
        ly = y + 18 + row * 20
        d_attr = f' stroke-dasharray="{dash}"' if dash else ""
        svg.append(f'<line x1="{lx}" y1="{ly}" x2="{lx+22}" y2="{ly}" '
                   f'stroke="{cor}" stroke-width="2.5"{d_attr}/>')
        svg.append(f'<circle cx="{lx+11}" cy="{ly}" r="3.5" fill="{cor}" stroke="#fff" stroke-width="1.5"/>')
        svg.append(f'<text x="{lx+28}" y="{ly+4}" class="lbl" fill="#1E293B">{lbl}</text>')

def _line_series(svg, series_list, mx, my, markers=True):
    syms = ["circle","square","triangle","diamond"]
    for idx, (pontos, cor, dash_pat) in enumerate(series_list):
        pts_sorted = sorted(pontos, key=lambda p: p[0])
        coords = [(mx(p[0]), my(p[1])) for p in pts_sorted]
        d = " ".join(f"{'M' if i==0 else 'L'}{cx:.1f},{cy:.1f}"
                     for i,(cx,cy) in enumerate(coords))
        da = f' stroke-dasharray="{dash_pat}"' if dash_pat else ""
        svg.append(f'<path d="{d}" fill="none" stroke="{cor}" stroke-width="2.3"{da} '
                   f'stroke-linejoin="round" stroke-linecap="round"/>')
        if markers:
            s = syms[idx % len(syms)]
            for cx,cy in coords:
                if s == "circle":
                    svg.append(f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="4" '
                               f'fill="{cor}" stroke="#fff" stroke-width="1.5"/>')
                elif s == "square":
                    svg.append(f'<rect x="{cx-3.5:.1f}" y="{cy-3.5:.1f}" width="7" height="7" '
                               f'fill="{cor}" stroke="#fff" stroke-width="1.5" rx="1"/>')
                elif s == "triangle":
                    svg.append(f'<polygon points="{cx:.1f},{cy-5:.1f} '
                               f'{cx-5:.1f},{cy+4:.1f} {cx+5:.1f},{cy+4:.1f}" '
                               f'fill="{cor}" stroke="#fff" stroke-width="1.5"/>')
                elif s == "diamond":
                    svg.append(f'<polygon points="{cx:.1f},{cy-5:.1f} {cx+5:.1f},{cy:.1f} '
                               f'{cx:.1f},{cy+5:.1f} {cx-5:.1f},{cy:.1f}" '
                               f'fill="{cor}" stroke="#fff" stroke-width="1.5"/>')

def _error_band(svg, rows, mx, my, cor, campo="mediaTempo_s", campo_std="desvioPadrao_s"):
    """Banda de desvio-padrão sombreada."""
    rows_s = sorted(rows, key=lambda r: r["tamanhoN"])
    upper = [(r["tamanhoN"], r[campo]+r[campo_std]) for r in rows_s]
    lower = [(r["tamanhoN"], r[campo]-r[campo_std]) for r in rows_s]
    lower_r = list(reversed(lower))
    coords_u = [f"{mx(p[0]):.1f},{my(max(1e-12,p[1])):.1f}" for p in upper]
    coords_l = [f"{mx(p[0]):.1f},{my(max(1e-12,p[1])):.1f}" for p in lower_r]
    poly = " ".join(coords_u + coords_l)
    svg.append(f'<polygon points="{poly}" fill="{cor}" opacity="0.12"/>')

def save(svg, path):
    svg.append('</svg>')
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"  [OK] {os.path.basename(path)}")

# =============================================================================
# FIG-1  Trie vs Patricia — tempo de inserção (prefixos densos e dispersos)
# Insight: Patricia escala muito melhor em prefixos densos; em dispersos a
#          vantagem diminui. Revela o custo real da compactação de prefixos.
# =============================================================================
def fig1_trie_patricia_insercao():
    W,H = 860,520; pL,pR,pT,pB = 90,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    rTD = get("Trie",    "Prefixos_Densos",    "Insercao")
    rPD = get("Patricia","Prefixos_Densos",    "Insercao")
    rTS = get("Trie",    "Prefixos_Dispersos", "Insercao")
    rPS = get("Patricia","Prefixos_Dispersos", "Insercao")

    all_y = [r["mediaTempo_s"] for rows in [rTD,rPD,rTS,rPS] for r in rows]
    min_x,max_x = 100,100000
    min_y,max_y = 0.0, max(all_y)*1.2

    svg = _svg_open(W,H)
    _header(svg,pL,"Escalabilidade de Tempo de Inserção: Trie vs. Árvore Patricia",
            "Prefixos Densos (dicionário real) vs. Prefixos Dispersos (strings aleatórias)","Estruturas de Prefixos")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')
    mx,my = _axes(svg,pL,pT,pW,pH,min_x,max_x,min_y,max_y,True,False,
                  "Tamanho da Entrada N (número de palavras)",
                  "Tempo de Inserção (s)", y_unit=" s")

    series = [
        (pts(rTD), COR["Trie_D"],  None),
        (pts(rPD), COR["Pat_D"],   None),
        (pts(rTS), COR["Trie_Sp"], "6,3"),
        (pts(rPS), COR["Pat_Sp"],  "6,3"),
    ]
    _line_series(svg, series, mx, my)
    _legend_card(svg,[
        ("Trie — Prefixos Densos",     COR["Trie_D"],  None),
        ("Patricia — Prefixos Densos", COR["Pat_D"],   None),
        ("Trie — Prefixos Dispersos",  COR["Trie_Sp"], "6,3"),
        ("Patricia — Prefixos Dispersos",COR["Pat_Sp"],"6,3"),
    ], pL+pW-380, pT+10, cols=2)
    save(svg, os.path.join(FIG,"fig1_trie_patricia_insercao.svg"))

# =============================================================================
# FIG-2  Trie vs Patricia — total de nós alocados (densos e dispersos)
# Insight: A compressão da Patricia reduz ~74% dos nós em densos; em dispersos
#          a diferença cai pois há menos prefixos a compartilhar.
# =============================================================================
def fig2_trie_patricia_nos():
    W,H = 860,520; pL,pR,pT,pB = 95,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    rTD = get("Trie",    "Prefixos_Densos",    "Insercao")
    rPD = get("Patricia","Prefixos_Densos",    "Insercao")
    rTS = get("Trie",    "Prefixos_Dispersos", "Insercao")
    rPS = get("Patricia","Prefixos_Dispersos", "Insercao")

    all_y = [r["contagemNos"] for rows in [rTD,rPD,rTS,rPS] for r in rows]
    min_x,max_x = 100,100000
    min_y,max_y = 0.0, max(all_y)*1.15

    svg = _svg_open(W,H)
    _header(svg,pL,"Total de Nós Alocados: Trie vs. Árvore Patricia",
            "Comprova a compactação de arestas da Patricia e sua dependência da densidade de prefixos","Complexidade Espacial")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')
    mx,my = _axes(svg,pL,pT,pW,pH,min_x,max_x,min_y,max_y,True,False,
                  "Tamanho da Entrada N (número de palavras)","Nós Alocados")

    series = [
        (pts(rTD,"contagemNos"), COR["Trie_D"],  None),
        (pts(rPD,"contagemNos"), COR["Pat_D"],   None),
        (pts(rTS,"contagemNos"), COR["Trie_Sp"], "6,3"),
        (pts(rPS,"contagemNos"), COR["Pat_Sp"],  "6,3"),
    ]
    _line_series(svg, series, mx, my)

    # callout: Patricia densos N=100k
    pat_d_100k = next((r["contagemNos"] for r in rPD if r["tamanhoN"]==100000), 129021)
    px = mx(100000); py = my(pat_d_100k)
    svg.append(f'<rect x="{px-160}" y="{py-19}" width="152" height="17" fill="#EFF6FF" '
               f'stroke="#3B82F6" stroke-width="1" rx="3"/>')
    svg.append(f'<text x="{px-84}" y="{py-7}" text-anchor="middle" class="lbl" fill="#1D4ED8">'
               f'Patricia: 74% menos nós</text>')

    # Legenda no canto superior esquerdo (as curvas crescem para a direita;
    # o canto superior esquerdo da área de plot é garantidamente vazio).
    _legend_card(svg,[
        ("Trie — Prefixos Densos",       COR["Trie_D"],  None),
        ("Patricia — Prefixos Densos",   COR["Pat_D"],   None),
        ("Trie — Prefixos Dispersos",    COR["Trie_Sp"], "6,3"),
        ("Patricia — Prefixos Dispersos",COR["Pat_Sp"],  "6,3"),
    ], pL+10, pT+10, cols=2)
    save(svg, os.path.join(FIG,"fig2_trie_patricia_nos.svg"))

# =============================================================================
# FIG-3  Pior Caso da BST vs AVL, Treap, Splay — chaves ordenadas crescentes
# Insight: BST degenera em O(N²); AVL/Splay mantêm O(N log N). BST não tem
#          dados para N=50k e 100k (timeout/interrompido → plotar somente até 10k).
# =============================================================================
def fig3_pior_caso_ordenado():
    W,H = 860,520; pL,pR,pT,pB = 90,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    rBST   = get("BST",      "Ordenado_Crescente","Insercao")   # só até N=10k
    rAVL   = get("AVLTree",  "Ordenado_Crescente","Insercao")
    rSplay = get("SplayTree","Ordenado_Crescente","Insercao")
    rTreap = get("Treap",    "Ordenado_Crescente","Insercao")

    all_y = [r["mediaTempo_s"] for rows in [rBST,rAVL,rSplay,rTreap] for r in rows]
    min_x,max_x = 100,100000
    min_y,max_y = 1e-7, max(all_y)*3.0

    svg = _svg_open(W,H)
    _header(svg,pL,"Pior Caso com Chaves Ordenadas: BST vs. AVL, Treap e Splay",
            "BST degenera em lista encadeada O(N²); auto-balanceáveis mantêm O(N log N)","Árvores de Busca Binária — Análise de Pior Caso")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')
    mx,my = _axes(svg,pL,pT,pW,pH,min_x,max_x,min_y,max_y,True,True,
                  "Tamanho da Entrada N (chaves 1..N em ordem crescente)",
                  "Tempo de Inserção (s) — escala log₁₀", y_unit=" s")

    # banda de erro para AVL (ilustra estabilidade)
    _error_band(svg, rAVL, mx, my, COR["AVL"])
    series = [
        (pts(rBST),   COR["BST"],   None),
        (pts(rAVL),   COR["AVL"],   None),
        (pts(rTreap), COR["Treap"], None),
        (pts(rSplay), COR["Splay"], None),
    ]
    _line_series(svg, series, mx, my)

    # callout BST em N=10k
    px = mx(10000); py = my(rBST[-1]["mediaTempo_s"])
    svg.append(f'<rect x="{px+8}" y="{py-10}" width="138" height="17" fill="#FEF2F2" '
               f'stroke="#DC2626" stroke-width="1" rx="3"/>')
    svg.append(f'<text x="{px+15}" y="{py+2}" class="lbl" fill="#B91C1C">BST: sem dados p/ N>10k</text>')

    _legend_card(svg,[
        ("BST — Pior Caso O(N²)",  COR["BST"],   None),
        ("Árvore AVL",             COR["AVL"],   None),
        ("Árvore Treap",           COR["Treap"], None),
        ("Árvore Splay",           COR["Splay"], None),
    ], pL+pW-380, pT+10, cols=2)
    save(svg, os.path.join(FIG,"fig3_pior_caso_ordenado.svg"))

# =============================================================================
# FIG-4  AVL vs Splay vs Treap — inserção em 4 distribuições (sem BST,
#         pois a BST não tem dados para N>10k em ordenado)
# Insight: Splay é penalizado em dados quase-ordenados; AVL é mais estável;
#          Treap tem oscilação maior por prioridades aleatórias.
# =============================================================================
def fig4_insercao_4_distribuicoes():
    W,H = 860,520; pL,pR,pT,pB = 90,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    configs = [
        ("Aleatorio_Uniforme",  "Aleatório"),
        ("Quase_Ordenado",      "Quase-Ordenado"),
        ("Ordenado_Crescente",  "Ordenado Cresc."),
        ("Ordenado_Decrescente","Ordenado Decresc."),
    ]
    estruturas = [
        ("AVLTree",  COR["AVL"],   None, "Árvore AVL"),
        ("Treap",    COR["Treap"], "6,3","Árvore Treap"),
        ("SplayTree",COR["Splay"], "3,3","Árvore Splay"),
    ]

    # coletamos para descobrir max_y (usar somente N=100k por estrutura/cenário)
    all_y = []
    for est,_,_,_ in estruturas:
        for cen,_ in configs:
            rows = get(est,cen,"Insercao")
            all_y += [r["mediaTempo_s"] for r in rows]

    min_x,max_x = 100,100000
    min_y,max_y = 0.0, max(all_y)*1.2

    svg = _svg_open(W,H)
    _header(svg,pL,"Tempo de Inserção em Quatro Distribuições de Dados",
            "AVL, Treap e Splay sob dados aleatórios, quase-ordenados e ordenados (crescente e decrescente)",
            "Árvores de Busca Binária — Análise Multi-Distribuição")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')
    mx,my = _axes(svg,pL,pT,pW,pH,min_x,max_x,min_y,max_y,True,False,
                  "Tamanho da Entrada N","Tempo de Inserção (s)", y_unit=" s")

    dashes_cen = [None,"6,3","3,3","9,3"]
    for idx_e,(est,cor,_,nome_e) in enumerate(estruturas):
        for idx_c,(cen,nome_c) in enumerate(configs):
            rows = get(est,cen,"Insercao")
            dash = dashes_cen[idx_c]
            _line_series(svg, [(pts(rows), cor, dash)], mx, my, markers=(idx_c==0))

    # -----------------------------------------------------------------------
    # Legenda em dois cartões empilhados no canto superior esquerdo do plot.
    # Todas as curvas partem de valores próximos de zero para N=100..1000,
    # portanto o topo-esquerdo é garantidamente livre de dados.
    # -----------------------------------------------------------------------
    # Cartão 1: Cor = Estrutura
    _legend_card(svg,[
        ("Árvore AVL",   COR["AVL"],   None),
        ("Árvore Treap", COR["Treap"], None),
        ("Árvore Splay", COR["Splay"], None),
    ], pL+10, pT+10)

    # Cartão 2: Traçado = Distribuição (abaixo do cartão 1)
    # alt card_h = 4*20+12 = 92; card1_h = 3*20+12 = 72
    card1_h = 3*20+12
    x2 = pL+10; y2 = pT+10+card1_h+6
    card2_h = 4*20+12
    svg.append(f'<rect x="{x2}" y="{y2}" width="185" height="{card2_h}" class="lcrd" rx="5"/>')
    svg.append(f'<text x="{x2+10}" y="{y2+14}" style="font-size:9px;font-weight:700;fill:#64748B;'
               f'font-family:Segoe UI,sans-serif">Traçado = Distribuição</text>')
    for i,(nome_c,dash) in enumerate(zip(
            ["Aleatório","Quase-Ordenado","Ordenado Cresc.","Ordenado Decresc."],
            dashes_cen)):
        da = f' stroke-dasharray="{dash}"' if dash else ""
        lx = x2+10; ly = y2+26+i*20
        svg.append(f'<line x1="{lx}" y1="{ly}" x2="{lx+20}" y2="{ly}" stroke="#475569" stroke-width="2"{da}/>')
        svg.append(f'<text x="{lx+26}" y="{ly+4}" class="lbl" fill="#334155">{nome_c}</text>')

    save(svg, os.path.join(FIG,"fig4_insercao_4_distribuicoes.svg"))

# =============================================================================
# FIG-5  Localidade Temporal — Splay vs AVL vs Treap (Zipf 80-20 e 90-10)
# Insight: A Splay tree explora localidade; quanto mais concentrado o acesso
#          (90-10 > 80-20), maior sua vantagem por autoajuste.
# =============================================================================
def fig5_localidade_zipf():
    W,H = 720,440; pL,pR,pT,pB = 85,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    def t_ms(est, cen):
        rows = get(est, cen, "Busca_100k")
        # Converter para milissegundos para melhor resolução visual
        return rows[0]["mediaTempo_s"] * 1000.0 if rows else None

    dados = {
        "Splay": [t_ms("SplayTree","Zipf_80_20"), t_ms("SplayTree","Zipf_90_10")],
        "AVL":   [t_ms("AVLTree",  "Zipf_80_20"), t_ms("AVLTree",  "Zipf_90_10")],
        "Treap": [t_ms("Treap",    "Zipf_80_20"), t_ms("Treap",    "Zipf_90_10")],
    }
    cenarios = ["Zipf 80-20", "Zipf 90-10"]
    estruturas_ord = ["Splay","AVL","Treap"]
    cores_bar = [COR["Splay"],COR["AVL"],COR["Treap"]]

    all_vals = [v for vals in dados.values() for v in vals if v is not None]
    max_y = max(all_vals) * 1.3
    min_y = 0.0

    svg = _svg_open(W,H)
    _header(svg,pL,"Localidade Temporal: 100 mil Buscas sob Distribuição de Zipf",
            "Splay autoajusta raiz para chaves quentes; AVL e Treap têm custo fixo O(log N)",
            "Localidade Temporal de Referência")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')

    def my(v):
        return pT + pH - (v - min_y)/(max_y-min_y)*pH

    # Grade Y em milissegundos
    for i in range(6):
        ty = max_y * i / 5
        yy = my(ty)
        svg.append(f'<line x1="{pL}" y1="{yy:.1f}" x2="{pL+pW}" y2="{yy:.1f}" class="grid"/>')
        svg.append(f'<text x="{pL-7}" y="{yy+3:.1f}" text-anchor="end" class="tck">{ty:.1f} ms</text>')

    svg.append(f'<line x1="{pL}" y1="{pT+pH}" x2="{pL+pW}" y2="{pT+pH}" class="ax"/>')
    svg.append(f'<line x1="{pL}" y1="{pT}"    x2="{pL}"    y2="{pT+pH}" class="ax"/>')

    nE = len(estruturas_ord)
    nC = len(cenarios)
    gw = pW / nC
    bw = gw * 0.6 / nE

    for ic, cen in enumerate(cenarios):
        cx = pL + (ic + 0.5) * gw
        svg.append(f'<text x="{cx:.1f}" y="{pT+pH+18}" text-anchor="middle" class="tck">{cen}</text>')
        for ie, (est, cor) in enumerate(zip(estruturas_ord, cores_bar)):
            val = dados[est][ic]
            if val is None: continue
            bh = (val/max_y)*pH
            bx = cx - nE*bw/2 + ie*bw
            by = my(val)
            svg.append(f'<rect x="{bx:.1f}" y="{by:.1f}" width="{bw-2:.1f}" height="{bh:.1f}" '
                       f'fill="{cor}" rx="3"/>')
            # Exibir em ms com resolução adequada
            lbl = f"{val:.2f} ms" if val >= 0.1 else f"{val:.3f} ms"
            svg.append(f'<text x="{bx+bw/2-1:.1f}" y="{by-6:.1f}" text-anchor="middle" '
                       f'class="lbl" fill="{cor}">{lbl}</text>')

    svg.append(f'<text x="{pL+pW/2:.1f}" y="{pT+pH+42}" text-anchor="middle" class="axl">'
               f'Padrão de Distribuição de Acessos (N = 50.000 chaves inseridas)</text>')
    svg.append(f'<text x="22" y="{pT+pH/2:.1f}" text-anchor="middle" '
               f'transform="rotate(-90 22 {pT+pH/2:.1f})" class="axl">Tempo Total de 100k Buscas (ms)</text>')

    _legend_card(svg,[
        ("Árvore Splay (Autoajuste Dinâmico)", COR["Splay"], None),
        ("Árvore AVL  (Balanceamento Fixo)",   COR["AVL"],   None),
        ("Árvore Treap (Probabilística)",       COR["Treap"], None),
    ], pL+pW-215, pT+12)
    save(svg, os.path.join(FIG,"fig5_localidade_zipf.svg"))

# =============================================================================
# FIG-6  Rotações acumuladas na inserção — AVL vs Splay vs Treap (aleatório)
# Insight: AVL realiza ~0.68 rot/inserção (constante); Splay e Treap crescem
#          mais por terem que restruturar caminhos inteiros por operação.
# =============================================================================
def fig6_rotacoes_acumuladas():
    W,H = 860,520; pL,pR,pT,pB = 95,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    rAVL   = get("AVLTree",  "Aleatorio_Uniforme","Insercao")
    rTreap = get("Treap",    "Aleatorio_Uniforme","Insercao")
    rSplay = get("SplayTree","Aleatorio_Uniforme","Insercao")

    all_y = [r["mediaOpInternas"] for rows in [rAVL,rTreap,rSplay] for r in rows]
    min_x,max_x = 100,100000
    min_y,max_y = 0.0, max(all_y)*1.2

    svg = _svg_open(W,H)
    _header(svg,pL,"Rotações Acumuladas na Inserção (Caso Médio — Dados Aleatórios)",
            "AVL executa ≤ 2 rotações por inserção; Splay e Treap reorganizam caminhos inteiros",
            "Custo de Reorganização Estrutural")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')
    mx,my = _axes(svg,pL,pT,pW,pH,min_x,max_x,min_y,max_y,True,False,
                  "Tamanho da Entrada N","Rotações Acumuladas")

    series = [
        (pts(rAVL,  "mediaOpInternas"), COR["AVL"],   None),
        (pts(rTreap,"mediaOpInternas"), COR["Treap"], None),
        (pts(rSplay,"mediaOpInternas"), COR["Splay"], None),
    ]
    _line_series(svg, series, mx, my)
    _legend_card(svg,[
        ("Árvore AVL   (≤2 rot. / inserção)",  COR["AVL"],   None),
        ("Árvore Treap (heap ascendente)",      COR["Treap"], None),
        ("Árvore Splay (rotações no caminho)",  COR["Splay"], None),
    ], pL+pW-365, pT+10)
    save(svg, os.path.join(FIG,"fig6_rotacoes_acumuladas.svg"))

# =============================================================================
# FIG-7  KD-Tree — tempo de 1-NN e Range Query por N (uniforme vs clusters)
# Insight: Clusters degradam a poda espacial porque os pontos concentrados
#          confundem a separação de hiperplanos — Range Query cresce mais rápido.
#          Plotamos o tempo médio (s) vs N para 2000 consultas de cada tipo.
# =============================================================================
def fig7_kdtree_consultas():
    W,H = 860,520; pL,pR,pT,pB = 90,30,82,68
    pW,pH = W-pL-pR, H-pT-pB

    rNN_U  = get("KDTree_2D","Uniformes_2D","Nearest_Neighbor_2kx")
    rNN_C  = get("KDTree_2D","Clusters_2D", "Nearest_Neighbor_2kx")
    rRQ_U  = get("KDTree_2D","Uniformes_2D","Range_Search_2kx")
    rRQ_C  = get("KDTree_2D","Clusters_2D", "Range_Search_2kx")

    all_y = [r["mediaTempo_s"] for rows in [rNN_U,rNN_C,rRQ_U,rRQ_C] for r in rows]
    min_x,max_x = 100,100000
    min_y,max_y = 0.0, max(all_y)*1.18

    svg = _svg_open(W,H)
    _header(svg,pL,"KD-Tree: Tempo de 2.000 Consultas 1-NN e Range Query",
            "Dados uniformes vs. clusters gaussianos — clusters degradam a poda de hiperplanos",
            "Estruturas Espaciais Multidimensionais (KD-Tree 2D)")
    svg.append(f'<rect x="{pL}" y="{pT}" width="{pW}" height="{pH}" fill="#F8FAFC" rx="3"/>')
    mx,my = _axes(svg,pL,pT,pW,pH,min_x,max_x,min_y,max_y,True,False,
                  "Número de Pontos na Árvore (N)","Tempo de 2.000 Consultas (s)", y_unit=" s")

    series = [
        (pts(rNN_U), COR["KD_U"],  None),
        (pts(rNN_C), COR["KD_C"],  None),
        (pts(rRQ_U), COR["KD_U"],  "6,3"),
        (pts(rRQ_C), COR["KD_C"],  "6,3"),
    ]
    _line_series(svg, series, mx, my)
    _legend_card(svg,[
        ("1-NN — Dados Uniformes",       COR["KD_U"], None),
        ("1-NN — Clusters Gaussianos",   COR["KD_C"], None),
        ("Range Query — Dados Uniformes",COR["KD_U"], "6,3"),
        ("Range Query — Clusters",       COR["KD_C"], "6,3"),
    ], pL+pW-380, pT+10, cols=2)
    save(svg, os.path.join(FIG,"fig7_kdtree_consultas.svg"))

# =============================================================================
if __name__ == "__main__":
    print("="*65)
    print(" GERANDO SUÍTE FINAL DE FIGURAS CIENTÍFICAS VETORIAIS (SVG)")
    print("="*65)
    fig1_trie_patricia_insercao()
    fig2_trie_patricia_nos()
    fig3_pior_caso_ordenado()
    fig4_insercao_4_distribuicoes()
    # fig5_localidade_zipf() REMOVIDA: medição inválida por eliminação de código morto (-O3)
    fig6_rotacoes_acumuladas()
    fig7_kdtree_consultas()
    print("="*65)
    print(f"  6 figuras em: relatorio/figuras/")
    print("="*65)
