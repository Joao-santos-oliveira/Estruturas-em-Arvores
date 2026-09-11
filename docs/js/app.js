/**
 * Aplicação Principal: Controlador e Interface do Visualizador de Árvores
 */

document.addEventListener('DOMContentLoaded', () => {
    // Instâncias das estruturas
    const structures = {
        trie: new TrieVisual(),
        patricia: new PatriciaVisual(),
        splay: new SplayTreeVisual(),
        treap: new TreapVisual(),
        kdtree: new KDTreeVisual(),
        bst: new BSTVisual(),
        avl: new AVLTreeVisual()
    };

    let currentStructureKey = 'trie';
    let renderer = new TreeRenderer('mainTreeSvg');
    let spatialCanvas = null;

    // Elementos DOM
    const structureSelect = document.getElementById('structureSelect');
    const inputVal1 = document.getElementById('inputVal1');
    const inputVal2 = document.getElementById('inputVal2');
    const inputVal2Group = document.getElementById('inputVal2Group');
    const btnInsert = document.getElementById('btnInsert');
    const btnSearch = document.getElementById('btnSearch');
    const btnRemove = document.getElementById('btnRemove');
    const btnRandom = document.getElementById('btnRandom');
    const btnClear = document.getElementById('btnClear');
    const metricNodes = document.getElementById('metricNodes');
    const metricAux = document.getElementById('metricAux');
    const metricAuxLabel = document.getElementById('metricAuxLabel');
    const operationLog = document.getElementById('operationLog');

    const kdtreeDualContainer = document.getElementById('kdtreeDualContainer');
    const standardCanvasContainer = document.getElementById('standardCanvasContainer');

    // Inicialização da KD-Tree Spatial Canvas
    spatialCanvas = new Spatial2DCanvas('spatialCanvasContainer', (x, y) => {
        if (currentStructureKey === 'kdtree') {
            structures.kdtree.insert(x, y);
            logOperation(`[Clique 2D] Inserido ponto (${x}, ${y})`);
            updateView();
        }
    });

    // Navegação por Abas
    const navButtons = document.querySelectorAll('.tab-btn');
    const viewSections = document.querySelectorAll('.view-section');

    navButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            navButtons.forEach(b => b.classList.remove('active'));
            viewSections.forEach(s => s.classList.remove('active'));

            btn.classList.add('active');
            const targetSection = document.getElementById(btn.dataset.target);
            if (targetSection) targetSection.classList.add('active');

            if (btn.dataset.target === 'threeStatesSection') {
                renderThreeStatesView();
            } else if (btn.dataset.target === 'visualizerSection') {
                updateView();
            }
        });
    });

    // Alternar Tema (Dark / Light)
    const themeToggleBtn = document.getElementById('themeToggleBtn');
    themeToggleBtn.addEventListener('click', () => {
        const currentTheme = document.body.getAttribute('data-theme');
        const nextTheme = currentTheme === 'light' ? 'dark' : 'light';
        document.body.setAttribute('data-theme', nextTheme);
        themeToggleBtn.innerHTML = nextTheme === 'light' ? '🌙' : '☀️';
        if (spatialCanvas) spatialCanvas.draw();
    });

    // Troca de Estrutura
    structureSelect.addEventListener('change', (e) => {
        currentStructureKey = e.target.value;
        adjustInputsForStructure();
        updateView();
        logOperation(`Estrutura selecionada: ${structureSelect.options[structureSelect.selectedIndex].text}`);
    });

    function adjustInputsForStructure() {
        const isKD = currentStructureKey === 'kdtree';
        const isTreap = currentStructureKey === 'treap';
        const isString = currentStructureKey === 'trie' || currentStructureKey === 'patricia';

        if (isKD) {
            inputVal1.placeholder = "Coordenada X (0-100)";
            inputVal2.placeholder = "Coordenada Y (0-100)";
            inputVal2Group.style.display = 'flex';
            kdtreeDualContainer.style.display = 'grid';
            standardCanvasContainer.style.display = 'none';
        } else if (isTreap) {
            inputVal1.placeholder = "Chave Numérica (ex: 42)";
            inputVal2.placeholder = "Prioridade Heap (opcional)";
            inputVal2Group.style.display = 'flex';
            kdtreeDualContainer.style.display = 'none';
            standardCanvasContainer.style.display = 'block';
        } else {
            inputVal1.placeholder = isString ? "Palavra / String (ex: casa)" : "Chave Numérica (ex: 25)";
            inputVal2Group.style.display = 'none';
            kdtreeDualContainer.style.display = 'none';
            standardCanvasContainer.style.display = 'block';
        }
    }

    function logOperation(msg, isHighlight = false) {
        const entry = document.createElement('div');
        entry.className = `log-entry ${isHighlight ? 'highlight' : ''}`;
        const time = new Date().toLocaleTimeString();
        entry.textContent = `[${time}] ${msg}`;
        operationLog.appendChild(entry);
        operationLog.scrollTop = operationLog.scrollHeight;
    }

    const complexityInfo = {
        trie: {
            busca: "O(L)",
            insercao: "O(L)",
            remocao: "O(L)",
            espaco: "O(N · L · Σ)",
            nota: "L = tamanho da palavra, Σ = alfabeto"
        },
        patricia: {
            busca: "O(L)",
            insercao: "O(L)",
            remocao: "O(L)",
            espaco: "O(N · Σ) [Compacto]",
            nota: "Economia de nós por compressão de arestas"
        },
        splay: {
            busca: "O(log N) amort. / O(N) pior",
            insercao: "O(log N) amort. / O(N) pior",
            remocao: "O(log N) amort. / O(N) pior",
            espaco: "O(N)",
            nota: "Autoajuste trazendo nós para a raiz"
        },
        treap: {
            busca: "O(log N) prob. / O(N) pior",
            insercao: "O(log N) prob. / O(N) pior",
            remocao: "O(log N) prob. / O(N) pior",
            espaco: "O(N)",
            nota: "BST sobre Chave + Max-Heap em Prioridade"
        },
        kdtree: {
            busca: "1-NN: O(log N) médio / O(N)",
            insercao: "O(log N) médio / O(N)",
            remocao: "O(log N) médio / O(N)",
            espaco: "O(N · K)",
            nota: "Particionamento alternado em K=2 dimensões"
        },
        avl: {
            busca: "O(log N) garantido",
            insercao: "O(log N) garantido",
            remocao: "O(log N) garantido",
            espaco: "O(N)",
            nota: "Balanceamento estrito: FB ∈ {-1, 0, 1}"
        },
        bst: {
            busca: "O(log N) médio / O(N) pior",
            insercao: "O(log N) médio / O(N) pior",
            remocao: "O(log N) médio / O(N) pior",
            espaco: "O(N)",
            nota: "Sem balanceamento (risco de degeneração)"
        }
    };

    function updateComplexityCard() {
        const info = complexityInfo[currentStructureKey];
        const card = document.getElementById('complexityCardContent');
        if (!info || !card) return;

        card.innerHTML = `
            <div><strong>Busca:</strong> ${info.busca}</div>
            <div><strong>Inserção:</strong> ${info.insercao}</div>
            <div><strong>Remoção:</strong> ${info.remocao}</div>
            <div><strong>Espaço:</strong> ${info.espaco}</div>
            <div style="margin-top: 4px; font-size: 10.5px; color: var(--primary);">${info.nota}</div>
        `;
    }

    function updateMetrics() {
        const struct = structures[currentStructureKey];
        if (!struct) return;

        if (currentStructureKey === 'trie' || currentStructureKey === 'patricia') {
            metricNodes.textContent = struct.totalNos;
            metricAuxLabel.textContent = currentStructureKey === 'patricia' ? 'Splits de Aresta' : 'Palavras';
            metricAux.textContent = currentStructureKey === 'patricia' ? struct.totalSplits : struct.totalPalavras;
        } else if (currentStructureKey === 'kdtree') {
            metricNodes.textContent = struct.totalPontos;
            metricAuxLabel.textContent = 'Podas Espaciais';
            metricAux.textContent = struct.podasRealizadas;
        } else {
            metricNodes.textContent = struct.totalNos;
            metricAuxLabel.textContent = 'Rotações';
            metricAux.textContent = struct.totalRotacoes || 0;
        }

        updateComplexityCard();
    }

    function updateView(options = {}) {
        const struct = structures[currentStructureKey];
        if (!struct) return;

        const hierarchy = struct.toHierarchy();

        if (currentStructureKey === 'kdtree') {
            const kdTreeRenderer = new TreeRenderer('kdTreeSvg');
            kdTreeRenderer.render(hierarchy, options);

            const { linhas, pontos } = struct.coletarLinhasDeCorte();
            spatialCanvas.updateData(linhas, pontos, options);
        } else {
            renderer.render(hierarchy, options);
        }

        updateMetrics();
    }

    // Inserção
    btnInsert.addEventListener('click', () => {
        const val1 = inputVal1.value.trim();
        const val2 = inputVal2.value.trim();
        if (!val1) return;

        const struct = structures[currentStructureKey];
        let res;

        if (currentStructureKey === 'kdtree') {
            res = struct.insert(val1, val2 || Math.floor(Math.random() * 90) + 10);
            logOperation(`KD-Tree: Inserido (${val1}, ${val2})`);
        } else if (currentStructureKey === 'treap') {
            res = struct.insert(val1, val2 ? Number(val2) : null);
            logOperation(`Treap: Inserido ${val1} (p: ${res.prioridade})`);
        } else {
            res = struct.insert(val1);
            logOperation(`${structureSelect.value.toUpperCase()}: Inserido "${val1}"`);
        }

        inputVal1.value = '';
        inputVal2.value = '';
        updateView();
    });

    // Busca
    btnSearch.addEventListener('click', () => {
        const val1 = inputVal1.value.trim();
        const val2 = inputVal2.value.trim();
        if (!val1) return;

        const struct = structures[currentStructureKey];

        if (currentStructureKey === 'kdtree') {
            const alvoX = Number(val1);
            const alvoY = Number(val2 || 50);
            const res = struct.nearestNeighbor(alvoX, alvoY);
            if (res.vizinhoMaisProximo) {
                logOperation(`1-NN para (${alvoX},${alvoY}): Mais próximo = (${res.vizinhoMaisProximo.x},${res.vizinhoMaisProximo.y}), Dist = ${res.distanciaEuclidiana.toFixed(2)} (Calc: ${res.distanciasCalculadas})`, true);
                updateView({
                    queryPoint: { x: alvoX, y: alvoY },
                    nearestPoint: res.vizinhoMaisProximo,
                    queryRadius: res.distanciaEuclidiana,
                    highlightedNodes: res.nosVisitados
                });
            }
        } else {
            const res = struct.search(val1);
            const encontrada = res.encontrada;
            logOperation(`Busca por "${val1}": ${encontrada ? 'ENCONTRADA' : 'NÃO ENCONTRADA'}`, true);
            updateView({ highlightedNodes: res.nosVisitados || [] });
        }
    });

    // Remoção
    btnRemove.addEventListener('click', () => {
        const val1 = inputVal1.value.trim();
        const val2 = inputVal2.value.trim();
        if (!val1) return;

        const struct = structures[currentStructureKey];
        if (struct.remove) {
            let res;
            if (currentStructureKey === 'kdtree') {
                res = struct.remove(val1, val2 || 50);
                logOperation(`Remoção de Ponto (${val1}, ${val2 || 50}): ${res.sucesso ? 'Concluída' : 'Não encontrado'}`);
            } else {
                res = struct.remove(val1);
                logOperation(`Remoção de "${val1}": ${res.sucesso ? 'Concluída' : 'Não encontrado'}`);
            }
            inputVal1.value = '';
            if (inputVal2) inputVal2.value = '';
            updateView();
        } else {
            logOperation(`Remoção não aplicável para esta estrutura.`);
        }
    });

    // Aleatório
    btnRandom.addEventListener('click', () => {
        const struct = structures[currentStructureKey];
        if (currentStructureKey === 'trie' || currentStructureKey === 'patricia') {
            const palavrasExemplo = ['algoritmo', 'arvore', 'balanceamento', 'busca', 'chave', 'computacao', 'dado', 'estrutura', 'heap', 'prefixo'];
            const p = palavrasExemplo[Math.floor(Math.random() * palavrasExemplo.length)] + Math.floor(Math.random() * 99);
            struct.insert(p);
            logOperation(`Inserido aleatório: "${p}"`);
        } else if (currentStructureKey === 'kdtree') {
            const rx = Math.floor(Math.random() * 80) + 10;
            const ry = Math.floor(Math.random() * 80) + 10;
            struct.insert(rx, ry);
            logOperation(`Inserido ponto aleatório: (${rx}, ${ry})`);
        } else {
            const num = Math.floor(Math.random() * 99) + 1;
            struct.insert(num);
            logOperation(`Inserido número aleatório: ${num}`);
        }
        updateView();
    });

    // Limpar
    btnClear.addEventListener('click', () => {
        structures[currentStructureKey].reset();
        logOperation(`Árvore ${structureSelect.value.toUpperCase()} limpa.`);
        updateView();
    });

    // Inicialização dos 3 Estados Estruturais (Seção 3 do PDF)
    const threeStatesSelect = document.getElementById('threeStatesSelect');
    threeStatesSelect.addEventListener('change', renderThreeStatesView);

    function renderThreeStatesView() {
        const key = threeStatesSelect.value;
        const preset = THREE_STATES_PRESETS[key];
        if (!preset) return;

        document.getElementById('threeStatesDescription').textContent = preset.descricao;

        // Renderizar Estado 1
        const s1 = new (getConstructor(key))();
        popularEstrutura(s1, key, preset.estado1);
        document.getElementById('state1Desc').textContent = preset.estado1.explicacao;
        const r1 = new TreeRenderer('state1Svg');
        r1.render(s1.toHierarchy());

        // Renderizar Estado 2
        const s2 = new (getConstructor(key))();
        popularEstrutura(s2, key, preset.estado2);
        document.getElementById('state2Desc').textContent = preset.estado2.explicacao;
        const r2 = new TreeRenderer('state2Svg');
        r2.render(s2.toHierarchy(), { activeNodeId: preset.estado2.buscar ? 1 : null });

        // Renderizar Estado 3
        const s3 = new (getConstructor(key))();
        popularEstrutura(s3, key, preset.estado3);
        document.getElementById('state3Desc').textContent = preset.estado3.explicacao;
        const r3 = new TreeRenderer('state3Svg');
        r3.render(s3.toHierarchy());
    }

    function getConstructor(key) {
        switch (key) {
            case 'trie': return TrieVisual;
            case 'patricia': return PatriciaVisual;
            case 'splay': return SplayTreeVisual;
            case 'treap': return TreapVisual;
            case 'kdtree': return KDTreeVisual;
            default: return TrieVisual;
        }
    }

    function popularEstrutura(instancia, key, estadoConfig) {
        if (estadoConfig.chaves) {
            estadoConfig.chaves.forEach(c => instancia.insert(c));
        } else if (estadoConfig.chavesFinais) {
            estadoConfig.chavesFinais.forEach(c => instancia.insert(c));
        } else if (estadoConfig.pares) {
            estadoConfig.pares.forEach(([c, p]) => instancia.insert(c, p));
        } else if (estadoConfig.paresFinais) {
            estadoConfig.paresFinais.forEach(([c, p]) => instancia.insert(c, p));
        } else if (estadoConfig.pontos) {
            estadoConfig.pontos.forEach(([x, y]) => instancia.insert(x, y));
        }

        if (estadoConfig.buscar && instancia.search) {
            instancia.search(estadoConfig.buscar);
        }
    }

    // Exportador Automático para o Relatório (Versão ZIP)
    const btnExportAllSVGs = document.getElementById('btnExportAllSVGs');
    if (btnExportAllSVGs) {
        btnExportAllSVGs.addEventListener('click', async () => {
            if (typeof JSZip === 'undefined') {
                alert('A biblioteca JSZip não foi carregada. Verifique sua conexão com a internet.');
                return;
            }

            const zip = new JSZip();
            const trees = ['trie', 'patricia', 'splay', 'treap', 'kdtree'];
            const originalValue = threeStatesSelect.value;
            btnExportAllSVGs.textContent = 'Empacotando ZIP...';
            btnExportAllSVGs.disabled = true;

            for (let tree of trees) {
                threeStatesSelect.value = tree;
                renderThreeStatesView();
                
                // Extrai SVG com pequeno delay para garantir que renderizou
                await new Promise(r => setTimeout(r, 150));
                
                const ids = ['state1Svg', 'state2Svg', 'state3Svg'];
                for (let i = 0; i < ids.length; i++) {
                    let svg = document.querySelector('#' + ids[i] + ' svg').outerHTML;
                    if (!svg.includes('xmlns=')) {
                        svg = svg.replace('<svg', '<svg xmlns="http://www.w3.org/2000/svg"');
                    }
                    zip.file(`${tree}_estado${i + 1}.svg`, svg);
                }
            }

            threeStatesSelect.value = originalValue;
            renderThreeStatesView();

            try {
                const blob = await zip.generateAsync({type:"blob"});
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = "Imagens_SVGs_Relatorio.zip";
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                URL.revokeObjectURL(url);
                alert('Arquivo ZIP com as 15 imagens gerado com sucesso! Salvo nos seus Downloads.');
            } catch (err) {
                alert('Erro ao gerar o ZIP: ' + err);
            }

            btnExportAllSVGs.textContent = 'Baixar 15 Imagens (Relatório)';
            btnExportAllSVGs.disabled = false;
        });
    }

    // Inicialização com árvore vazia
    adjustInputsForStructure();
    updateView();
});
