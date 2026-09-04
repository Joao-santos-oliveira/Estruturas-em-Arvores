/**
 * Implementação da BST (Árvore Binária de Busca Padrão)
 */
class NoBST {
    constructor(chave, id = 0) {
        this.id = id;
        this.chave = Number(chave);
        this.esq = null;
        this.dir = null;
    }
}

class BSTVisual {
    constructor() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalNos = 0;
    }

    reset() {
        this.proximoId = 1;
        this.raiz = null;
        this.totalNos = 0;
    }

    insert(chave) {
        chave = Number(chave);
        if (isNaN(chave)) return { sucesso: false };

        const inserirRec = (no) => {
            if (!no) {
                this.totalNos++;
                return new NoBST(chave, this.proximoId++);
            }
            if (chave < no.chave) no.esq = inserirRec(no.esq);
            else if (chave > no.chave) no.dir = inserirRec(no.dir);
            return no;
        };

        this.raiz = inserirRec(this.raiz);
        return { sucesso: true, chave, totalNos: this.totalNos };
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

        const removerRec = (no) => {
            if (!no) return null;
            if (chave < no.chave) {
                no.esq = removerRec(no.esq);
            } else if (chave > no.chave) {
                no.dir = removerRec(no.dir);
            } else {
                removeu = true;
                if (!no.esq) {
                    this.totalNos--;
                    return no.dir;
                } else if (!no.dir) {
                    this.totalNos--;
                    return no.esq;
                }
                // 2 filhos: sucessor in-order (menor da subárvore direita)
                let temp = no.dir;
                while (temp.esq) temp = temp.esq;
                no.chave = temp.chave;
                no.dir = removerRec(no.dir);
            }
            return no;
        };

        this.raiz = removerRec(this.raiz);
        return { sucesso: removeu, chave, totalNos: this.totalNos };
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
