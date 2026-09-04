/**
 * Motor de Renderização Hierárquica Vetorial (SVG) com Pan & Zoom
 * Suporte aprimorado para nós especializados (Treap, KD-Tree, Patricia, AVL)
 */
class TreeRenderer {
    constructor(svgContainerId) {
        this.container = document.getElementById(svgContainerId);
        this.svg = null;
        this.g = null;
        this.scale = 1;
        this.translateX = 0;
        this.translateY = 0;
        this.isDragging = false;
        this.startX = 0;
        this.startY = 0;

        this.nodeRadius = 22;
        this.levelHeight = 75;
        this.siblingGap = 28;

        this._initSVG();
        this._setupEvents();
    }

    _initSVG() {
        if (!this.container) return;
        this.container.innerHTML = '';

        this.svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        this.svg.setAttribute('width', '100%');
        this.svg.setAttribute('height', '100%');
        this.svg.style.cursor = 'grab';

        const defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');
        defs.innerHTML = `
            <linearGradient id="nodeGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#3B82F6" />
                <stop offset="100%" stop-color="#1D4ED8" />
            </linearGradient>
            <linearGradient id="nodeEndGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#10B981" />
                <stop offset="100%" stop-color="#047857" />
            </linearGradient>
            <linearGradient id="nodeActiveGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#F59E0B" />
                <stop offset="100%" stop-color="#D97706" />
            </linearGradient>
            <linearGradient id="treapCardGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                <stop offset="0%" stop-color="#1E293B" />
                <stop offset="100%" stop-color="#0F172A" />
            </linearGradient>
            <filter id="nodeShadow" x="-20%" y="-20%" width="140%" height="140%">
                <feDropShadow dx="0" dy="3" stdDeviation="3" flood-color="#000" flood-opacity="0.25" />
            </filter>
        `;
        this.svg.appendChild(defs);

        this.g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        this.svg.appendChild(this.g);
        this.container.appendChild(this.svg);
    }

    _setupEvents() {
        if (!this.container) return;

        this.svg.addEventListener('mousedown', (e) => {
            this.isDragging = true;
            this.startX = e.clientX - this.translateX;
            this.startY = e.clientY - this.translateY;
            this.svg.style.cursor = 'grabbing';
        });

        window.addEventListener('mousemove', (e) => {
            if (!this.isDragging) return;
            this.translateX = e.clientX - this.startX;
            this.translateY = e.clientY - this.startY;
            this._updateTransform();
        });

        window.addEventListener('mouseup', () => {
            this.isDragging = false;
            if (this.svg) this.svg.style.cursor = 'grab';
        });

        this.container.addEventListener('wheel', (e) => {
            e.preventDefault();
            const delta = e.deltaY > 0 ? 0.9 : 1.1;
            const newScale = Math.min(Math.max(this.scale * delta, 0.2), 3.0);

            const rect = this.container.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const mouseY = e.clientY - rect.top;

            this.translateX = mouseX - (mouseX - this.translateX) * (newScale / this.scale);
            this.translateY = mouseY - (mouseY - this.translateY) * (newScale / this.scale);
            this.scale = newScale;

            this._updateTransform();
        }, { passive: false });
    }

    _updateTransform() {
        if (this.g) {
            this.g.setAttribute('transform', `translate(${this.translateX}, ${this.translateY}) scale(${this.scale})`);
        }
    }

    resetView() {
        this.scale = 1;
        this.translateX = 0;
        this.translateY = 0;
        this._updateTransform();
    }

