/**
 * Implementação da KD-Tree 2D com Nearest Neighbor e Range Search
 */
class NoKD {
    constructor(x, y, eixo, id = 0) {
        this.id = id;
        this.ponto = { x: Number(x), y: Number(y) };
        this.eixo = eixo; // 0 para corte em X, 1 para corte em Y
        this.esq = null;
        this.dir = null;
    }
}

class KDTreeVisual {
    constructor() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalPontos = 0;
        this.distanciasCalculadas = 0;
        this.podasRealizadas = 0;
    }

    reset() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalPontos = 0;
        this.distanciasCalculadas = 0;
        this.podasRealizadas = 0;
    }

    insert(x, y) {
        x = Number(x);
        y = Number(y);
        if (isNaN(x) || isNaN(y)) return { sucesso: false };

        const inserirRec = (no, profundidade) => {
            if (!no) {
                this.totalPontos++;
                return new NoKD(x, y, profundidade % 2, this.proximoId++);
            }

            let eixo = no.eixo;
            let valNovo = (eixo === 0) ? x : y;
            let valAtual = (eixo === 0) ? no.ponto.x : no.ponto.y;

            if (valNovo < valAtual) {
                no.esq = inserirRec(no.esq, profundidade + 1);
            } else {
                no.dir = inserirRec(no.dir, profundidade + 1);
            }
            return no;
        };

        this.raiz = inserirRec(this.raiz, 0);
        return {
            sucesso: true,
            ponto: { x, y },
            totalPontos: this.totalPontos
        };
    }

    remove(x, y) {
        x = Number(x);
        y = Number(y);
        let removido = false;

        const encontrarMinimo = (no, eixoCorte, profundidade) => {
            if (!no) return null;
            let eixoAtual = profundidade % 2;
            
            if (eixoAtual === eixoCorte) {
                if (!no.esq) return no;
                return encontrarMinimo(no.esq, eixoCorte, profundidade + 1);
            }

            let esqMin = encontrarMinimo(no.esq, eixoCorte, profundidade + 1);
            let dirMin = encontrarMinimo(no.dir, eixoCorte, profundidade + 1);
            let res = no;

            if (esqMin && esqMin.ponto[eixoCorte === 0 ? 'x' : 'y'] < res.ponto[eixoCorte === 0 ? 'x' : 'y']) res = esqMin;
            if (dirMin && dirMin.ponto[eixoCorte === 0 ? 'x' : 'y'] < res.ponto[eixoCorte === 0 ? 'x' : 'y']) res = dirMin;

            return res;
        };

        const removerRec = (no, px, py, profundidade) => {
            if (!no) return null;

            let eixo = profundidade % 2;

            if (no.ponto.x === px && no.ponto.y === py) {
                if (no.dir) {
                    let minNo = encontrarMinimo(no.dir, eixo, profundidade + 1);
                    no.ponto = { x: minNo.ponto.x, y: minNo.ponto.y };
                    no.dir = removerRec(no.dir, minNo.ponto.x, minNo.ponto.y, profundidade + 1);
                    removido = true;
                } else if (no.esq) {
                    let minNo = encontrarMinimo(no.esq, eixo, profundidade + 1);
                    no.ponto = { x: minNo.ponto.x, y: minNo.ponto.y };
                    no.dir = removerRec(no.esq, minNo.ponto.x, minNo.ponto.y, profundidade + 1);
                    no.esq = null;
                    removido = true;
                } else {
                    removido = true;
                    return null;
                }
                return no;
            }

            let ptVal = eixo === 0 ? px : py;
            let noVal = eixo === 0 ? no.ponto.x : no.ponto.y;

            if (ptVal < noVal) {
                no.esq = removerRec(no.esq, px, py, profundidade + 1);
            } else {
                no.dir = removerRec(no.dir, px, py, profundidade + 1);
            }
            return no;
        };

        this.raiz = removerRec(this.raiz, x, y, 0);
        if (removido) this.totalPontos--;
        return { sucesso: removido };
    }

    _distanciaQuadrada(p1, p2) {
        let dx = p1.x - p2.x;
        let dy = p1.y - p2.y;
        this.distanciasCalculadas++;
        return dx * dx + dy * dy;
    }

    nearestNeighbor(alvoX, alvoY) {
        let alvo = { x: Number(alvoX), y: Number(alvoY) };
        if (!this.raiz) return { encontrado: null, passos: [] };

        this.distanciasCalculadas = 0;
        let melhorNo = null;
        let melhorDist = Infinity;
        let nosVisitados = [];
        let podas = [];

        const buscarNN = (no) => {
            if (!no) return;

            nosVisitados.push(no.id);
            let d = this._distanciaQuadrada(no.ponto, alvo);
            if (d < melhorDist) {
                melhorDist = d;
                melhorNo = no;
            }

            let eixo = no.eixo;
            let valAlvo = (eixo === 0) ? alvo.x : alvo.y;
            let valNo = (eixo === 0) ? no.ponto.x : no.ponto.y;

            let proximoLado = (valAlvo < valNo) ? no.esq : no.dir;
            let outroLado = (valAlvo < valNo) ? no.dir : no.esq;

            // Explora o lado mais promissor primeiro
            buscarNN(proximoLado);

            // Verifica se a hiperesfera cruza o plano de corte (hiperplano)
            let diffPlano = valAlvo - valNo;
            if (diffPlano * diffPlano < melhorDist) {
                buscarNN(outroLado);
            } else {
                this.podasRealizadas++;
                if (outroLado) podas.push(outroLado.id);
            }
        };

        buscarNN(this.raiz);
        return {
            alvo,
            vizinhoMaisProximo: melhorNo ? melhorNo.ponto : null,
            distanciaEuclidiana: Math.sqrt(melhorDist),
            distanciasCalculadas: this.distanciasCalculadas,
            nosVisitados,
            podas
        };
    }

    rangeSearch(minX, minY, maxX, maxY) {
        let caixa = {
            minX: Math.min(Number(minX), Number(maxX)),
            minY: Math.min(Number(minY), Number(maxY)),
            maxX: Math.max(Number(minX), Number(maxX)),
            maxY: Math.max(Number(minY), Number(maxY))
        };

        let resultados = [];
        let nosVisitados = [];

        const buscarRange = (no) => {
            if (!no) return;
            nosVisitados.push(no.id);

            let pt = no.ponto;
            if (pt.x >= caixa.minX && pt.x <= caixa.maxX &&
                pt.y >= caixa.minY && pt.y <= caixa.maxY) {
                resultados.push(pt);
            }

            let eixo = no.eixo;
            if (eixo === 0) {
                if (caixa.minX <= pt.x) buscarRange(no.esq);
                if (caixa.maxX >= pt.x) buscarRange(no.dir);
            } else {
                if (caixa.minY <= pt.y) buscarRange(no.esq);
                if (caixa.maxY >= pt.y) buscarRange(no.dir);
            }
        };

        buscarRange(this.raiz);
        return {
            caixa,
            pontosEncontrados: resultados,
            totalEncontrados: resultados.length,
            nosVisitados
        };
    }

    // Coleta as linhas de corte para renderização no plano cartesiano 2D
    coletarLinhasDeCorte(box = { minX: 0, maxX: 100, minY: 0, maxY: 100 }) {
        let linhas = [];
        let pontos = [];

        const percorrer = (no, b) => {
            if (!no) return;
            pontos.push({ id: no.id, ...no.ponto, eixo: no.eixo });

            if (no.eixo === 0) {
                // Corte vertical (X = c)
                linhas.push({
                    id: no.id,
                    eixo: 'X',
                    x1: no.ponto.x,
                    y1: b.minY,
                    x2: no.ponto.x,
                    y2: b.maxY,
                    ponto: no.ponto
                });
                percorrer(no.esq, { ...b, maxX: no.ponto.x });
                percorrer(no.dir, { ...b, minX: no.ponto.x });
            } else {
                // Corte horizontal (Y = c)
                linhas.push({
                    id: no.id,
                    eixo: 'Y',
                    x1: b.minX,
                    y1: no.ponto.y,
                    x2: b.maxX,
                    y2: no.ponto.y,
                    ponto: no.ponto
                });
                percorrer(no.esq, { ...b, maxY: no.ponto.y });
                percorrer(no.dir, { ...b, minY: no.ponto.y });
            }
        };

        percorrer(this.raiz, box);
        return { linhas, pontos };
    }

    toHierarchy() {
        const converter = (no) => {
            if (!no) return null;
            let children = [];
            if (no.esq) children.push(converter(no.esq));
            if (no.dir) children.push(converter(no.dir));
            let eixoStr = (no.eixo === 0) ? 'X' : 'Y';
            return {
                id: no.id,
                label: `(${no.ponto.x}, ${no.ponto.y}) [${eixoStr}]`,
                eixo: eixoStr,
                ponto: no.ponto,
                isRoot: no === this.raiz,
                children: children.filter(Boolean)
            };
        };
        return converter(this.raiz);
    }
}
