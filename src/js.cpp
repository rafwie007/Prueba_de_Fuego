#include "js.h"

const char APP_JS[] PROGMEM = R"rawliteral(

const MAX = 600;
let ts=[], t1s=[], t2s=[], pr=[], pl=[];

// Threshold crossing times (in seconds from experiment start)
let threshTimes = { t30: null, t180: null, t200: null };

const cv       = document.getElementById('cv');
const ctx      = cv.getContext('2d');
const connBadge    = document.getElementById('connBadge');
const wifiStaStatus = document.getElementById('wifiStaStatus');
const wifiStaSsid   = document.getElementById('wifiStaSsid');
const ssidInput  = document.getElementById('ssidInput');
const passInput  = document.getElementById('passInput');
const btnSaveWifi = document.getElementById('btnSaveWifi');
const wifiMsg    = document.getElementById('wifiMsg');
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
function updateWifiUI(info) {
  wifiStaStatus.textContent  = info.staConnected ? 'Conectado' : 'Desconectado';
  wifiStaStatus.style.color  = info.staConnected ? '#4f4' : '#f44';
  wifiStaSsid.textContent    = info.staConnected ? (info.staSsid || '---') : '---';
  if (info.configuredSsid) ssidInput.value = info.configuredSsid;
}

function loadStatus() {
  fetch('/status').then(r => r.json()).then(info => {
    const running = !!info.running;
    setButtons(running);
    setRunningState(running);
  }).catch(() => { setButtons(false); setRunningState(false); });
}

function loadWifiStatus() {
  fetch('/wifi').then(r => r.json()).then(info => {
    updateWifiUI(info);
    setConn(info.staConnected);
  }).catch(() => {
    wifiStaStatus.textContent = 'No disponible';
    wifiStaStatus.style.color = '#f44';
    setConn(false);
  });
}

function saveWifi() {
  const ssid = ssidInput.value.trim();
  const pass = passInput.value;
  if (!ssid) {
    wifiMsg.textContent = 'SSID requerido';
    wifiMsg.style.color = '#f44';
    return;
  }
  btnSaveWifi.disabled = true;
  wifiMsg.textContent = 'Guardando...';
  wifiMsg.style.color = '#ccc';
  fetch(`/wifi-config?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`)
    .then(r => r.json())
    .then(result => {
      if (result.ok) {
        wifiMsg.textContent = 'Guardado. Reintentando conexion...';
        wifiMsg.style.color = '#4f4';
        setTimeout(loadWifiStatus, 3000);
      } else {
        wifiMsg.textContent = 'Error: ' + (result.error || 'desconocido');
        wifiMsg.style.color = '#f44';
      }
    }).catch(() => {
      wifiMsg.textContent = 'No se pudo guardar la configuracion';
      wifiMsg.style.color = '#f44';
    }).finally(() => btnSaveWifi.disabled = false);
}
window.saveWifi = saveWifi;

loadStatus();
loadWifiStatus();
setInterval(loadWifiStatus, 5000);