    _calcularLayout(treeData) {
        if (!treeData) return null;

        const calcularSubarvore = (no, nivel = 0) => {
            no.nivel = nivel;
            no.y = nivel * this.levelHeight + 50;

            const isTreap = no.isTreap;
            const nodeWidth = isTreap ? 80 : (this.nodeRadius * 2);

            if (!no.children || no.children.length === 0) {
                no.larguraSubarvore = nodeWidth + this.siblingGap;
                return;
            }

            let larguraTotal = 0;
            for (let filho of no.children) {
                calcularSubarvore(filho, nivel + 1);
                larguraTotal += filho.larguraSubarvore;
            }
            no.larguraSubarvore = Math.max(larguraTotal, nodeWidth + this.siblingGap);
        };

        calcularSubarvore(treeData, 0);

        const atribuirX = (no, xInicial) => {
            if (!no.children || no.children.length === 0) {
                no.x = xInicial + no.larguraSubarvore / 2;
                return;
            }

            let xAtual = xInicial;
            for (let filho of no.children) {
                atribuirX(filho, xAtual);
                xAtual += filho.larguraSubarvore;
            }

            let primeiroFilho = no.children[0];
            let ultimoFilho = no.children[no.children.length - 1];
            no.x = (primeiroFilho.x + ultimoFilho.x) / 2;
        };

        const larguraContainer = this.container ? this.container.clientWidth : 800;
        const xInicial = (larguraContainer - treeData.larguraSubarvore) / 2;
        atribuirX(treeData, Math.max(20, xInicial));

        return treeData;
    }

