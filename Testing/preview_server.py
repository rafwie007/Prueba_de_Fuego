#!/usr/bin/env python3
"""
Prueba de Fuego — Dashboard Preview Server
==========================================
Extracts HTML/CSS/JS straight from your .cpp files, injects a mock SSE
data stream, and serves everything on http://localhost:8080

Usage:
    python3 preview_server.py [--port 8080] [--project /path/to/cpp/files]

Any time you save a change to html.cpp, css.cpp, or js.cpp just reload
the browser — the server always re-reads the files on each request.
"""

import argparse
import re
import time
import math
import random
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

# ── Config ────────────────────────────────────────────────────────────────────
DEFAULT_PORT    = 8080
DEFAULT_PROJECT = Path(r"C:\Users\rafwi\Documents\PlatformIO\Projects\Prueba_de_Fuego\Testing\Claude_Sharing")

# ── Extract rawliteral content from a .cpp file ───────────────────────────────
def extract(filepath: Path) -> str:
    try:
        text = filepath.read_text(encoding="utf-8")
    except FileNotFoundError:
        return f"/* ERROR: {filepath} not found */"
    m = re.search(r'R"rawliteral\((.*?)\)rawliteral"', text, re.DOTALL)
    return m.group(1) if m else f"/* ERROR: no rawliteral block found in {filepath.name} */"

# ── Mock SSE data generator ───────────────────────────────────────────────────
# Simulates a fire-resistance test: furnace heats up, cold face lags behind.
class MockSensor:
    def __init__(self):
        self.t = 0.0          # seconds since experiment start
        self.running = False
        self.start_time = None

    def start(self):
        self.running = True
        self.start_time = time.time()
        self.t = 0.0

    def stop(self):
        self.running = False

    def reading(self):
        if not self.running:
            return None
        self.t = time.time() - self.start_time

        # Furnace curve: sigmoid-ish ramp to ~900°C over ~15 min
        t_min = self.t / 60.0
        t1_base = 20 + 880 * (1 - math.exp(-t_min / 4))
        t2_base = t1_base * 0.97  # slight asymmetry between thermocouples

        noise1 = random.gauss(0, 2.5)
        noise2 = random.gauss(0, 2.5)

        t1 = round(t1_base + noise1, 2)
        t2 = round(t2_base + noise2, 2)
        prom = round((t1 + t2) / 2, 2)
        delta = round(abs(t1 - t2), 2)

        # Cold face (placa): lags behind furnace with thermal mass
        placa_base = 20 + 600 * (1 - math.exp(-t_min / 8))
        placa = round(placa_base + random.gauss(0, 1.5), 2)

        falla = False
        horno_valido = prom > 50

        return {
            "t": round(self.t, 1),
            "t1": t1,
            "t2": t2,
            "prom": prom,
            "delta": delta,
            "placa": placa,
            "falla": "true" if falla else "false",
            "hornoValido": "true" if horno_valido else "false",
        }

sensor = MockSensor()
recorded_data = []   # list of reading dicts captured while running

# ── Mock JS patch — replaces fetch('/events') SSE with mock polling ───────────
MOCK_JS_PATCH = """
// ── PREVIEW MOCK: override SSE with polling /mock-data ──────────────────────
(function() {
  // Kill the real EventSource
  const OrigES = window.EventSource;

  // Poll /mock-data every 500 ms and call the existing push() function
  function startMockStream() {
    setInterval(async () => {
      try {
        const r = await fetch('/mock-data');
        if (!r.ok) return;
        const d = await r.json();
        if (d && d.t !== undefined) {
          push(d);
          setConn(true);
        }
      } catch(e) {
        setConn(false);
      }
    }, 500);
  }

  // Intercept EventSource construction and start mock instead
  window.EventSource = function(url) {
    const fake = { close: () => {} };
    startMockStream();
    return fake;
  };

  // Override fetch for /start, /stop, /status, /csv so buttons work
  const origFetch = window.fetch;
  window.fetch = function(url, opts) {
    if (url === '/start')  return origFetch('/mock-start');
    if (url === '/stop')   return origFetch('/mock-stop');
    if (url === '/status') return origFetch('/mock-status');
    if (url === '/csv')    return origFetch('/mock-csv');
    return origFetch(url, opts);
  };
})();
"""