// ── Threshold UI update ───────────────────────
function updateThreshUI() {
  function fmt(s) {
    if (s === null) return '--';
    const m = Math.floor(s / 60);
    const sec = (s % 60).toFixed(1);
    return m > 0 ? `${m}m ${sec}s` : `${sec}s`;
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

  // Dynamic Y range
  const allTemps = [...pr, ...pl, ...t1s, ...t2s];
  const rawMin = Math.min(...allTemps);
  const rawMax = Math.max(...allTemps);
  // Include threshold lines in range
  const threshVals = [30, 180, 200];
  const yMin = Math.floor(Math.min(rawMin, ...threshVals) / 10) * 10 - 10;
  const yMax = Math.ceil(Math.max(rawMax, ...threshVals) / 10) * 10 + 15;

  if (!isFinite(yMin) || !isFinite(yMax)) return;

  const x0 = ts[0], xn = ts[ts.length - 1];
  const xRange = (xn - x0) || 1;
  const yRange = (yMax - yMin) || 1;

  const px = t  => left + ((t  - x0) / xRange) * (W - left - right);
  const py = v  => H - bottom - ((v - yMin) / yRange) * (H - top - bottom);

  // ── Grid & Y axis labels ──────────────────────
  const ySteps = 6;
  ctx.strokeStyle = '#1e1e1e';
  ctx.lineWidth = 1;
  ctx.font = '10px monospace';
  ctx.fillStyle = '#555';

  for (let i = 0; i <= ySteps; i++) {
    const v = yMin + i * (yMax - yMin) / ySteps;
    const y = py(v);
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(W - right, y);
    ctx.stroke();
    ctx.fillText(v.toFixed(0) + '\u00b0', 3, y + 3);
  }

  // X axis labels (time in minutes)
  const xSteps = 6;
  for (let i = 0; i <= xSteps; i++) {
    const t = x0 + i * xRange / xSteps;
    const x = px(t);
    ctx.strokeStyle = '#1e1e1e';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(x, top);
    ctx.lineTo(x, H - bottom);
    ctx.stroke();
    ctx.fillStyle = '#555';
    const mins = (t / 60).toFixed(1);
    ctx.fillText(mins + 'm', x - 12, H - bottom + 14);
  }

  // ── Axes ──────────────────────────────────────
  ctx.strokeStyle = '#333';
  ctx.lineWidth = 2;
  ctx.beginPath(); ctx.moveTo(left, top); ctx.lineTo(left, H - bottom); ctx.stroke();
  ctx.beginPath(); ctx.moveTo(left, H - bottom); ctx.lineTo(W - right, H - bottom); ctx.stroke();

  // ── Axis titles ───────────────────────────────
  ctx.save();
  ctx.fillStyle = '#444';
  ctx.font = '11px sans-serif';
  ctx.translate(12, (H - bottom + top) / 2);
  ctx.rotate(-Math.PI / 2);
  ctx.fillText('Temperatura (\u00b0C)', 0, 0);
  ctx.restore();
  ctx.fillStyle = '#444';
  ctx.font = '11px sans-serif';
  ctx.fillText('Tiempo (minutos)', left + (W - left - right) / 2 - 45, H - 4);

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
    if (r.ok) { setButtons(true);  setRunningState(true);  }
    else       { setButtons(false); }
  }).catch(() => setButtons(false));
}

function stopExp() {
  fetch('/stop').then(r => {
    if (r.ok) { setButtons(false); setRunningState(false); }
    else       { setButtons(true);  }
  }).catch(() => setButtons(true));
}
window.startExp = startExp;
window.stopExp  = stopExp;

// ── SSE data push ──────────────────────────────
function push(d) {
  ts.push(d.t); t1s.push(d.t1); t2s.push(d.t2);
  pr.push(d.prom); pl.push(d.placa);
  if (ts.length > MAX) { ts.shift(); t1s.shift(); t2s.shift(); pr.shift(); pl.shift(); }

  // Detect threshold crossings (only while running, first crossing)
  if (d.running) {
    if (threshTimes.t30  === null && d.placa >= 30)  threshTimes.t30  = d.t;
    if (threshTimes.t180 === null && d.placa >= 180) threshTimes.t180 = d.t;
    if (threshTimes.t200 === null && d.placa >= 200) threshTimes.t200 = d.t;
    updateThreshUI();
  }

  // Update cards
  document.getElementById('t1').textContent    = d.t1.toFixed(1);
  document.getElementById('t2').textContent    = d.t2.toFixed(1);
  document.getElementById('prom').textContent  = d.prom.toFixed(1);
  document.getElementById('delta').textContent = d.delta.toFixed(1);
  document.getElementById('placa').textContent = d.placa.toFixed(1);

  if (d.running) {
    recLabel.textContent = 'EN EJECUCION';
    recStatus.classList.add('rec-recording');
    recStatus.classList.remove('rec-stopped');
    elapsedTime.textContent = formatTime(d.t);
  } else {
    recLabel.textContent = 'DETENIDO';
    recStatus.classList.add('rec-stopped');
    recStatus.classList.remove('rec-recording');
    elapsedTime.textContent = '';
  }

  draw();
}

