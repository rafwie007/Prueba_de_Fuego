#include "js.h"

const char APP_JS[] PROGMEM = R"rawliteral(

const MAX = 600;
let ts=[], t1s=[], t2s=[], pr=[], pl=[];

// Threshold crossing times (in seconds from experiment start)
let threshTimes = { t30: null, t180: null, t200: null };

// ── Frozen snapshot taken at STOP ──────────────────────────────
// While isRecording=false, push() still receives idle readings from the
// ESP but we must NOT overwrite the experiment data the user wants to
// export. snapshotCanvas holds the last frame drawn during recording.
let isRecording = false;
let snapshotCanvas = null;   // offscreen canvas frozen at STOP

function freezeSnapshot() {
  snapshotCanvas = document.createElement('canvas');
  snapshotCanvas.width  = cv.width;
  snapshotCanvas.height = cv.height;
  snapshotCanvas.getContext('2d').drawImage(cv, 0, 0);
}

const cv       = document.getElementById('cv');
const ctx      = cv.getContext('2d');
const connBadge    = document.getElementById('connBadge');
const recStatus  = document.getElementById('recStatus');
const recLabel   = document.getElementById('recLabel');
const elapsedTime = document.getElementById('elapsedTime');
const chartWrap  = document.getElementById('chartWrap');
const btnStart   = document.getElementById('btnStart');
const btnStop    = document.getElementById('btnStop');
const expNameInput = document.getElementById('expName');

// ── Connection badge ───────────────────────────
function setConn(connected) {
  connBadge.textContent = connected ? 'Conectado' : 'Sin conexion';
  connBadge.classList.toggle('badge-connected', connected);
  connBadge.classList.toggle('badge-disconnected', !connected);
}
setConn(false);

// ── Canvas resize ──────────────────────────────
function resize() {
  cv.width  = cv.offsetWidth;
  cv.height = cv.offsetHeight || 300;
}
window.addEventListener('resize', () => { resize(); draw(); });
resize();

// ── WiFi UI ────────────────────────────────────
function loadStatus() {
  fetch('/status').then(r => r.json()).then(info => {
    const running = !!info.running;
    setButtons(running);
    setRunningState(running);
  }).catch(() => { setButtons(false); setRunningState(false); });
}

loadStatus();

// ── Threshold UI update ───────────────────────
function updateThreshUI() {
  function fmt(s) {
    if (s === null) return '--';
    return s.toFixed(2) + 's';
  }
  const el30  = document.getElementById('thresh30');
  const el180 = document.getElementById('thresh180');
  const el200 = document.getElementById('thresh200');

  el30.textContent  = fmt(threshTimes.t30);
  el180.textContent = fmt(threshTimes.t180);
  el200.textContent = fmt(threshTimes.t200);

  el30.classList.toggle('reached',  threshTimes.t30  !== null);
  el180.classList.toggle('reached', threshTimes.t180 !== null);
  el200.classList.toggle('reached', threshTimes.t200 !== null);
}

