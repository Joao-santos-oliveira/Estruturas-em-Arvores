/**
 * Implementação da Árvore Patricia (Radix Tree compacta) para visualização web
 */
class NoPatricia {
    constructor(prefixo = '', id = 0) {
        this.id = id;
        this.prefixo = prefixo;
        this.fimPalavra = false;
        this.filhos = new Map(); // primeiro_char -> NoPatricia
    }
}

class PatriciaVisual {
    constructor() {
        this.proximoId = 1;
        this.raiz = new NoPatricia('', this.proximoId++);
        this.totalPalavras = 0;
        this.totalNos = 1;
        this.totalSplits = 0;
        this.totalMerges = 0;
        this.historicoOperacoes = [];
    }

    reset() {
        this.proximoId = 1;
        this.raiz = new NoPatricia('', this.proximoId++);
        this.totalPalavras = 0;
        this.totalNos = 1;
        this.totalSplits = 0;
        this.totalMerges = 0;
        this.historicoOperacoes = [];
    }

    _prefixoComum(s1, s2) {
        let i = 0;
        while (i < s1.length && i < s2.length && s1[i] === s2[i]) i++;
        return i;
    }

    insert(palavra) {
        if (!palavra || palavra.trim() === '') return { sucesso: false, passos: [] };
        palavra = palavra.trim().toLowerCase();

        let passos = [];
        let splitOcorreu = false;

        const inserirRecursivo = (no, sufixo) => {
            if (sufixo.length === 0) {
                let jaEra = no.fimPalavra;
                no.fimPalavra = true;
                if (!jaEra) this.totalPalavras++;
                passos.push({ tipo: 'marcar_fim', noId: no.id });
                return;
            }

            let primeiroChar = sufixo[0];
            if (!no.filhos.has(primeiroChar)) {
                // Caso 1: Criação direta de aresta compacta com toda a string restante
                let novo = new NoPatricia(sufixo, this.proximoId++);
                novo.fimPalavra = true;
                no.filhos.set(primeiroChar, novo);
                this.totalNos++;
                this.totalPalavras++;
                passos.push({ tipo: 'nova_aresta', noId: novo.id, prefixo: sufixo, paiId: no.id });
                return;
            }

            let filho = no.filhos.get(primeiroChar);
            let lenComum = this._prefixoComum(filho.prefixo, sufixo);

            if (lenComum === filho.prefixo.length) {
                // Caso 2: Prefixo do filho está contido no sufixo a inserir
                passos.push({ tipo: 'percorrer_aresta', noId: filho.id, prefixo: filho.prefixo });
                inserirRecursivo(filho, sufixo.substring(lenComum));
            } else {
                // Caso 3: Split da aresta compacta existente
                splitOcorreu = true;
                this.totalSplits++;

                let prefixoComum = filho.prefixo.substring(0, lenComum);
                let restoFilho = filho.prefixo.substring(lenComum);
                let restoSufixo = sufixo.substring(lenComum);

                // Nó intermediário do split
                let noIntermediario = new NoPatricia(prefixoComum, this.proximoId++);
                this.totalNos++;

                // Ajusta o filho existente
                filho.prefixo = restoFilho;
                noIntermediario.filhos.set(restoFilho[0], filho);

                if (restoSufixo.length === 0) {
                    noIntermediario.fimPalavra = true;
                    this.totalPalavras++;
                } else {
                    let novoNo = new NoPatricia(restoSufixo, this.proximoId++);
                    novoNo.fimPalavra = true;
                    noIntermediario.filhos.set(restoSufixo[0], novoNo);
                    this.totalNos++;
                    this.totalPalavras++;
                }

                no.filhos.set(primeiroChar, noIntermediario);
                passos.push({
                    tipo: 'split_aresta',
                    noSplitId: noIntermediario.id,
                    prefixoComum,
                    restoFilho,
                    restoSufixo
                });
            }
        };

        inserirRecursivo(this.raiz, palavra);
        let resultado = {
            sucesso: true,
            palavra,
            splitOcorreu,
            totalNos: this.totalNos,
            totalPalavras: this.totalPalavras,
            totalSplits: this.totalSplits,
            passos
        };
        this.historicoOperacoes.push({ op: 'inserir', ...resultado });
        return resultado;
    }

