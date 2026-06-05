#include "css.h"

const char STYLE_CSS[] PROGMEM = R"rawliteral(
@import url('https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@400;600;700;800&family=Share+Tech+Mono&display=swap');

*{box-sizing:border-box;margin:0;padding:0}
html,body{height:100%}
body{
  font-family:'Barlow Condensed','Segoe UI',system-ui,sans-serif;
  background:#020c14;
  color:#b8ccd8;
  overflow-x:hidden;
}

/* ── SCANLINE TEXTURE overlay ────────────────── */
body::before{
  content:'';position:fixed;inset:0;pointer-events:none;z-index:9000;
  background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,.08) 2px,rgba(0,0,0,.08) 4px);
}

/* ── LAYOUT ─────────────────────────────────── */
.page-wrap{display:grid;grid-template-columns:290px 1fr;min-height:100vh;gap:0}
.sidebar{
  background:#030f1a;
  border-right:1px solid #0e3050;
  padding:0;
  display:flex;flex-direction:column;
  overflow-y:auto;
  position:relative;
}
/* subtle vertical accent stripe */
.sidebar::after{
  content:'';position:absolute;top:0;right:-1px;width:2px;height:100%;
  background:linear-gradient(180deg,#144e73 0%,#e8820c 50%,#144e73 100%);
  opacity:.6;
}
.main-area{padding:12px;display:flex;flex-direction:column;gap:0;min-width:0;background:#020c14;overflow:hidden}

/* ── BRAND BLOCK ─────────────────────────────── */
.brand{
  padding:16px 16px 12px;
  background:#030f1a;
  border-bottom:1px solid #0e3050;
}
.brand-logo{display:flex;align-items:center;gap:10px;margin-bottom:12px}
.logo-img{width:64px;height:64px;flex-shrink:0;object-fit:contain;mix-blend-mode:screen}
.brand-text{display:flex;flex-direction:column;gap:1px}
.brand-name{
  font-family:'Barlow Condensed',sans-serif;
  font-size:2.5rem;font-weight:800;
  color:#ebf4fb;letter-spacing:.06em;line-height:1;
}
.brand-accent{color:#e8820c}

.title-block{margin-bottom:10px}
h1{
  font-family:'Barlow Condensed',sans-serif;
  font-weight:800;
  color:#ebf4fb;
  letter-spacing:.04em;
  line-height:1.05;
  text-transform:uppercase;
}
/* "PRUEBA DE" — smaller fixed label */
.title-small{
  font-size:1.1rem;
  font-weight:700;
  color:#4a7a9b;
  letter-spacing:.1em;
}
/* "RESISTENCIA" + "AL FUEGO" — big */
h1{font-size:2.1rem}
.title-accent{color:#e8820c}

/* ── BADGE ───────────────────────────────────── */
.badge{
  display:inline-block;
  padding:3px 10px;border-radius:3px;
  font-family:'Share Tech Mono',monospace;
  font-size:.68rem;font-weight:400;letter-spacing:.06em;
}
.badge-connected{background:#071e10;color:#3dd68c;border:1px solid #1a5a38;box-shadow:0 0 8px #3dd68c22}
.badge-disconnected{background:#1a0505;color:#f55;border:1px solid #5a1010}

/* ── DIVIDER ─────────────────────────────────── */
.divider{height:1px;background:linear-gradient(90deg,transparent,#0e3050 30%,#0e3050 70%,transparent);margin:0}

/* ── SECTION BLOCKS ──────────────────────────── */
.section-block{padding:12px 16px}
.section-label{
  font-size:.68rem;font-weight:700;text-transform:uppercase;
  letter-spacing:.14em;color:#144e73;margin-bottom:8px;
  font-family:'Barlow Condensed',sans-serif;
}

/* ── EXPERIMENT NAME ─────────────────────────── */
.name-group{display:flex;flex-direction:column;gap:5px;padding:12px 16px}
.name-group label{font-size:.65rem;color:#144e73;text-transform:uppercase;letter-spacing:.1em;font-weight:600}
input[type=text],input[type=password]{
  background:#061422;border:1px solid #0e3050;border-radius:3px;
  color:#ebf4fb;padding:7px 10px;
  font-family:'Share Tech Mono',monospace;
  font-size:.82rem;outline:none;width:100%;
  transition:border-color .2s,box-shadow .2s;
}
input[type=text]:focus,input[type=password]:focus{
  border-color:#e8820c;
  box-shadow:0 0 0 2px #e8820c1a;
}

/* ── BUTTONS ─────────────────────────────────── */
.btn-group{display:flex;gap:6px}
button{
  border:none;padding:8px 12px;border-radius:3px;cursor:pointer;
  font-family:'Barlow Condensed',sans-serif;
  font-weight:700;font-size:.9rem;letter-spacing:.08em;text-transform:uppercase;
  transition:background .15s,box-shadow .15s,transform .1s;
  flex:1;
}
button:disabled{opacity:.22;cursor:not-allowed;transform:none!important}
button:active:not(:disabled){transform:scale(.97)}

#btnStart{background:#e8820c;color:#fff;box-shadow:0 0 14px #e8820c33}
#btnStart:hover:not(:disabled){background:#f59520;box-shadow:0 0 20px #e8820c55}
#btnStop{background:transparent;color:#4a7a9b;border:1px solid #144e73}
#btnStop:hover:not(:disabled){background:#0d2236;color:#ebf4fb;border-color:#4ab8f0}
#btnCSV{background:transparent;color:#4a7a9b;border:1px solid #0e3050;font-size:.82rem}
#btnCSV:hover:not(:disabled){border-color:#144e73;color:#ebf4fb}
#btnPDF{background:transparent;color:#4a7a9b;border:1px solid #0e3050;font-size:.82rem}
#btnPDF:hover:not(:disabled){border-color:#144e73;color:#ebf4fb}

/* ── REC STATUS ──────────────────────────────── */
@keyframes blink{0%,100%{opacity:1;box-shadow:0 0 6px currentColor}50%{opacity:.15;box-shadow:none}}
@keyframes recglow{0%,100%{box-shadow:inset 0 0 20px #e8820c0a,0 0 0 #e8820c00}50%{box-shadow:inset 0 0 20px #e8820c18,0 0 8px #e8820c22}}
.rec-status{
  margin:0 16px 0;
  display:flex;align-items:center;gap:8px;padding:8px 12px;
  border-radius:3px;font-weight:700;font-size:.85rem;letter-spacing:.06em;
  transition:background .3s,border-color .3s,color .3s;
  font-family:'Barlow Condensed',sans-serif;
}
.rec-stopped{background:#061422;color:#1e4060;border:1px solid #0e3050}
.rec-recording{background:#100800;color:#e8820c;border:1px solid #3a2000;animation:recglow 2s ease-in-out infinite}
.rec-dot{width:8px;height:8px;border-radius:50%;background:#1e4060;flex-shrink:0;transition:background .3s}
.rec-recording .rec-dot{background:#e8820c;animation:blink 1s ease-in-out infinite}
.elapsed{font-family:'Share Tech Mono',monospace;font-size:.95rem;margin-left:auto;letter-spacing:.04em}

/* ── METRIC CARDS — 2 columns ────────────────── */
.cards{display:grid;grid-template-columns:1fr 1fr;gap:6px}
.card{
  background:#061422;
  border-radius:3px;
  padding:8px 10px;
  border:1px solid #0e3050;
  border-left:2px solid #144e73;
  transition:border-color .3s,border-left-color .3s;
  position:relative;overflow:hidden;
}
.card::before{
  content:'';position:absolute;top:0;left:0;right:0;height:1px;
  background:linear-gradient(90deg,#144e73,transparent);
}
.card .label{
  font-size:.6rem;color:#144e73;text-transform:uppercase;
  letter-spacing:.1em;margin-bottom:3px;font-weight:700;
  font-family:'Barlow Condensed',sans-serif;
}
.card .val{
  font-size:1.55rem;font-weight:700;
  font-family:'Share Tech Mono',monospace;
  color:#ebf4fb;
  transition:color .3s;
  line-height:1.1;
}

.c-blue{color:#4ab8f0}
.c-orange{color:#e8820c}
.c-yellow{color:#f0c030}
.c-green{color:#3dd68c}
.c-red{color:#f04040}

/* ── THRESHOLD PANEL ─────────────────────────── */
.thresh-panel{
  margin:0 16px 16px;
  background:#061422;
  border:1px solid #0e3050;
  border-radius:3px;
  overflow:hidden;
}
.thresh-title{
  font-size:.68rem;font-weight:700;text-transform:uppercase;
  letter-spacing:.1em;color:#e8820c;
  padding:7px 12px;
  background:#0a1e30;
  border-bottom:1px solid #0e3050;
  font-family:'Barlow Condensed',sans-serif;
}
.thresh-row{
  display:flex;justify-content:space-between;align-items:center;
  padding:6px 12px;border-bottom:1px solid #0a1e2a;font-size:.82rem;
}
.thresh-row:last-child{border-bottom:none}
.thresh-label{
  color:#4a7a9b;font-family:'Barlow Condensed',sans-serif;
  font-size:.85rem;font-weight:600;letter-spacing:.06em;
}
.thresh-val{
  font-family:'Share Tech Mono',monospace;font-weight:400;
  color:#1e4060;font-size:.8rem;
  transition:color .4s;
}
.thresh-val.reached{color:#e8820c;text-shadow:0 0 8px #e8820c44}

/* ── CHART AREA ──────────────────────────────── */
@keyframes chartGlow{0%,100%{box-shadow:0 0 20px #e8820c11}50%{box-shadow:0 0 40px #e8820c33}}
.chart-wrap{
  border:1px solid #0e3050;border-radius:4px;overflow:hidden;
  transition:border-color .4s,box-shadow .4s;
  display:flex;flex-direction:column;
  height:calc(100vh - 32px);
}
.chart-wrap.recording{border-color:#e8820c55;animation:chartGlow 2.5s ease-in-out infinite}

.chart-header{
  display:flex;align-items:center;flex-wrap:wrap;gap:12px;
  padding:8px 14px;
  background:#030f1a;
  border-bottom:1px solid #0e3050;
}
.chart-title{
  font-family:'Barlow Condensed',sans-serif;
  font-size:.75rem;font-weight:700;
  letter-spacing:.14em;color:#144e73;
  text-transform:uppercase;
  white-space:nowrap;
}
canvas{background:#020c14;width:100%;flex:1;display:block;min-height:200px}

/* ── CHART LEGEND ────────────────────────────── */
.chart-legend{display:flex;flex-wrap:wrap;gap:12px;margin-left:auto}
.leg-item{display:flex;align-items:center;gap:5px;font-size:.68rem;color:#2e5a7a;font-family:'Barlow Condensed',sans-serif;letter-spacing:.06em;font-weight:600}
.leg-line{display:inline-block;width:16px;height:2px;border-radius:1px}
.leg-dash{width:16px;height:0;border-top:2px dashed;background:transparent!important}

/* ── RESPONSIVE ──────────────────────────────── */
@media(max-width:900px){
  .page-wrap{grid-template-columns:1fr}
  .sidebar{border-right:none;border-bottom:1px solid #0e3050}
  .chart-wrap{height:60vw;min-height:280px}
}
)rawliteral";