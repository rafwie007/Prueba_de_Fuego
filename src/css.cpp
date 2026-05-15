#include "css.h"

const char STYLE_CSS[] PROGMEM = R"rawliteral(
*{box-sizing:border-box;margin:0;padding:0}
html,body{height:100%}
body{font-family:'Segoe UI',system-ui,sans-serif;background:#0a0a0a;color:#e8e8e8;padding:0;overflow-x:hidden}

/* ── LAYOUT ─────────────────────────────────── */
.page-wrap{display:grid;grid-template-columns:320px 1fr;grid-template-rows:auto 1fr;min-height:100vh;gap:0}
.sidebar{grid-column:1;grid-row:1/3;background:#0e0e0e;border-right:1px solid #1e1e1e;padding:20px 16px;display:flex;flex-direction:column;gap:14px;overflow-y:auto}
.main-area{grid-column:2;grid-row:1/3;padding:20px;display:flex;flex-direction:column;gap:14px;min-width:0}

@media(max-width:900px){
  .page-wrap{grid-template-columns:1fr;grid-template-rows:auto}
  .sidebar{grid-column:1;grid-row:auto;border-right:none;border-bottom:1px solid #1e1e1e}
  .main-area{grid-column:1;grid-row:auto}
}

/* ── HEADER ──────────────────────────────────── */
.header{display:flex;align-items:center;gap:10px;flex-wrap:wrap;padding-bottom:14px;border-bottom:1px solid #1e1e1e}
h1{font-size:1.15rem;font-weight:700;color:#f90;letter-spacing:.02em;line-height:1.2}
h1 small{display:block;font-size:.65rem;font-weight:400;color:#555;letter-spacing:.08em;text-transform:uppercase;margin-top:2px}

/* ── BADGE ───────────────────────────────────── */
.badge{padding:3px 10px;border-radius:20px;font-size:.7rem;font-weight:700;letter-spacing:.04em;margin-left:auto}
.badge-connected{background:#0a2a0a;color:#4f4;border:1px solid #1e5a1e}
.badge-disconnected{background:#2a0a0a;color:#f55;border:1px solid #5a1e1e}

/* ── SECTION LABELS ──────────────────────────── */
.section-label{font-size:.65rem;font-weight:700;text-transform:uppercase;letter-spacing:.1em;color:#444;margin-bottom:6px}

/* ── EXPERIMENT NAME ─────────────────────────── */
.name-group{display:flex;flex-direction:column;gap:5px}
.name-group label{font-size:.68rem;color:#555;text-transform:uppercase;letter-spacing:.07em}
input[type=text],input[type=password]{
  background:#141414;border:1px solid #252525;border-radius:6px;
  color:#ddd;padding:8px 11px;font-size:.88rem;outline:none;width:100%;
  transition:border-color .2s,background .2s
}
input[type=text]:focus,input[type=password]:focus{border-color:#f90;background:#191919}

/* ── BUTTONS ─────────────────────────────────── */
.btn-group{display:flex;gap:8px;flex-wrap:wrap}
button{
  border:none;padding:9px 16px;border-radius:7px;cursor:pointer;
  font-weight:700;font-size:.82rem;letter-spacing:.03em;
  transition:opacity .15s,background .15s,transform .1s
}
button:disabled{opacity:.28;cursor:not-allowed;transform:none!important}
button:active:not(:disabled){transform:scale(.97)}

#btnStart{background:#f90;color:#111;flex:1}
#btnStart:hover:not(:disabled){background:#ffab33}
#btnStop{background:#c03030;color:#fff;flex:1}
#btnStop:hover:not(:disabled){background:#e04040}
#btnCSV{background:#161616;color:#888;border:1px solid #252525;flex:1}
#btnCSV:hover:not(:disabled){background:#202020;color:#ccc}
#btnPDF{background:#161616;color:#888;border:1px solid #252525;flex:1}
#btnPDF:hover:not(:disabled){background:#202020;color:#ccc}

/* ── WIFI PANEL ──────────────────────────────── */
.wifi-panel{background:#0c0c0c;border:1px solid #1e1e1e;border-radius:8px;padding:12px}
.wifi-title{font-size:.7rem;font-weight:700;text-transform:uppercase;letter-spacing:.09em;color:#555;margin-bottom:10px}
.wifi-row{display:flex;justify-content:space-between;align-items:center;font-size:.82rem;margin-bottom:10px;gap:8px}
.wifi-row span:first-child{color:#777}
.wifi-config{display:flex;flex-direction:column;gap:7px}
.wifi-config button{background:#1a3a1a;color:#aaa;border:1px solid #2a5a2a;width:100%}
.wifi-config button:hover:not(:disabled){background:#254a25;color:#ccc}
.wifi-msg{font-size:.75rem;color:#888;margin-top:6px;min-height:16px}

/* ── REC STATUS ──────────────────────────────── */
@keyframes blink{0%,100%{opacity:1;transform:scale(1)}50%{opacity:.2;transform:scale(1.7)}}
.rec-status{
  display:flex;align-items:center;gap:10px;padding:10px 14px;
  border-radius:8px;font-weight:700;font-size:.88rem;
  transition:background .3s,border-color .3s,color .3s
}
.rec-stopped{background:#121212;color:#3a3a3a;border:1px solid #1e1e1e}
.rec-recording{background:#1e0000;color:#f44;border:1px solid #4a0000}
.rec-dot{width:11px;height:11px;border-radius:50%;background:#252525;flex-shrink:0;transition:background .3s}
.rec-recording .rec-dot{background:#f33;animation:blink 1s ease-in-out infinite}
.elapsed{font-family:'Courier New',monospace;font-size:1rem;letter-spacing:.06em}
.data-points{margin-left:auto;font-size:.72rem;color:#3a3a3a;font-weight:400}

/* ── METRIC CARDS ────────────────────────────── */
.cards{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
@media(max-width:600px){.cards{grid-template-columns:repeat(2,1fr)}}
.card{
  background:#0e0e0e;border-radius:8px;padding:10px 14px;
  border:1px solid #1a1a1a;transition:border-color .3s
}
.card .label{font-size:.62rem;color:#444;text-transform:uppercase;letter-spacing:.08em;margin-bottom:5px}
.card .val{font-size:1.65rem;font-weight:700;font-family:'Courier New',monospace;color:#ccc;transition:color .3s}

/* intersection time badges inside cards */
.card .time-badge{font-size:.65rem;color:#555;font-family:monospace;margin-top:2px}

.c-blue{color:#3ab8f0}
.c-orange{color:#f09030}
.c-yellow{color:#e8d030}
.c-green{color:#40d040}
.c-red{color:#f04040}

/* ── THRESHOLD TIMES PANEL ───────────────────── */
.thresh-panel{background:#0c0c0c;border:1px solid #1e1e1e;border-radius:8px;padding:12px}
.thresh-title{font-size:.68rem;font-weight:700;text-transform:uppercase;letter-spacing:.09em;color:#555;margin-bottom:9px}
.thresh-row{display:flex;justify-content:space-between;align-items:center;padding:5px 0;border-bottom:1px solid #161616;font-size:.82rem}
.thresh-row:last-child{border-bottom:none}
.thresh-label{color:#666}
.thresh-val{font-family:'Courier New',monospace;font-weight:700;color:#888}
.thresh-val.reached{color:#4f4}

/* ── CHART ───────────────────────────────────── */
@keyframes chartGlow{0%,100%{box-shadow:0 0 12px #f4420a33}50%{box-shadow:0 0 32px #f4420d66}}
.chart-wrap{
  border-radius:10px;overflow:hidden;
  border:1px solid #1e1e1e;transition:border-color .4s,box-shadow .4s;
  flex:1;min-height:300px;display:flex;flex-direction:column
}
.chart-wrap.recording{border-color:#5a1111;animation:chartGlow 2s ease-in-out infinite}
canvas{background:#0d0d0d;width:100%;flex:1;display:block;min-height:280px}
.chart-legend{
  display:flex;flex-wrap:wrap;gap:14px;padding:8px 14px;
  background:#0a0a0a;border-top:1px solid #161616
}
.leg-item{display:flex;align-items:center;gap:6px;font-size:.7rem;color:#555}
.leg-line{display:inline-block;width:18px;height:3px;border-radius:2px}
)rawliteral";