    search(palavra) {
        if (!palavra) return { encontrada: false, passos: [] };
        palavra = palavra.trim().toLowerCase();

        let atual = this.raiz;
        let sufixo = palavra;
        let passos = [{ noId: atual.id, status: 'raiz' }];
        let nosVisitados = [atual.id];

        while (sufixo.length > 0) {
            let primeiroChar = sufixo[0];
            if (!atual.filhos.has(primeiroChar)) {
                return { encontrada: false, passos, nosVisitados };
            }

            let filho = atual.filhos.get(primeiroChar);
            let lenComum = this._prefixoComum(filho.prefixo, sufixo);

            if (lenComum !== filho.prefixo.length) {
                // Não correspondeu ao rótulo inteiro da aresta
                return { encontrada: false, passos, nosVisitados };
            }

            passos.push({ noId: filho.id, prefixo: filho.prefixo });
            nosVisitados.push(filho.id);
            sufixo = sufixo.substring(lenComum);
            atual = filho;
        }

        return {
            encontrada: atual.fimPalavra,
            passos,
            nosVisitados,
            noFinalId: atual.id
        };
    }

    remove(palavra) {
        if (!palavra) return { sucesso: false };
        palavra = palavra.trim().toLowerCase();

        let mergeOcorreu = false;

        const removerRecursivo = (no, sufixo) => {
            if (sufixo.length === 0) {
                if (!no.fimPalavra) return false;
                no.fimPalavra = false;
                this.totalPalavras--;

                // Se virou folha sem filhos e não é raiz, sinaliza poda
                if (no.filhos.size === 0 && no !== this.raiz) return true;

                // Se ficou com 1 único filho e não é fim de palavra, faz merge
                if (no.filhos.size === 1 && !no.fimPalavra && no !== this.raiz) {
                    let chaveUnica = Array.from(no.filhos.keys())[0];
                    let unicoFilho = no.filhos.get(chaveUnica);
                    no.prefixo += unicoFilho.prefixo;
                    no.fimPalavra = unicoFilho.fimPalavra;
                    no.filhos = unicoFilho.filhos;
                    this.totalNos--;
                    this.totalMerges++;
                    mergeOcorreu = true;
                }
                return false;
            }

            let primeiroChar = sufixo[0];
            if (!no.filhos.has(primeiroChar)) return false;

            let filho = no.filhos.get(primeiroChar);
            let lenComum = this._prefixoComum(filho.prefixo, sufixo);

            if (lenComum !== filho.prefixo.length) return false;

            let devePodarFilho = removerRecursivo(filho, sufixo.substring(lenComum));

            if (devePodarFilho) {
                no.filhos.delete(primeiroChar);
                this.totalNos--;

                // Verifica se o nó 'no' agora deve sofrer fusão (merge) com seu único filho restante
                if (no.filhos.size === 1 && !no.fimPalavra && no !== this.raiz) {
                    let chaveRestante = Array.from(no.filhos.keys())[0];
                    let filhoRestante = no.filhos.get(chaveRestante);
                    no.prefixo += filhoRestante.prefixo;
                    no.fimPalavra = filhoRestante.fimPalavra;
                    no.filhos = filhoRestante.filhos;
                    this.totalNos--;
                    this.totalMerges++;
                    mergeOcorreu = true;
                }
            }

            return false;
        };

        removerRecursivo(this.raiz, palavra);
        return {
            sucesso: true,
            palavra,
            mergeOcorreu,
            totalNos: this.totalNos,
            totalPalavras: this.totalPalavras,
            totalMerges: this.totalMerges
        };
    }

    toHierarchy() {
        if (this.totalPalavras === 0 && this.raiz.filhos.size === 0) return null;
        const converter = (no, textoAresta = '') => {
            let chaves = Array.from(no.filhos.keys()).sort();
            return {
                id: no.id,
                label: no === this.raiz ? 'Raiz (λ)' : `"${no.prefixo}"`,
                edgeLabel: no.prefixo,
                prefixo: no.prefixo,
                isEnd: no.fimPalavra,
                isRoot: no === this.raiz,
                children: chaves.map(k => converter(no.filhos.get(k), no.filhos.get(k).prefixo))
            };
        };
        return converter(this.raiz);
    }
}