// ── DRAW ───────────────────────────────────────
function draw() {
  const W = cv.width, H = cv.height;
  const left = 52, right = 18, top = 20, bottom = 44;

  ctx.clearRect(0, 0, W, H);
  if (ts.length < 2) return;

  // Dynamic Y range — follows data only, thresholds float in/out of view
  const allTemps = [...pr, ...pl, ...t1s, ...t2s];
  const rawMin = Math.min(...allTemps);
  const rawMax = Math.max(...allTemps);
  const span   = Math.max(rawMax - rawMin, 20);        // never collapse to zero
  const headroom = span * 0.12;                         // 12% clear space above
  const yMin = Math.floor((rawMin - headroom) / 10) * 10;
  const yMax = Math.ceil ((rawMax + headroom) / 10) * 10;

  if (!isFinite(yMin) || !isFinite(yMax)) return;

  const x0 = ts[0], xn = ts[ts.length - 1];
  const xRange = (xn - x0) || 1;
  const yRange = (yMax - yMin) || 1;

  const px = t  => left + ((t  - x0) / xRange) * (W - left - right);
  const py = v  => H - bottom - ((v - yMin) / yRange) * (H - top - bottom);

  // ── Grid & Y axis labels ──────────────────────
  const ySteps = 6;
  ctx.strokeStyle = '#0e2030';
  ctx.lineWidth = 1;
  ctx.font = '10px monospace';
  ctx.fillStyle = '#3a6a8a';

  for (let i = 0; i <= ySteps; i++) {
    const v = yMin + i * (yMax - yMin) / ySteps;
    const y = py(v);
    ctx.strokeStyle = '#0e2030';
    ctx.setLineDash([2, 4]);
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(W - right, y);
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.fillStyle = '#3a6a8a';
    ctx.fillText(v.toFixed(0) + '\u00b0', 3, y + 3);
  }

  // X axis labels — seconds, snapped to 30s or 60s intervals
  const xRange_s = xn - x0;
  // Pick a tick interval that gives ~5-8 ticks: prefer 60s, fall back to 30s for short runs
  const tickInterval = xRange_s <= 270 ? 30 : 60;
  const firstTick = Math.ceil(x0 / tickInterval) * tickInterval;

  ctx.font = '10px monospace';
  for (let t = firstTick; t <= xn + 0.01; t += tickInterval) {
    const x = px(t);
    if (x < left || x > W - right) continue;
    ctx.strokeStyle = '#1e2e3e';
    ctx.lineWidth = 1;
    ctx.setLineDash([2, 4]);
    ctx.beginPath();
    ctx.moveTo(x, top);
    ctx.lineTo(x, H - bottom);
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.fillStyle = '#3a6a8a';
    const label = t.toFixed(0) + 's';
    ctx.fillText(label, x - ctx.measureText(label).width / 2, H - bottom + 14);
  }

  // ── Axes ──────────────────────────────────────
  ctx.strokeStyle = '#144e73';
  ctx.lineWidth = 1.5;
  ctx.beginPath(); ctx.moveTo(left, top); ctx.lineTo(left, H - bottom); ctx.stroke();
  ctx.beginPath(); ctx.moveTo(left, H - bottom); ctx.lineTo(W - right, H - bottom); ctx.stroke();

  // ── Axis titles ───────────────────────────────
  ctx.save();
  ctx.fillStyle = '#144e73';
  ctx.font = '10px monospace';
  ctx.translate(11, (H - bottom + top) / 2);
  ctx.rotate(-Math.PI / 2);
  ctx.fillText('Temp (\u00b0C)', 0, 0);
  ctx.restore();
  ctx.fillStyle = '#144e73';
  ctx.font = '10px monospace';
  ctx.fillText('Tiempo (segundos)', left + (W - left - right) / 2 - 50, H - 3);

  // ── Threshold lines ───────────────────────────
  const threshDefs = [
    { val: 30,  color: '#f44', dash: [4, 6],  alpha: 0.35, key: 't30'  },
    { val: 180, color: '#f44', dash: [6, 4],  alpha: 0.75, key: 't180' },
    { val: 200, color: '#f88', dash: [8, 4],  alpha: 0.75, key: 't200' },
  ];
  ctx.font = '10px monospace';
  threshDefs.forEach(th => {
    const y = py(th.val);
    ctx.globalAlpha = th.alpha;
    ctx.strokeStyle = th.color;
    ctx.lineWidth = 1;
    ctx.setLineDash(th.dash);
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(W - right, y);
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.globalAlpha = th.alpha * 0.9;
    ctx.fillStyle = th.color;
    ctx.fillText(th.val + '\u00b0C', W - right - 32, y - 3);
    ctx.globalAlpha = 1;
  });

  // ── Data curves ───────────────────────────────
  // T1, T2 — subtle validation lines (behind main data)
  [[t1s, '#555', 1.2], [t2s, '#333', 1.0]].forEach(([arr, color, lw]) => {
    if (!arr.length) return;
    ctx.strokeStyle = color;
    ctx.lineWidth = lw;
    ctx.globalAlpha = 0.8;
    ctx.beginPath();
    arr.forEach((v, i) => {
      const x = px(ts[i]), y = py(v);
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
    });
    ctx.stroke();
    ctx.globalAlpha = 1;
  });

  // Furnace average — medium prominence
  if (pr.length) {
    ctx.strokeStyle = '#3ab8f0';
    ctx.lineWidth = 2;
    ctx.globalAlpha = 0.9;
    ctx.beginPath();
    pr.forEach((v, i) => {
      const x = px(ts[i]), y = py(v);
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
    });
    ctx.stroke();
    ctx.globalAlpha = 1;
  }

  // Cold face (placa) — dominant curve
  if (pl.length) {
    ctx.strokeStyle = '#f09030';
    ctx.lineWidth = 2.8;
    ctx.beginPath();
    pl.forEach((v, i) => {
      const x = px(ts[i]), y = py(v);
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
    });
    ctx.stroke();
  }

  // ── Threshold intersection markers ───────────
  threshDefs.forEach(th => {
    const tTime = threshTimes[th.key];
    if (tTime === null) return;
    // find the index closest to that time
    let bestI = 0;
    let bestDiff = Infinity;
    ts.forEach((t, i) => {
      const d = Math.abs(t - tTime);
      if (d < bestDiff) { bestDiff = d; bestI = i; }
    });
    if (bestDiff > 3) return; // not in current view window
    const mx = px(ts[bestI]);
    const my = py(pl[bestI]);
    ctx.globalAlpha = 0.9;
    ctx.fillStyle = th.color;
    ctx.beginPath();
    ctx.arc(mx, my, 5, 0, Math.PI * 2);
    ctx.fill();
    ctx.strokeStyle = '#0d0d0d';
    ctx.lineWidth = 1.5;
    ctx.stroke();
    ctx.globalAlpha = 1;
  });
}

