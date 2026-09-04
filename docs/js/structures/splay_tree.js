/**
 * Implementação da Árvore Splay com registro dos passos Zig, Zig-Zig e Zig-Zag
 */
class NoSplay {
    constructor(chave, id = 0) {
        this.id = id;
        this.chave = chave;
        this.esq = null;
        this.dir = null;
    }
}

class SplayTreeVisual {
    constructor() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalNos = 0;
        this.totalRotacoes = 0;
        this.historicoPassosSplay = [];
    }

    reset() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalNos = 0;
        this.totalRotacoes = 0;
        this.historicoPassosSplay = [];
    }

    _rotacaoDireita(x) {
        let y = x.esq;
        x.esq = y.dir;
        y.dir = x;
        this.totalRotacoes++;
        return y;
    }

    _rotacaoEsquerda(x) {
        let y = x.dir;
        x.dir = y.esq;
        y.esq = x;
        this.totalRotacoes++;
        return y;
    }

    // Executa a operação fundamental Splay trazendo a chave (ou mais próxima) para a raiz
    splay(raiz, chave, passos = []) {
        if (!raiz || raiz.chave === chave) return raiz;

        // Chave está na subárvore esquerda
        if (chave < raiz.chave) {
            if (!raiz.esq) return raiz;

            // Caso 1: Zig-Zig (Esq-Esq)
            if (chave < raiz.esq.chave) {
                raiz.esq.esq = this.splay(raiz.esq.esq, chave, passos);
                raiz = this._rotacaoDireita(raiz);
                if (passos) passos.push({ tipo: 'Zig-Zig (Direita-Direita)', chave });
            }
            // Caso 2: Zig-Zag (Esq-Dir)
            else if (chave > raiz.esq.chave) {
                raiz.esq.dir = this.splay(raiz.esq.dir, chave, passos);
                if (raiz.esq.dir) {
                    raiz.esq = this._rotacaoEsquerda(raiz.esq);
                    if (passos) passos.push({ tipo: 'Zig-Zag (Esquerda)', chave });
                }
            }

            if (!raiz.esq) return raiz;
            let resultado = this._rotacaoDireita(raiz);
            if (passos) passos.push({ tipo: 'Zig (Direita)', chave });
            return resultado;
        }
        // Chave está na subárvore direita
        else {
            if (!raiz.dir) return raiz;

            // Caso 3: Zig-Zag (Dir-Esq)
            if (chave < raiz.dir.chave) {
                raiz.dir.esq = this.splay(raiz.dir.esq, chave, passos);
                if (raiz.dir.esq) {
                    raiz.dir = this._rotacaoDireita(raiz.dir);
                    if (passos) passos.push({ tipo: 'Zig-Zag (Direita)', chave });
                }
            }
            // Caso 4: Zig-Zig (Dir-Dir)
            else if (chave > raiz.dir.chave) {
                raiz.dir.dir = this.splay(raiz.dir.dir, chave, passos);
                raiz = this._rotacaoEsquerda(raiz);
                if (passos) passos.push({ tipo: 'Zig-Zig (Esquerda-Esquerda)', chave });
            }

            if (!raiz.dir) return raiz;
            let resultado = this._rotacaoEsquerda(raiz);
            if (passos) passos.push({ tipo: 'Zig (Esquerda)', chave });
            return resultado;
        }
    }

    insert(chave) {
        chave = Number(chave);
        if (isNaN(chave)) return { sucesso: false };

        let passos = [];
        if (!this.raiz) {
            this.raiz = new NoSplay(chave, this.proximoId++);
            this.totalNos++;
            return { sucesso: true, chave, totalNos: this.totalNos, passos: [{ tipo: 'Nova Raiz' }] };
        }

        // Traz o nó mais próximo para a raiz
        this.raiz = this.splay(this.raiz, chave, passos);

        if (this.raiz.chave === chave) {
            return { sucesso: false, chave, mensagem: 'Chave já existente', totalNos: this.totalNos };
        }

        let novo = new NoSplay(chave, this.proximoId++);
        this.totalNos++;

        if (chave < this.raiz.chave) {
            novo.dir = this.raiz;
            novo.esq = this.raiz.esq;
            this.raiz.esq = null;
        } else {
            novo.esq = this.raiz;
            novo.dir = this.raiz.dir;
            this.raiz.dir = null;
        }

        this.raiz = novo;
        passos.push({ tipo: 'Inserido na Raiz', chave });
        return {
            sucesso: true,
            chave,
            totalNos: this.totalNos,
            totalRotacoes: this.totalRotacoes,
            passos
        };
    }

    search(chave) {
        chave = Number(chave);
        if (isNaN(chave) || !this.raiz) return { encontrada: false, passos: [] };

        let passos = [];
        this.raiz = this.splay(this.raiz, chave, passos);
        let encontrada = (this.raiz.chave === chave);

        return {
            encontrada,
            chave,
            raizAtual: this.raiz.chave,
            totalRotacoes: this.totalRotacoes,
            passos
        };
    }

    remove(chave) {
        chave = Number(chave);
        if (isNaN(chave) || !this.raiz) return { sucesso: false };

        let passos = [];
        this.raiz = this.splay(this.raiz, chave, passos);

        if (this.raiz.chave !== chave) {
            return { sucesso: false, chave, mensagem: 'Chave não encontrada' };
        }

        let subEsq = this.raiz.esq;
        let subDir = this.raiz.dir;
        this.totalNos--;

        if (!subEsq) {
            this.raiz = subDir;
        } else {
            // Splay do maior elemento da subárvore esquerda
            subEsq = this.splay(subEsq, chave, passos);
            subEsq.dir = subDir;
            this.raiz = subEsq;
        }

        passos.push({ tipo: 'Removido e Recombinado (Join)' });
        return {
            sucesso: true,
            chave,
            totalNos: this.totalNos,
            totalRotacoes: this.totalRotacoes,
            passos
        };
    }

    toHierarchy() {
        const converter = (no) => {
            if (!no) return null;
            let children = [];
            if (no.esq) children.push(converter(no.esq));
            if (no.dir) children.push(converter(no.dir));
            return {
                id: no.id,
                label: `${no.chave}`,
                isRoot: no === this.raiz,
                children: children.filter(Boolean)
            };
        };
        return converter(this.raiz);
    }
}
