/**
 * Implementação da Árvore Trie (Prefix Tree) para visualização web
 */
class NoTrie {
    constructor(caractere = '', id = 0) {
        this.id = id;
        this.caractere = caractere;
        this.fimPalavra = false;
        this.filhos = new Map(); // char -> NoTrie
    }
}

class TrieVisual {
    constructor() {
        this.proximoId = 1;
        this.raiz = new NoTrie('', this.proximoId++);
        this.totalPalavras = 0;
        this.totalNos = 1;
        this.historicoOperacoes = [];
    }

    reset() {
        this.proximoId = 1;
        this.raiz = new NoTrie('', this.proximoId++);
        this.totalPalavras = 0;
        this.totalNos = 1;
        this.historicoOperacoes = [];
    }

    insert(palavra) {
        if (!palavra || palavra.trim() === '') return { sucesso: false, passos: [] };
        palavra = palavra.trim().toLowerCase();
        
        let atual = this.raiz;
        let passos = [];
        let novosNos = 0;

        for (let i = 0; i < palavra.length; ++i) {
            let c = palavra[i];
            let existe = atual.filhos.has(c);
            
            if (!existe) {
                let novo = new NoTrie(c, this.proximoId++);
                atual.filhos.set(c, novo);
                this.totalNos++;
                novosNos++;
                passos.push({ tipo: 'criar_no', char: c, noId: novo.id, profundidade: i + 1 });
            } else {
                passos.push({ tipo: 'percorrer', char: c, noId: atual.filhos.get(c).id, profundidade: i + 1 });
            }
            atual = atual.filhos.get(c);
        }

        let jaExistia = atual.fimPalavra;
        atual.fimPalavra = true;
        if (!jaExistia) this.totalPalavras++;

        let resultado = {
            sucesso: true,
            palavra,
            jaExistia,
            novosNos,
            passos,
            totalNos: this.totalNos,
            totalPalavras: this.totalPalavras
        };
        this.historicoOperacoes.push({ op: 'inserir', ...resultado });
        return resultado;
    }

    search(palavra) {
        if (!palavra) return { encontrada: false, passos: [] };
        palavra = palavra.trim().toLowerCase();
        
        let atual = this.raiz;
        let passos = [{ noId: atual.id, status: 'raiz' }];
        let nosVisitados = [atual.id];

        for (let i = 0; i < palavra.length; ++i) {
            let c = palavra[i];
            if (!atual.filhos.has(c)) {
                return { encontrada: false, passos, nosVisitados, falhaNoIndice: i };
            }
            atual = atual.filhos.get(c);
            passos.push({ noId: atual.id, char: c, fim: atual.fimPalavra });
            nosVisitados.push(atual.id);
        }

        return {
            encontrada: atual.fimPalavra,
            passos,
            nosVisitados,
            noFinalId: atual.id
        };
    }

    startsWith(prefixo) {
        if (!prefixo) return { existe: false, palavras: [] };
        prefixo = prefixo.trim().toLowerCase();
        
        let atual = this.raiz;
        for (let c of prefixo) {
            if (!atual.filhos.has(c)) return { existe: false, palavras: [] };
            atual = atual.filhos.get(c);
        }

        let palavras = [];
        this._coletarPalavras(atual, prefixo, palavras);
        return { existe: true, palavras, total: palavras.length };
    }

    _coletarPalavras(no, prefixoAtual, lista) {
        if (no.fimPalavra) lista.push(prefixoAtual);
        // Ordenar filhos para visualização determinística
        let chaves = Array.from(no.filhos.keys()).sort();
        for (let c of chaves) {
            this._coletarPalavras(no.filhos.get(c), prefixoAtual + c, lista);
        }
    }

    remove(palavra) {
        if (!palavra) return { sucesso: false };
        palavra = palavra.trim().toLowerCase();
        
        let nosRemovidos = 0;
        let resultado = { sucesso: false, nosRemovidos: 0 };

        const removerRecursivo = (no, i) => {
            if (i === palavra.length) {
                if (!no.fimPalavra) return false;
                no.fimPalavra = false;
                this.totalPalavras--;
                resultado.sucesso = true;
                return no.filhos.size === 0;
            }

            let c = palavra[i];
            if (!no.filhos.has(c)) return false;

            let filho = no.filhos.get(c);
            let devePodarFilho = removerRecursivo(filho, i + 1);

            if (devePodarFilho) {
                no.filhos.delete(c);
                this.totalNos--;
                nosRemovidos++;
                return !no.fimPalavra && no.filhos.size === 0 && no !== this.raiz;
            }
            return false;
        };

        removerRecursivo(this.raiz, 0);
        resultado.nosRemovidos = nosRemovidos;
        resultado.totalNos = this.totalNos;
        resultado.totalPalavras = this.totalPalavras;
        return resultado;
    }

    // Exporta representação em árvore com coordenadas para desenho
    toHierarchy() {
        if (this.totalPalavras === 0 && this.raiz.filhos.size === 0) return null;
        const converter = (no, textoAresta = '') => {
            let chaves = Array.from(no.filhos.keys()).sort();
            return {
                id: no.id,
                label: no.caractere === '' ? 'Raiz (λ)' : no.caractere,
                edgeLabel: textoAresta,
                isEnd: no.fimPalavra,
                isRoot: no === this.raiz,
                children: chaves.map(k => converter(no.filhos.get(k), k))
            };
        };
        return converter(this.raiz);
    }
}