// ── Experiment controls ────────────────────────
function setButtons(running) {
  btnStart.disabled = running;
  btnStop.disabled  = !running;
  chartWrap.classList.toggle('recording', running);
}

function setRunningState(running) {
  if (running) {
    recLabel.textContent = 'EN EJECUCION';
    recStatus.classList.add('rec-recording');
    recStatus.classList.remove('rec-stopped');
  } else {
    recLabel.textContent = 'DETENIDO';
    recStatus.classList.add('rec-stopped');
    recStatus.classList.remove('rec-recording');
    elapsedTime.textContent = '';
  }
}

function startExp() {
  fetch('/start').then(r => {
    if (r.ok) {
      // Clear previous experiment data
      ts=[]; t1s=[]; t2s=[]; pr=[]; pl=[];
      threshTimes = { t30: null, t180: null, t200: null };
      snapshotCanvas = null;
      isRecording = true;
      setButtons(true);
      setRunningState(true);
      updateThreshUI();
    } else {
      setButtons(false);
    }
  }).catch(() => setButtons(false));
}

function stopExp() {
  fetch('/stop').then(r => {
    if (r.ok) {
      isRecording = false;
      freezeSnapshot();          // lock the chart as-is for PDF
      setButtons(false);
      setRunningState(false);
    } else {
      setButtons(true);
    }
  }).catch(() => setButtons(true));
}
window.startExp = startExp;
window.stopExp  = stopExp;

