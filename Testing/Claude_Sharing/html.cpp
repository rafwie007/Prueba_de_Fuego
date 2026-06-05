#include "html.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Prueba de Resistencia al Fuego</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@400;600;700;800&family=Share+Tech+Mono&display=swap" rel="stylesheet">
<link rel="stylesheet" href="/style.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/jspdf/2.5.1/jspdf.umd.min.js"></script>
</head>
<body>

<div class="page-wrap">

  <!-- ═══════════════ SIDEBAR ═══════════════ -->
  <aside class="sidebar">

    <!-- Logo + title block -->
    <div class="brand">
      <div class="brand-logo">
        <img src="data:image/png;base64,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" class="logo-img" alt="Hazeladd logo">
        <div class="brand-text">
          <span class="brand-name">HAZEL<span class="brand-accent">ADD</span></span>
        </div>
      </div>
      <div class="title-block">
        <h1><span class="title-small">PRUEBA DE</span><br>RESISTENCIA<br><span class="title-accent">AL FUEGO</span></h1>
      </div>
      <div id="connBadge" class="badge badge-disconnected">● Sin conexión</div>
    </div>

    <div class="divider"></div>

    <!-- Nombre experimento -->
    <div class="name-group">
      <label for="expName">ID del experimento</label>
      <input type="text" id="expName" placeholder="ej: muestra_01" value="experimento">
    </div>

    <!-- Controles principales -->
    <div class="section-block">
      <div class="section-label">■ Control</div>
      <div class="btn-group">
        <button id="btnStart" onclick="startExp()">▶ START</button>
        <button id="btnStop"  onclick="stopExp()" disabled>■ STOP</button>
      </div>
    </div>

    <!-- Estado de grabacion -->
    <div id="recStatus" class="rec-status rec-stopped">
      <span class="rec-dot"></span>
      <span id="recLabel">DETENIDO</span>
      <span id="elapsedTime" class="elapsed"></span>
    </div>

    <!-- Exportar -->
    <div class="section-block">
      <div class="section-label">■ Exportar</div>
      <div class="btn-group">
        <button id="btnCSV" onclick="dlCSV()">↓ CSV</button>
        <button id="btnPDF" onclick="downloadPDF()">↓ PDF</button>
      </div>
    </div>

    <div class="divider"></div>

    <!-- Tarjetas de sensores — 2 columns -->
    <div class="section-block">
      <div class="section-label">■ Mediciones en vivo</div>
      <div class="cards">
        <div class="card">
          <div class="label">T1 (°C)</div>
          <div class="val" id="t1">—</div>
        </div>
        <div class="card">
          <div class="label">T2 (°C)</div>
          <div class="val" id="t2">—</div>
        </div>
        <div class="card">
          <div class="label">Promedio (°C)</div>
          <div class="val c-blue" id="prom">—</div>
        </div>
        <div class="card">
          <div class="label">Delta (°C)</div>
          <div class="val c-yellow" id="delta">—</div>
        </div>
        <div class="card">
          <div class="label">Placa (°C)</div>
          <div class="val c-orange" id="placa">—</div>
        </div>
        <div class="card">
          <div class="label">Estado</div>
          <div class="val" id="estado">—</div>
        </div>
      </div>
    </div>

    <!-- Tiempos umbral -->
    <div class="thresh-panel">
      <div class="thresh-title">▶ Tiempos de umbral — Placa</div>
      <div class="thresh-row">
        <span class="thresh-label">≥ 30 °C</span>
        <span class="thresh-val" id="thresh30">--</span>
      </div>
      <div class="thresh-row">
        <span class="thresh-label">≥ 180 °C</span>
        <span class="thresh-val" id="thresh180">--</span>
      </div>
      <div class="thresh-row">
        <span class="thresh-label">≥ 200 °C</span>
        <span class="thresh-val" id="thresh200">--</span>
      </div>
    </div>

  </aside>

  <!-- ═══════════════ MAIN AREA ══════════════ -->
  <main class="main-area">
    <div class="chart-wrap" id="chartWrap">
      <div class="chart-header">
        <span class="chart-title">TEMPERATURA vs TIEMPO</span>
        <div class="chart-legend">
          <span class="leg-item"><span class="leg-line" style="background:#4ab8f0"></span>Prom Horno</span>
          <span class="leg-item"><span class="leg-line" style="background:#e8820c"></span>Placa (Cara Fría)</span>
          <span class="leg-item"><span class="leg-line" style="background:#3a5a6a"></span>T1</span>
          <span class="leg-item"><span class="leg-line" style="background:#2a4050"></span>T2</span>
          <span class="leg-item"><span class="leg-line leg-dash" style="border-color:#f44"></span>Umbrales</span>
        </div>
      </div>
      <canvas id="cv"></canvas>
    </div>
  </main>

</div>

<script src="/app.js"></script>
</body>
</html>
)rawliteral";