    render(treeData, options = {}) {
        if (!this.g) return;
        this.g.innerHTML = '';

        if (!treeData) {
            this.g.innerHTML = `
                <text x="50%" y="50%" text-anchor="middle" fill="#64748B" font-size="13" font-family="Inter, sans-serif">
                    Árvore vazia. Insira elementos para visualizar.
                </text>
            `;
            return;
        }

        const layout = this._calcularLayout(treeData);
        const highlightedNodes = options.highlightedNodes || [];
        const activeNodeId = options.activeNodeId || null;

        // 1. Arestas
        const desenharArestas = (no) => {
            if (!no.children) return;
            for (let filho of no.children) {
                const line = document.createElementNS('http://www.w3.org/2000/svg', 'path');
                const d = `M ${no.x} ${no.y} C ${no.x} ${(no.y + filho.y) / 2}, ${filho.x} ${(no.y + filho.y) / 2}, ${filho.x} ${filho.y}`;
                line.setAttribute('d', d);
                line.setAttribute('stroke', '#475569');
                line.setAttribute('stroke-width', '2');
                line.setAttribute('fill', 'none');
                this.g.appendChild(line);

                // Rótulo na aresta (Trie / Patricia)
                if (filho.edgeLabel && filho.edgeLabel !== '') {
                    const midX = (no.x + filho.x) / 2;
                    const midY = (no.y + filho.y) / 2;

                    const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                    const txt = filho.edgeLabel;
                    const w = Math.max(22, txt.length * 8 + 8);
                    rect.setAttribute('x', midX - w / 2);
                    rect.setAttribute('y', midY - 9);
                    rect.setAttribute('width', w);
                    rect.setAttribute('height', 18);
                    rect.setAttribute('rx', 4);
                    rect.setAttribute('fill', '#1E293B');
                    rect.setAttribute('stroke', '#3B82F6');
                    rect.setAttribute('stroke-width', '1');
                    this.g.appendChild(rect);

                    const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                    text.setAttribute('x', midX);
                    text.setAttribute('y', midY + 4);
                    text.setAttribute('text-anchor', 'middle');
                    text.setAttribute('font-size', '10.5');
                    text.setAttribute('font-weight', '700');
                    text.setAttribute('fill', '#60A5FA');
                    text.textContent = txt;
                    this.g.appendChild(text);
                }

                desenharArestas(filho);
            }
        };
        desenharArestas(layout);

        // 2. Nós
        const desenharNos = (no) => {
            const nodeG = document.createElementNS('http://www.w3.org/2000/svg', 'g');
            nodeG.setAttribute('transform', `translate(${no.x}, ${no.y})`);

            const isHigh = highlightedNodes.includes(no.id);
            const isActive = activeNodeId === no.id;

            // RENDERIZAÇÃO ESPECIALIZADA PARA TREAP (Chave + Prioridade)
            if (no.isTreap) {
                const cardW = 74;
                const cardH = 44;

                const card = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                card.setAttribute('x', -cardW / 2);
                card.setAttribute('y', -cardH / 2);
                card.setAttribute('width', cardW);
                card.setAttribute('height', cardH);
                card.setAttribute('rx', 8);
                card.setAttribute('fill', isHigh ? '#F59E0B' : '#1E293B');
                card.setAttribute('stroke', isHigh ? '#FFF' : (isActive ? '#EC4899' : '#3B82F6'));
                card.setAttribute('stroke-width', isHigh || isActive ? '2.5' : '1.5');
                card.setAttribute('filter', 'url(#nodeShadow)');
                nodeG.appendChild(card);

                // Texto Chave (BST)
                const txtChave = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                txtChave.setAttribute('x', '0');
                txtChave.setAttribute('y', '-3');
                txtChave.setAttribute('text-anchor', 'middle');
                txtChave.setAttribute('font-size', '11.5');
                txtChave.setAttribute('font-weight', '700');
                txtChave.setAttribute('fill', '#F8FAFC');
                txtChave.textContent = `k: ${no.chave}`;
                nodeG.appendChild(txtChave);

                // Badge de Prioridade (Heap)
                const badge = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                badge.setAttribute('x', -26);
                badge.setAttribute('y', 4);
                badge.setAttribute('width', 52);
                badge.setAttribute('height', 14);
                badge.setAttribute('rx', 4);
                badge.setAttribute('fill', 'rgba(217, 119, 6, 0.3)');
                badge.setAttribute('stroke', '#D97706');
                badge.setAttribute('stroke-width', '1');
                nodeG.appendChild(badge);

                const txtPrio = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                txtPrio.setAttribute('x', '0');
                txtPrio.setAttribute('y', '14.5');
                txtPrio.setAttribute('text-anchor', 'middle');
                txtPrio.setAttribute('font-size', '9.5');
                txtPrio.setAttribute('font-weight', '700');
                txtPrio.setAttribute('fill', '#FBBF24');
                txtPrio.textContent = `p: ${no.prioridade}`;
                nodeG.appendChild(txtPrio);
            }
            // RENDERIZAÇÃO PADRÃO PARA DEMAIS ESTRUTURAS
            else {
                let labelText = String(no.label || '');
                let isWide = labelText.length > 5;

                let shape;
                if (isWide) {
                    shape = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
                    const rw = Math.max(50, labelText.length * 8 + 16);
                    shape.setAttribute('x', -rw / 2);
                    shape.setAttribute('y', -this.nodeRadius);
                    shape.setAttribute('width', rw);
                    shape.setAttribute('height', this.nodeRadius * 2);
                    shape.setAttribute('rx', 8);
                } else {
                    shape = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
                    shape.setAttribute('r', this.nodeRadius);
                }

                let fill = 'url(#nodeGrad)';
                if (no.isEnd) fill = 'url(#nodeEndGrad)';
                if (isHigh) fill = 'url(#nodeActiveGrad)';
                if (isActive) fill = '#EC4899';

                shape.setAttribute('fill', fill);
                shape.setAttribute('stroke', isHigh || isActive ? '#FFFFFF' : '#64748B');
                shape.setAttribute('stroke-width', isHigh || isActive ? '3' : '1.5');
                shape.setAttribute('filter', 'url(#nodeShadow)');
                nodeG.appendChild(shape);

                const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                text.setAttribute('text-anchor', 'middle');
                text.setAttribute('y', '4.5');
                text.setAttribute('font-size', isWide ? '10' : '11.5');
                text.setAttribute('font-weight', '700');
                text.setAttribute('fill', '#FFFFFF');
                text.textContent = labelText;
                nodeG.appendChild(text);
            }

            this.g.appendChild(nodeG);

            if (no.children) {
                for (let filho of no.children) desenharNos(filho);
            }
        };
        desenharNos(layout);
    }
}