// ── SSE data push ──────────────────────────────
function push(d) {
  // While stopped, ignore incoming data — chart stays frozen at last frame
  if (!isRecording) return;
  ts.push(d.t);
  t1s.push(d.t1);
  t2s.push(d.t2);
  pr.push(d.prom);
  pl.push(d.placa);
  if (ts.length > MAX) { ts.shift(); t1s.shift(); t2s.shift(); pr.shift(); pl.shift(); }

  // "running" = experiment started and horno is valid and no fault
  const running = !d.falla && d.hornoValido;

  // Detect threshold crossings on cold face plate
  if (running) {
    if (threshTimes.t30  === null && d.placa >= 30)  threshTimes.t30  = d.t;
    if (threshTimes.t180 === null && d.placa >= 180) threshTimes.t180 = d.t;
    if (threshTimes.t200 === null && d.placa >= 200) threshTimes.t200 = d.t;
    updateThreshUI();
  }

  // Update metric cards
  document.getElementById('t1').textContent    = d.t1.toFixed(1);
  document.getElementById('t2').textContent    = d.t2.toFixed(1);
  document.getElementById('prom').textContent  = d.prom.toFixed(1);
  document.getElementById('delta').textContent = d.delta.toFixed(1);
  document.getElementById('placa').textContent = d.placa.toFixed(1);

  // Estado card
  const estadoEl = document.getElementById('estado');
  if (d.falla) {
    estadoEl.textContent = 'FALLA';
    estadoEl.style.color = '#f44';
  } else if (d.hornoValido) {
    estadoEl.textContent = 'VALIDO';
    estadoEl.style.color = '#4f4';
  } else {
    estadoEl.textContent = 'ESPERA';
    estadoEl.style.color = '#fa4';
  }

  // Recording status bar
  if (running) {
    recLabel.textContent = 'EN EJECUCION';
    recStatus.classList.add('rec-recording');
    recStatus.classList.remove('rec-stopped');
    elapsedTime.textContent = formatTime(d.t);
  } else {
    recLabel.textContent = d.falla ? 'FALLA' : 'DETENIDO';
    recStatus.classList.add('rec-stopped');
    recStatus.classList.remove('rec-recording');
    elapsedTime.textContent = '';
  }

  draw();
}

function formatTime(s) {
  const m = Math.floor(s / 60);
  const sec = (s % 60).toFixed(2);
  return m > 0 ? `${m}m ${sec}s` : `${sec}s`;
}

const es = new EventSource('/events');
es.onopen  = () => setConn(true);
es.onerror = () => setConn(false);
es.onmessage = e => {
  try { push(JSON.parse(e.data)); }
  catch (x) { console.error('SSE parse error', x); }
};

// ── CSV export ─────────────────────────────────
function normalizeFileName(name) {
  return name.trim().replace(/\s+/g, '_').replace(/[^a-zA-Z0-9_\-\.]/g, '').substring(0, 40) || 'experimento';
}

function dlCSV() {
  const name = normalizeFileName(expNameInput.value || 'experimento');
  fetch('/csv').then(r => r.text()).then(t => {
    const a = document.createElement('a');
    a.href = 'data:text/csv;charset=utf-8,' + encodeURIComponent(t);
    a.download = `${name}.csv`;
    a.click();
  });
}
window.dlCSV = dlCSV;

