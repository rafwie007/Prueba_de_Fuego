#include "html.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Prueba de Fuego</title>
<link rel="stylesheet" href="/style.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/jspdf/2.5.1/jspdf.umd.min.js"></script>
</head>
<body>

<div class="page-wrap">

  <!-- ═══════════════ SIDEBAR ═══════════════ -->
  <aside class="sidebar">

    <div class="header">
      <h1>Prueba de Fuego<small>Monitor de Resistencia</small></h1>
      <div id="connBadge" class="badge badge-disconnected">Sin conexion</div>
    </div>

    <!-- Nombre experimento -->
    <div class="name-group">
      <label for="expName">Nombre del experimento</label>
      <input type="text" id="expName" placeholder="ej: muestra_01" value="experimento">
    </div>

    <!-- Controles principales -->
    <div>
      <div class="section-label">Control</div>
      <div class="btn-group">
        <button id="btnStart" onclick="startExp()">&#9654; START</button>
        <button id="btnStop"  onclick="stopExp()" disabled>&#9632; STOP</button>
      </div>
    </div>

    <!-- Exportar -->
    <div>
      <div class="section-label">Exportar</div>
      <div class="btn-group">
        <button id="btnCSV" onclick="dlCSV()">&#8595; CSV</button>
        <button id="btnPDF" onclick="downloadPDF()">&#8595; PDF</button>
      </div>
    </div>

    <!-- Estado de grabacion -->
    <div id="recStatus" class="rec-status rec-stopped">
      <span class="rec-dot"></span>
      <span id="recLabel">DETENIDO</span>
      <span id="elapsedTime" class="elapsed"></span>
    </div>

    <!-- Tarjetas de sensores -->
    <div>
      <div class="section-label">Mediciones en vivo</div>
      <div class="cards">
        <div class="card">
          <div class="label">T1 (&deg;C)</div>
          <div class="val" id="t1">&#8212;</div>
        </div>
        <div class="card">
          <div class="label">T2 (&deg;C)</div>
          <div class="val" id="t2">&#8212;</div>
        </div>
        <div class="card">
          <div class="label">Promedio (&deg;C)</div>
          <div class="val c-blue" id="prom">&#8212;</div>
        </div>
        <div class="card">
          <div class="label">Delta (&deg;C)</div>
          <div class="val c-yellow" id="delta">&#8212;</div>
        </div>
        <div class="card">
          <div class="label">Placa (&deg;C)</div>
          <div class="val c-orange" id="placa">&#8212;</div>
        </div>
        <div class="card">
          <div class="label">Estado Horno</div>
          <div class="val" id="estado">&#8212;</div>
        </div>
      </div>
    </div>

    <!-- Tiempos umbral -->
    <div class="thresh-panel">
      <div class="thresh-title">Tiempos de umbral (Placa)</div>
      <div class="thresh-row">
        <span class="thresh-label">&#9654; 30 &deg;C</span>
        <span class="thresh-val" id="thresh30">--</span>
      </div>
      <div class="thresh-row">
        <span class="thresh-label">&#9654; 180 &deg;C</span>
        <span class="thresh-val" id="thresh180">--</span>
      </div>
      <div class="thresh-row">
        <span class="thresh-label">&#9654; 200 &deg;C</span>
        <span class="thresh-val" id="thresh200">--</span>
      </div>
    </div>

  </aside>

  <!-- ═══════════════ MAIN AREA ══════════════ -->
  <main class="main-area">
    <div class="chart-wrap" id="chartWrap">
      <canvas id="cv"></canvas>
      <div class="chart-legend">
        <span class="leg-item"><span class="leg-line" style="background:#3ab8f0"></span>Prom Horno</span>
        <span class="leg-item"><span class="leg-line" style="background:#f09030"></span>Placa (Cara Fria)</span>
        <span class="leg-item"><span class="leg-line" style="background:#666"></span>T1</span>
        <span class="leg-item"><span class="leg-line" style="background:#333;border:1px solid #555"></span>T2</span>
        <span class="leg-item"><span class="leg-line" style="background:#f44;opacity:.6"></span>Umbral 30&deg;C</span>
        <span class="leg-item"><span class="leg-line" style="background:#f44"></span>Umbral 180&deg;C</span>
        <span class="leg-item"><span class="leg-line" style="background:#f88"></span>Umbral 200&deg;C</span>
      </div>
    </div>
  </main>

</div>

<script src="/app.js"></script>
</body>
</html>
)rawliteral";