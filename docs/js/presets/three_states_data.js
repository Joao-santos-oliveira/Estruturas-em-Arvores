/**
 * Cenários dos 3 Estados Estruturais canônicos (Seção 3 do PDF do Trabalho Prático)
 */
const THREE_STATES_PRESETS = {
    trie: {
        nome: "Árvore Trie",
        descricao: "Evolução estrutural: Inserção de prefixos comuns compartilhados e poda de nós órfãos na remoção.",
        estado1: {
            titulo: "Estado 1: Pós-Inserção Inicial",
            explicacao: "Inserção das chaves ['casa', 'caso', 'castelo']. As palavras compartilham o prefixo 'cas', ramificando em 'a', 'o' e 't'.",
            chaves: ["casa", "caso", "castelo"]
        },
        estado2: {
            titulo: "Estado 2: Intermediário (Bifurcação de Prefixos)",
            explicacao: "Inserção de 'caminho' e 'carro'. O nó raiz 'c' passa a ter novos ramos 'a' -> 'm' e 'a' -> 'r', ampliando o grau de ramificação.",
            chaves: ["casa", "caso", "castelo", "caminho", "carro"]
        },
        estado3: {
            titulo: "Estado 3: Pós-Remoção com Poda Estrutural",
            explicacao: "Remoção de 'castelo'. Os nós ('t' -> 'e' -> 'l' -> 'o') sem outros filhos são podados da árvore, liberando memória.",
            remover: "castelo",
            chavesFinais: ["casa", "caso", "caminho", "carro"]
        }
    },
    patricia: {
        nome: "Árvore Patricia (Radix Compacta)",
        descricao: "Evolução estrutural: Compactação de arestas, divisão de prefixos (Split) e fusão na remoção (Merge).",
        estado1: {
            titulo: "Estado 1: Pós-Inserção Inicial",
            explicacao: "Inserção de ['teste', 'testar']. Como compartilham 'test', a aresta é compactada com prefixo 'test' e bifurca em 'e' e 'ar'.",
            chaves: ["teste", "testar"]
        },
        estado2: {
            titulo: "Estado 2: Intermediário (Divisão de Aresta - Split)",
            explicacao: "Inserção de 'tempo'. A aresta 'test' é dividida em 'te', gerando um nó intermediário que aponta para 'st' e 'mpo'.",
            chaves: ["teste", "testar", "tempo"]
        },
        estado3: {
            titulo: "Estado 3: Pós-Remoção com Fusão de Nós (Merge)",
            explicacao: "Remoção de 'tempo'. O nó intermediário 'te' fica com apenas um filho ('st') e sofre fusão (merge), restaurando a aresta 'test'.",
            remover: "tempo",
            chavesFinais: ["teste", "testar"]
        }
    },
    splay: {
        nome: "Árvore Splay",
        descricao: "Evolução estrutural: Autoajuste dinâmico por rotações Zig, Zig-Zig e Zig-Zag trazendo chaves para a raiz.",
        estado1: {
            titulo: "Estado 1: Pós-Inserção Inicial",
            explicacao: "Inserção de [10, 20, 30, 40, 50]. Cada inserção traz a nova chave para a raiz através de rotações Zig-Zig.",
            chaves: [10, 20, 30, 40, 50]
        },
        estado2: {
            titulo: "Estado 2: Intermediário (Autoajuste por Busca - Splay)",
            explicacao: "Busca pelo elemento profundo '10'. O algoritmo aplica rotações Zig-Zig sucessivas, trazendo o 10 para a raiz e diminuindo a profundidade.",
            operacao: "search(10)",
            chaves: [10, 20, 30, 40, 50],
            buscar: 10
        },
        estado3: {
            titulo: "Estado 3: Pós-Remoção com Recombinação (Split & Join)",
            explicacao: "Remoção de '50'. O elemento é trazido para a raiz por splay, removido, e as subárvores esquerda e direita são recombinadas.",
            remover: 50,
            chavesFinais: [10, 20, 30, 40]
        }
    },
    treap: {
        nome: "Árvore Treap (Tree + Heap)",
        descricao: "Evolução estrutural: Invariante de BST nas chaves e Max-Heap nas prioridades geradas deterministicamente.",
        estado1: {
            titulo: "Estado 1: Pós-Inserção Inicial",
            explicacao: "Inserção de [(20, p:90), (10, p:70), (30, p:85)]. A chave 20 é raiz por ter prioridade 90.",
            pares: [[20, 90], [10, 70], [30, 85]]
        },
        estado2: {
            titulo: "Estado 2: Intermediário (Subida por Rotação de Heap)",
            explicacao: "Inserção de (15, p:99). Como a prioridade 99 supera todas, o nó sobe através de rotações até assumir a nova raiz.",
            pares: [[20, 90], [10, 70], [30, 85], [15, 99]]
        },
        estado3: {
            titulo: "Estado 3: Pós-Remoção com Rotações Descendentes",
            explicacao: "Remoção de '15'. O nó da raiz desce por rotações com o filho de maior prioridade até virar folha e ser excluído.",
            remover: 15,
            paresFinais: [[20, 90], [10, 70], [30, 85]]
        }
    },
    kdtree: {
        nome: "KD-Tree (2D)",
        descricao: "Evolução estrutural: Particionamento alternado do espaço por hiperplanos ortogonais (X e Y).",
        estado1: {
            titulo: "Estado 1: Pós-Inserção Inicial",
            explicacao: "Inserção de [(50,50), (25,30), (75,80)]. A raiz corta o plano verticalmente em X=50; os filhos cortam horizontalmente em Y=30 e Y=80.",
            pontos: [[50, 50], [25, 30], [75, 80]]
        },
        estado2: {
            titulo: "Estado 2: Intermediário (Particionamento Espacial Denso)",
            explicacao: "Inserção de [(20,80), (35,10), (80,20)]. O plano cartesiano 2D é dividido em 6 quadrantes ortogonais delimitados.",
            pontos: [[50, 50], [25, 30], [75, 80], [20, 80], [35, 10], [80, 20]]
        },
        estado3: {
            titulo: "Estado 3: Consulta 1-NN com Poda de Hiperplano",
            explicacao: "Consulta de Vizinho Mais Próximo para o alvo (22, 75). A hiperesfera encontra (20,80) e poda a subárvore direita de X=50 sem visitá-la.",
            alvoNN: [22, 75],
            pontos: [[50, 50], [25, 30], [75, 80], [20, 80], [35, 10], [80, 20]]
        }
    }
};
