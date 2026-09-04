/**
 * Implementação da Árvore Treap (BST sobre Chave + Max-Heap sobre Prioridade)
 */
class NoTreap {
    constructor(chave, prioridade, id = 0) {
        this.id = id;
        this.chave = chave;
        this.prioridade = prioridade;
        this.esq = null;
        this.dir = null;
    }
}

class TreapVisual {
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

    _rotacaoDireita(y) {
        let x = y.esq;
        y.esq = x.dir;
        x.dir = y;
        this.totalRotacoes++;
        return x;
    }

    _rotacaoEsquerda(x) {
        let y = x.dir;
        x.dir = y.esq;
        y.esq = x;
        this.totalRotacoes++;
        return y;
    }

    insert(chave, prioridade = null) {
        chave = Number(chave);
        if (isNaN(chave)) return { sucesso: false };
        if (prioridade === null || isNaN(Number(prioridade))) {
            prioridade = Math.floor(Math.random() * 1000) + 1;
        } else {
            prioridade = Number(prioridade);
        }

        let passos = [];
        let inseriuNovo = false;

        const inserirRec = (no) => {
            if (!no) {
                this.totalNos++;
                inseriuNovo = true;
                return new NoTreap(chave, prioridade, this.proximoId++);
            }

            if (chave < no.chave) {
                no.esq = inserirRec(no.esq);
                // Mantém propriedade de Max-Heap: filho não pode ter prioridade maior que pai
                if (no.esq.prioridade > no.prioridade) {
                    passos.push({ tipo: 'Rotacao Direita (Heap Fix)', pai: no.chave, filho: no.esq.chave });
                    no = this._rotacaoDireita(no);
                }
            } else if (chave > no.chave) {
                no.dir = inserirRec(no.dir);
                if (no.dir.prioridade > no.prioridade) {
                    passos.push({ tipo: 'Rotacao Esquerda (Heap Fix)', pai: no.chave, filho: no.dir.chave });
                    no = this._rotacaoEsquerda(no);
                }
            }
            return no;
        };

        this.raiz = inserirRec(this.raiz);
        return {
            sucesso: inseriuNovo,
            chave,
            prioridade,
            totalNos: this.totalNos,
            totalRotacoes: this.totalRotacoes,
            passos
        };
    }

    search(chave) {
        chave = Number(chave);
        let atual = this.raiz;
        let nosVisitados = [];

        while (atual) {
            nosVisitados.push(atual.id);
            if (chave === atual.chave) {
                return { encontrada: true, chave, prioridade: atual.prioridade, nosVisitados };
            }
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
                // Nó encontrado
                removeu = true;
                if (!no.esq && !no.dir) {
                    this.totalNos--;
                    return null;
                } else if (!no.esq) {
                    passos.push({ tipo: 'Rotacao Descendente Esquerda', chave: no.chave });
                    no = this._rotacaoEsquerda(no);
                    no.esq = removerRec(no.esq);
                } else if (!no.dir) {
                    passos.push({ tipo: 'Rotacao Descendente Direita', chave: no.chave });
                    no = this._rotacaoDireita(no);
                    no.dir = removerRec(no.dir);
                } else {
                    // Ambos os filhos existem: rotaciona com o de maior prioridade
                    if (no.esq.prioridade > no.dir.prioridade) {
                        passos.push({ tipo: 'Rotacao Descendente Direita', chave: no.chave });
                        no = this._rotacaoDireita(no);
                        no.dir = removerRec(no.dir);
                    } else {
                        passos.push({ tipo: 'Rotacao Descendente Esquerda', chave: no.chave });
                        no = this._rotacaoEsquerda(no);
                        no.esq = removerRec(no.esq);
                    }
                }
            }
            return no;
        };

        this.raiz = removerRec(this.raiz);
        return {
            sucesso: removeu,
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
                label: `k:${no.chave} p:${no.prioridade}`,
                chave: no.chave,
                prioridade: no.prioridade,
                isTreap: true,
                isRoot: no === this.raiz,
                children: children.filter(Boolean)
            };
        };
        return converter(this.raiz);
    }
}