// ── PDF export ─────────────────────────────────
async function downloadPDF() {
  const name = normalizeFileName(expNameInput.value || 'experimento');
  const { jsPDF } = window.jspdf;
  const doc = new jsPDF({ orientation: 'landscape', unit: 'mm', format: 'a4' });

  const W = 297, H = 210;
  const margin = 15;

  // ── Background ───────────────────────────────
  doc.setFillColor(7, 17, 26);
  doc.rect(0, 0, W, H, 'F');

  // ── Header bar ───────────────────────────────
  doc.setFillColor(11, 28, 41);
  doc.rect(0, 0, W, 22, 'F');
  // Orange accent line under header
  doc.setFillColor(232, 130, 12);
  doc.rect(0, 22, W, 1.2, 'F');

  doc.setTextColor(235, 244, 251);
  doc.setFontSize(13);
  doc.setFont('helvetica', 'bold');
  doc.text('PRUEBA DE RESISTENCIA AL FUEGO', margin, 14);

  doc.setTextColor(74, 122, 155);
  doc.setFontSize(8.5);
  doc.setFont('helvetica', 'normal');
  const now = new Date().toLocaleString('es-CL');
  doc.text(`Experimento: ${name}    |    Generado: ${now}`, W - margin, 14, { align: 'right' });

  // ── Graph — taller now that table is gone ────
  // Graph takes most of the page; threshold table sits to the right
  const graphX = margin;
  const graphY = 27;
  const graphW = W - margin * 2 - 70;  // leave 70mm on right for thresh table
  const graphH = H - graphY - margin - 10;

  const pdfCanvas = snapshotCanvas || cv;
  const imgData = pdfCanvas.toDataURL('image/png');
  doc.addImage(imgData, 'PNG', graphX, graphY, graphW, graphH);

  // thin border around graph
  doc.setDrawColor(20, 78, 115);
  doc.setLineWidth(0.4);
  doc.rect(graphX, graphY, graphW, graphH);

  // ── Threshold times table (right column) ─────
  const tTableX = graphX + graphW + 8;
  const tTableW = W - tTableX - margin;
  const tTableY = graphY;

  // Section title
  doc.setFontSize(7.5);
  doc.setFont('helvetica', 'bold');
  doc.setTextColor(20, 78, 115);
  doc.text('TIEMPOS DE UMBRAL', tTableX, tTableY + 5);
  doc.setFontSize(7);
  doc.setFont('helvetica', 'normal');
  doc.setTextColor(74, 122, 155);
  doc.text('(Placa — Cara Fría)', tTableX, tTableY + 10);

  // Accent line under title
  doc.setFillColor(232, 130, 12);
  doc.rect(tTableX, tTableY + 12, tTableW, 0.6, 'F');

  function fmtPDF(s) {
    return s !== null ? s.toFixed(2) + ' s' : 'No alcanzado';
  }

  const thRows = [
    { label: '≥ 30 °C',  val: fmtPDF(threshTimes.t30),  reached: threshTimes.t30  !== null },
    { label: '≥ 180 °C', val: fmtPDF(threshTimes.t180), reached: threshTimes.t180 !== null },
    { label: '≥ 200 °C', val: fmtPDF(threshTimes.t200), reached: threshTimes.t200 !== null },
  ];

  const cellH = 14;
  thRows.forEach((row, ri) => {
    const cy = tTableY + 15 + ri * cellH;
    // Row background
    doc.setFillColor(ri % 2 === 0 ? 13 : 11, ri % 2 === 0 ? 34 : 28, ri % 2 === 0 ? 54 : 41);
    doc.rect(tTableX, cy, tTableW, cellH, 'F');

    // Temp label
    doc.setFontSize(8);
    doc.setFont('helvetica', 'bold');
    doc.setTextColor(235, 244, 251);
    doc.text(row.label, tTableX + 2, cy + 5.5);

    // Value
    doc.setFontSize(9.5);
    doc.setFont('helvetica', 'bold');
    if (row.reached) {
      doc.setTextColor(232, 130, 12);  // orange = reached
    } else {
      doc.setTextColor(46, 106, 138);  // muted blue = not reached
    }
    doc.text(row.val, tTableX + 2, cy + 11);
  });

  // Also add experiment duration below
  const durY = tTableY + 15 + thRows.length * cellH + 6;
  doc.setFillColor(20, 78, 115);
  doc.rect(tTableX, durY, tTableW, 0.5, 'F');
  doc.setFontSize(7.5);
  doc.setFont('helvetica', 'bold');
  doc.setTextColor(20, 78, 115);
  doc.text('DURACIÓN TOTAL', tTableX, durY + 6);
  doc.setFontSize(9.5);
  doc.setFont('helvetica', 'bold');
  doc.setTextColor(235, 244, 251);
  doc.text(ts.length ? formatTime(ts[ts.length - 1]) : '--', tTableX, durY + 13);

  // ── Footer ───────────────────────────────────
  doc.setFillColor(11, 28, 41);
  doc.rect(0, H - 8, W, 8, 'F');
  doc.setTextColor(20, 78, 115);
  doc.setFontSize(7);
  doc.setFont('helvetica', 'normal');
  doc.text('Reporte generado automaticamente · Hazeladd', W / 2, H - 2.5, { align: 'center' });

  doc.save(`${name}.pdf`);
}
window.downloadPDF = downloadPDF;

)rawliteral";