function formatTime(s) {
  const m = Math.floor(s / 60);
  const sec = (s % 60).toFixed(1);
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

  // Background
  doc.setFillColor(13, 13, 13);
  doc.rect(0, 0, W, H, 'F');

  // Header bar
  doc.setFillColor(20, 20, 20);
  doc.rect(0, 0, W, 22, 'F');
  doc.setTextColor(255, 153, 0);
  doc.setFontSize(14);
  doc.setFont('helvetica', 'bold');
  doc.text('PRUEBA DE RESISTENCIA AL FUEGO', margin, 14);
  doc.setTextColor(100, 100, 100);
  doc.setFontSize(9);
  doc.setFont('helvetica', 'normal');
  const now = new Date().toLocaleString('es-CL');
  doc.text(`Experimento: ${name}    |    Generado: ${now}`, W - margin, 14, { align: 'right' });

  // Graph image
  const imgData = cv.toDataURL('image/png');
  const graphY = 26, graphH = 100;
  const graphW = W - margin * 2;
  doc.addImage(imgData, 'PNG', margin, graphY, graphW, graphH);

  // Metrics table
  const tableY = graphY + graphH + 8;
  const cols = ['Sensor', 'Ultimo valor'];
  const rows = [
    ['T1 (Horno)', ts.length ? t1s[t1s.length - 1].toFixed(1) + ' \u00b0C' : '--'],
    ['T2 (Horno)', ts.length ? t2s[t2s.length - 1].toFixed(1) + ' \u00b0C' : '--'],
    ['Promedio Horno', ts.length ? pr[pr.length - 1].toFixed(1) + ' \u00b0C' : '--'],
    ['Placa (Cara Fria)', ts.length ? pl[pl.length - 1].toFixed(1) + ' \u00b0C' : '--'],
    ['Duracion', ts.length ? formatTime(ts[ts.length - 1]) : '--'],
  ];

  doc.setFontSize(9);
  doc.setTextColor(80, 80, 80);
  doc.text('MEDICIONES FINALES', margin, tableY);

  const cellW = 55, cellH = 8, tableX = margin;
  rows.forEach((row, ri) => {
    const y = tableY + 5 + ri * cellH;
    doc.setFillColor(ri % 2 === 0 ? 18 : 14, ri % 2 === 0 ? 18 : 14, ri % 2 === 0 ? 18 : 14);
    doc.rect(tableX, y, cellW * 2, cellH, 'F');
    doc.setTextColor(160, 160, 160);
    doc.text(row[0], tableX + 3, y + 5.5);
    doc.setTextColor(220, 220, 220);
    doc.text(row[1], tableX + cellW + 3, y + 5.5);
  });

  // Threshold times table
  const tTableX = margin + cellW * 2 + 12;
  doc.setFontSize(9);
  doc.setTextColor(80, 80, 80);
  doc.text('TIEMPOS DE UMBRAL (PLACA)', tTableX, tableY);

  const thRows = [
    ['Alcanza 30 \u00b0C',  threshTimes.t30  !== null ? formatTime(threshTimes.t30)  : 'No alcanzado'],
    ['Alcanza 180 \u00b0C', threshTimes.t180 !== null ? formatTime(threshTimes.t180) : 'No alcanzado'],
    ['Alcanza 200 \u00b0C', threshTimes.t200 !== null ? formatTime(threshTimes.t200) : 'No alcanzado'],
  ];
  thRows.forEach((row, ri) => {
    const y = tableY + 5 + ri * cellH;
    doc.setFillColor(ri % 2 === 0 ? 18 : 14, ri % 2 === 0 ? 18 : 14, ri % 2 === 0 ? 18 : 14);
    doc.rect(tTableX, y, cellW * 2, cellH, 'F');
    doc.setTextColor(160, 160, 160);
    doc.text(row[0], tTableX + 3, y + 5.5);
    doc.setTextColor(row[1] !== 'No alcanzado' ? 80 : 180, row[1] !== 'No alcanzado' ? 200 : 180, row[1] !== 'No alcanzado' ? 80 : 180);
    doc.text(row[1], tTableX + cellW + 3, y + 5.5);
  });

  // Footer
  doc.setFillColor(15, 15, 15);
  doc.rect(0, H - 8, W, 8, 'F');
  doc.setTextColor(50, 50, 50);
  doc.setFontSize(7);
  doc.text('Reporte generado automaticamente', W / 2, H - 2.5, { align: 'center' });

  doc.save(`${name}.pdf`);
}
window.downloadPDF = downloadPDF;

)rawliteral";