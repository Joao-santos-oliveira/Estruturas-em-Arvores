/**
 * Implementação da Árvore AVL com fatores de balanceamento e rotações
 */
class NoAVL {
    constructor(chave, id = 0) {
        this.id = id;
        this.chave = Number(chave);
        this.altura = 1;
        this.esq = null;
        this.dir = null;
    }
}

class AVLTreeVisual {
    constructor() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalNos = 0;
        this.totalRotacoes = 0;
    }

    reset() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalNos = 0;
        this.totalRotacoes = 0;
    }

    _altura(no) {
        return no ? no.altura : 0;
    }

    _fatorBalanceamento(no) {
        return no ? this._altura(no.esq) - this._altura(no.dir) : 0;
    }

    _atualizarAltura(no) {
        if (no) {
            no.altura = 1 + Math.max(this._altura(no.esq), this._altura(no.dir));
        }
    }

    _rotacaoDireita(y) {
        let x = y.esq;
        let T2 = x.dir;
        x.dir = y;
        y.esq = T2;
        this._atualizarAltura(y);
        this._atualizarAltura(x);
        this.totalRotacoes++;
        return x;
    }

    _rotacaoEsquerda(x) {
        let y = x.dir;
        let T2 = y.esq;
        y.esq = x;
        x.dir = T2;
        this._atualizarAltura(x);
        this._atualizarAltura(y);
        this.totalRotacoes++;
        return y;
    }

    insert(chave) {
        chave = Number(chave);
        if (isNaN(chave)) return { sucesso: false };

        let passos = [];

        const inserirRec = (no) => {
            if (!no) {
                this.totalNos++;
                return new NoAVL(chave, this.proximoId++);
            }

            if (chave < no.chave) no.esq = inserirRec(no.esq);
            else if (chave > no.chave) no.dir = inserirRec(no.dir);
            else return no; // Chave duplicada

            this._atualizarAltura(no);
            let fb = this._fatorBalanceamento(no);

            // Caso LL
            if (fb > 1 && chave < no.esq.chave) {
                passos.push({ tipo: 'Rotacao Simples Direita (LL)', no: no.chave });
                return this._rotacaoDireita(no);
            }
            // Caso RR
            if (fb < -1 && chave > no.dir.chave) {
                passos.push({ tipo: 'Rotacao Simples Esquerda (RR)', no: no.chave });
                return this._rotacaoEsquerda(no);
            }
            // Caso LR
            if (fb > 1 && chave > no.esq.chave) {
                passos.push({ tipo: 'Rotacao Dupla Esquerda-Direita (LR)', no: no.chave });
                no.esq = this._rotacaoEsquerda(no.esq);
                return this._rotacaoDireita(no);
            }
            // Caso RL
            if (fb < -1 && chave < no.dir.chave) {
                passos.push({ tipo: 'Rotacao Dupla Direita-Esquerda (RL)', no: no.chave });
                no.dir = this._rotacaoDireita(no.dir);
                return this._rotacaoEsquerda(no);
            }

            return no;
        };

        this.raiz = inserirRec(this.raiz);
        return { sucesso: true, chave, totalNos: this.totalNos, totalRotacoes: this.totalRotacoes, passos };
    }

    search(chave) {
        chave = Number(chave);
        let atual = this.raiz;
        let nosVisitados = [];
        while (atual) {
            nosVisitados.push(atual.id);
            if (chave === atual.chave) return { encontrada: true, chave, nosVisitados };
            atual = (chave < atual.chave) ? atual.esq : atual.dir;
        }
        return { encontrada: false, chave, nosVisitados };
    }

    remove(chave) {
        chave = Number(chave);
        let removeu = false;
        let passos = [];

        const removerRec = (no) => {
            if (!no) return null;

            if (chave < no.chave) {
                no.esq = removerRec(no.esq);
            } else if (chave > no.chave) {
                no.dir = removerRec(no.dir);
            } else {
                removeu = true;
                if (!no.esq || !no.dir) {
                    let temp = no.esq ? no.esq : no.dir;
                    if (!temp) {
                        no = null;
                    } else {
                        no = temp;
                    }
                    this.totalNos--;
                } else {
                    let temp = no.dir;
                    while (temp.esq) temp = temp.esq;
                    no.chave = temp.chave;
                    no.dir = removerRec(no.dir);
                }
            }

            if (!no) return null;

            this._atualizarAltura(no);
            let fb = this._fatorBalanceamento(no);

            // Casos de desbalanceamento pós-remoção
            if (fb > 1 && this._fatorBalanceamento(no.esq) >= 0) {
                passos.push({ tipo: 'Rotacao Direita (LL)', no: no.chave });
                return this._rotacaoDireita(no);
            }
            if (fb > 1 && this._fatorBalanceamento(no.esq) < 0) {
                passos.push({ tipo: 'Rotacao Dupla LR', no: no.chave });
                no.esq = this._rotacaoEsquerda(no.esq);
                return this._rotacaoDireita(no);
            }
            if (fb < -1 && this._fatorBalanceamento(no.dir) <= 0) {
                passos.push({ tipo: 'Rotacao Esquerda (RR)', no: no.chave });
                return this._rotacaoEsquerda(no);
            }
            if (fb < -1 && this._fatorBalanceamento(no.dir) > 0) {
                passos.push({ tipo: 'Rotacao Dupla RL', no: no.chave });
                no.dir = this._rotacaoDireita(no.dir);
                return this._rotacaoEsquerda(no);
            }

            return no;
        };

        this.raiz = removerRec(this.raiz);
        return { sucesso: removeu, chave, totalNos: this.totalNos, totalRotacoes: this.totalRotacoes, passos };
    }

    toHierarchy() {
        const converter = (no) => {
            if (!no) return null;
            let children = [];
            if (no.esq) children.push(converter(no.esq));
            if (no.dir) children.push(converter(no.dir));
            let fb = this._fatorBalanceamento(no);
            return {
                id: no.id,
                label: `${no.chave} (h:${no.altura}, fb:${fb})`,
                altura: no.altura,
                fb: fb,
                isRoot: no === this.raiz,
                children: children.filter(Boolean)
            };
        };
        return converter(this.raiz);
    }
}
