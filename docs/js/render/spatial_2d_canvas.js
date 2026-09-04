/**
 * Renderizador Interativo do Plano Cartesiano 2D para a KD-Tree
 */
class Spatial2DCanvas {
    constructor(canvasContainerId, onAddPointCallback) {
        this.container = document.getElementById(canvasContainerId);
        this.onAddPoint = onAddPointCallback;
        this.canvas = null;
        this.ctx = null;
        this.width = 400;
        this.height = 400;

        // Limites de coordenadas do mundo (0 a 100)
        this.worldBounds = { minX: 0, maxX: 100, minY: 0, maxY: 100 };

        this.points = [];
        this.splitLines = [];
        this.queryPoint = null;
        this.nearestPoint = null;
        this.queryRadius = null;
        this.rangeBox = null;

        this._initCanvas();
    }

    _initCanvas() {
        if (!this.container) return;
        this.container.innerHTML = '';

        this.canvas = document.createElement('canvas');
        this.canvas.width = this.width;
        this.canvas.height = this.height;
        this.canvas.style.width = '100%';
        this.canvas.style.height = '100%';
        this.canvas.style.borderRadius = '10px';
        this.canvas.style.cursor = 'crosshair';

        this.ctx = this.canvas.getContext('2d');
        this.container.appendChild(this.canvas);

        this.canvas.addEventListener('click', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            const scaleX = this.canvas.width / rect.width;
            const scaleY = this.canvas.height / rect.height;

            const px = (e.clientX - rect.left) * scaleX;
            const py = (e.clientY - rect.top) * scaleY;

            // Converte pixel para coordenadas do mundo
            const wx = Math.round(this._pixelToWorldX(px));
            const wy = Math.round(this._pixelToWorldY(py));

            if (this.onAddPoint) this.onAddPoint(wx, wy);
        });
    }

    _worldToPixelX(x) {
        return ((x - this.worldBounds.minX) / (this.worldBounds.maxX - this.worldBounds.minX)) * this.width;
    }

    _worldToPixelY(y) {
        // Inverte o eixo Y para padrão cartesiano clássico
        return this.height - ((y - this.worldBounds.minY) / (this.worldBounds.maxY - this.worldBounds.minY)) * this.height;
    }

    _pixelToWorldX(px) {
        return this.worldBounds.minX + (px / this.width) * (this.worldBounds.maxX - this.worldBounds.minX);
    }

    _pixelToWorldY(py) {
        return this.worldBounds.minY + ((this.height - py) / this.height) * (this.worldBounds.maxY - this.worldBounds.minY);
    }

    updateData(splitLines, points, options = {}) {
        this.splitLines = splitLines || [];
        this.points = points || [];
        this.queryPoint = options.queryPoint || null;
        this.nearestPoint = options.nearestPoint || null;
        this.queryRadius = options.queryRadius || null;
        this.rangeBox = options.rangeBox || null;
        this.draw();
    }

    draw() {
        if (!this.ctx) return;
        const ctx = this.ctx;
        ctx.clearRect(0, 0, this.width, this.height);

        // 1. Fundo e Grade
        ctx.fillStyle = '#0F172A';
        ctx.fillRect(0, 0, this.width, this.height);

        ctx.strokeStyle = '#1E293B';
        ctx.lineWidth = 1;
        for (let i = 10; i < 100; i += 10) {
            const px = this._worldToPixelX(i);
            const py = this._worldToPixelY(i);
            ctx.beginPath();
            ctx.moveTo(px, 0); ctx.lineTo(px, this.height);
            ctx.moveTo(0, py); ctx.lineTo(this.width, py);
            ctx.stroke();
        }

        // 2. Linhas de Corte KD (Eixo X = Vermelho, Eixo Y = Azul)
        for (let l of this.splitLines) {
            ctx.lineWidth = 2;
            if (l.eixo === 'X') {
                ctx.strokeStyle = '#EF4444'; // Vermelho para corte vertical
            } else {
                ctx.strokeStyle = '#3B82F6'; // Azul para corte horizontal
            }
            ctx.beginPath();
            ctx.moveTo(this._worldToPixelX(l.x1), this._worldToPixelY(l.y1));
            ctx.lineTo(this._worldToPixelX(l.x2), this._worldToPixelY(l.y2));
            ctx.stroke();
        }

        // 3. Caixa de Range Search (se houver)
        if (this.rangeBox) {
            const rx1 = this._worldToPixelX(this.rangeBox.minX);
            const ry1 = this._worldToPixelY(this.rangeBox.maxY);
            const rw = this._worldToPixelX(this.rangeBox.maxX) - rx1;
            const rh = this._worldToPixelY(this.rangeBox.minY) - ry1;

            ctx.fillStyle = 'rgba(16, 185, 129, 0.2)';
            ctx.fillRect(rx1, ry1, rw, rh);
            ctx.strokeStyle = '#10B981';
            ctx.lineWidth = 2;
            ctx.setLineDash([4, 4]);
            ctx.strokeRect(rx1, ry1, rw, rh);
            ctx.setLineDash([]);
        }

        // 4. Raio de busca 1-NN (se houver)
        if (this.queryPoint && this.queryRadius !== null) {
            const qx = this._worldToPixelX(this.queryPoint.x);
            const qy = this._worldToPixelY(this.queryPoint.y);
            const rPx = (this.queryRadius / (this.worldBounds.maxX - this.worldBounds.minX)) * this.width;

            ctx.beginPath();
            ctx.arc(qx, qy, rPx, 0, Math.PI * 2);
            ctx.fillStyle = 'rgba(245, 158, 11, 0.15)';
            ctx.fill();
            ctx.strokeStyle = '#F59E0B';
            ctx.lineWidth = 1.5;
            ctx.setLineDash([3, 3]);
            ctx.stroke();
            ctx.setLineDash([]);
        }

        // 5. Desenhar Pontos
        for (let p of this.points) {
            const px = this._worldToPixelX(p.x);
            const py = this._worldToPixelY(p.y);

            const isNearest = this.nearestPoint && (this.nearestPoint.x === p.x && this.nearestPoint.y === p.y);

            ctx.beginPath();
            ctx.arc(px, py, isNearest ? 7 : 5, 0, Math.PI * 2);
            ctx.fillStyle = isNearest ? '#10B981' : (p.eixo === 0 ? '#EF4444' : '#3B82F6');
            ctx.fill();
            ctx.strokeStyle = '#FFFFFF';
            ctx.lineWidth = 2;
            ctx.stroke();

            // Rótulo de coordenadas
            ctx.fillStyle = '#CBD5E1';
            ctx.font = '9px Segoe UI, sans-serif';
            ctx.fillText(`(${p.x},${p.y})`, px + 6, py - 4);
        }

        // 6. Desenhar Ponto Alvo (Query)
        if (this.queryPoint) {
            const qx = this._worldToPixelX(this.queryPoint.x);
            const qy = this._worldToPixelY(this.queryPoint.y);

            ctx.beginPath();
            ctx.arc(qx, qy, 6, 0, Math.PI * 2);
            ctx.fillStyle = '#EC4899';
            ctx.fill();
            ctx.strokeStyle = '#FFFFFF';
            ctx.lineWidth = 2;
            ctx.stroke();

            ctx.fillStyle = '#EC4899';
            ctx.font = 'bold 10px Segoe UI, sans-serif';
            ctx.fillText(`Alvo (${this.queryPoint.x},${this.queryPoint.y})`, qx + 8, qy + 12);
        }
    }
}