# ── Assemble the full preview HTML ────────────────────────────────────────────
def build_preview_html(project_dir: Path) -> str:
    raw_html = extract(project_dir / "html.cpp")
    css      = extract(project_dir / "css.cpp")
    js       = extract(project_dir / "js.cpp")

    # Strip the <link> and <script src> tags — we'll inline everything
    raw_html = re.sub(r'<link[^>]+/style\.css[^>]*>', '', raw_html)
    raw_html = re.sub(r'<script[^>]+/app\.js[^>]*></script>', '', raw_html)

    # Inject preview banner CSS
    banner_css = """
    #preview-banner {
      position: fixed; top: 0; left: 0; right: 0; z-index: 9999;
      background: linear-gradient(90deg, #1a0a00, #2a1500);
      border-bottom: 1px solid #f90;
      color: #f90; font-family: 'Courier New', monospace;
      font-size: 11px; font-weight: 700; letter-spacing: .08em;
      padding: 5px 14px; display: flex; align-items: center; gap: 12px;
    }
    #preview-banner span.dot {
      width: 7px; height: 7px; border-radius: 50%;
      background: #f90; display: inline-block;
      animation: blink 1.2s ease-in-out infinite;
    }
    #preview-banner .src { color: #666; font-weight: 400; margin-left: auto; }
    body { padding-top: 28px !important; }
    """

    # Inject the preview banner into <body>
    banner_html = f"""
<div id="preview-banner">
  <span class="dot"></span>
  PREVIEW MODE — reads from .cpp files live — reload after saving changes
  <span class="src">html.cpp · css.cpp · js.cpp</span>
</div>
"""

    # Build the final page
    raw_html = raw_html.replace('</head>', f"""
<style>
{css}
{banner_css}
</style>
</head>""")

    raw_html = raw_html.replace('<body>', f'<body>\n{banner_html}')

    raw_html = raw_html.replace('</body>', f"""
<script>
{MOCK_JS_PATCH}
{js}
</script>
</body>""")

    return raw_html

# ── HTTP Request Handler ───────────────────────────────────────────────────────
class Handler(BaseHTTPRequestHandler):
    project_dir: Path = DEFAULT_PROJECT

    def log_message(self, fmt, *args):
        # Suppress noisy access log; only print errors
        if args and str(args[1]) not in ('200', '204'):
            print(f"  {args[0]}  {args[1]}")

    def send_json(self, code, body):
        data = body.encode()
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', len(data))
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(data)

    def send_text(self, code, ctype, body):
        data = body.encode()
        self.send_response(code)
        self.send_header('Content-Type', ctype)
        self.send_header('Content-Length', len(data))
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        path = self.path.split('?')[0]

        # ── Main page (always fresh) ──────────────────────────────────────────
        if path == '/':
            html = build_preview_html(self.project_dir)
            self.send_text(200, 'text/html; charset=utf-8', html)

        # ── Mock data endpoint (polled every 500ms) ───────────────────────────
        elif path == '/mock-data':
            reading = sensor.reading()
            if reading is None:
                # Not running — send idle state
                import json
                self.send_json(200, '{"t":0,"t1":20.0,"t2":20.0,"prom":20.0,"delta":0.0,"placa":20.0,"falla":false,"hornoValido":false}')
            else:
                import json
                # Convert string booleans back to real booleans for JSON
                r = dict(reading)
                r['falla'] = r['falla'] == 'true'
                r['hornoValido'] = r['hornoValido'] == 'true'
                if sensor.running:
                    recorded_data.append(r)
                self.send_json(200, json.dumps(r))

        # ── Mock controls ──────────────────────────────────────────────────────
        elif path == '/mock-start':
            recorded_data.clear()
            sensor.start()
            print("  [mock] Experiment STARTED")
            self.send_json(200, '{"ok":true}')

        elif path == '/mock-stop':
            sensor.stop()
            print(f"  [mock] Experiment STOPPED — {len(recorded_data)} data points recorded")
            self.send_json(200, '{"ok":true}')

        elif path == '/mock-status':
            state = 'true' if sensor.running else 'false'
            self.send_json(200, f'{{"running":{state}}}')

        elif path == '/mock-csv':
            import io
            lines = ['t,t1,t2,prom,delta,placa']
            for r in recorded_data:
                lines.append(f"{r['t']},{r['t1']},{r['t2']},{r['prom']},{r['delta']},{r['placa']}")
            csv_text = '\n'.join(lines)
            self.send_text(200, 'text/csv', csv_text)

        else:
            self.send_json(404, '{"error":"not found"}')

# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(description='Prueba de Fuego preview server')
    parser.add_argument('--port',    type=int,  default=DEFAULT_PORT)
    parser.add_argument('--project', type=Path, default=DEFAULT_PROJECT)
    args = parser.parse_args()

    Handler.project_dir = args.project.resolve()

    if not Handler.project_dir.exists():
        print(f"ERROR: project directory not found: {Handler.project_dir}")
        return

    print(f"""
╔══════════════════════════════════════════════════════╗
║      Prueba de Fuego — Dashboard Preview Server      ║
╠══════════════════════════════════════════════════════╣
║  Open in browser:  http://localhost:{args.port:<5}             ║
║  Project files:    {str(args.project):<34}║
║                                                      ║
║  • Reload browser after editing any .cpp file        ║
║  • Press Ctrl+C to stop                              ║
╚══════════════════════════════════════════════════════╝
""")

    server = HTTPServer(('', args.port), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n  Server stopped.")

if __name__ == '__main__':
    main()
