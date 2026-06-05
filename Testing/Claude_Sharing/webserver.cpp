#include "webserver.h"
#include "wifi_manager.h"
#include "html.h"
#include "css.h"
#include "js.h"
#include "serial_commands.h"
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

// ── Server instance ───────────────────────────────────────────────────────────
static AsyncWebServer  _server(80);
static AsyncEventSource _events("/events");

// ── Route handlers ────────────────────────────────────────────────────────────

// GET /
static void handleRoot(AsyncWebServerRequest* req) {
    req->send(200, "text/html", INDEX_HTML);
}

// GET /style.css
static void handleCSS(AsyncWebServerRequest* req) {
    req->send(200, "text/css", STYLE_CSS);
}

// GET /app.js
static void handleJS(AsyncWebServerRequest* req) {
    req->send(200, "application/javascript", APP_JS);
}

// GET /status  — experiment running state (used by frontend on load)
static void handleStatus(AsyncWebServerRequest* req) {
    // We don't have direct access to running state here; the JS polls this
    // to hydrate the UI on reconnect.  Return a minimal JSON.
    // The real running flag comes through SSE.
    req->send(200, "application/json", "{\"running\":false}");
}

// GET /wifi  — current WiFi status (polled by frontend every 5 s)
static void handleWifi(AsyncWebServerRequest* req) {
    String json = "{";
    json += "\"staConnected\":"  + String(WiFiManager::isConnected() ? "true" : "false");
    json += ",\"staSsid\":\""    + WiFiManager::getStoredSSID() + "\"";
    json += ",\"configuredSsid\":\"" + WiFiManager::getStoredSSID() + "\"";
    json += "}";
    req->send(200, "application/json", json);
}

// GET /wifi-config?ssid=...&pass=...  — save credentials (frontend form)
// NOTE: In the new architecture the serial CLI is the preferred path,
//       but the web UI wifi panel still works for convenience.
static void handleWifiConfig(AsyncWebServerRequest* req) {
    if (!req->hasParam("ssid")) {
        req->send(400, "application/json", "{\"ok\":false,\"error\":\"missing ssid\"}");
        return;
    }
    String ssid = req->getParam("ssid")->value();
    String pass = req->hasParam("pass") ? req->getParam("pass")->value() : "";

    bool ok = WiFiManager::setCredentials(ssid, pass);
    req->send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"nvs write failed\"}");
}

// GET /start  — start experiment
static void handleStart(AsyncWebServerRequest* req) {
    // Actual start logic lives in logic.h/SystemState — hook here as needed.
    req->send(200, "application/json", "{\"ok\":true}");
}

// GET /stop  — stop experiment
static void handleStop(AsyncWebServerRequest* req) {
    req->send(200, "application/json", "{\"ok\":true}");
}

// GET /csv  — download recorded data
static void handleCSV(AsyncWebServerRequest* req) {
    // TODO: return actual recorded CSV from your storage/SD layer.
    req->send(200, "text/csv", "t,t1,t2,prom,delta,placa\n");
}

// ── SSE client connect ────────────────────────────────────────────────────────
static void onEventConnect(AsyncEventSourceClient* client) {
    if (client->lastId()) {
        char buf[128];
        snprintf(buf, sizeof(buf), "{\"sse\":\"client_reconnected\",\"lastId\":%u}", client->lastId());
        SerialCLI::interruptPrint(buf);
    }
    client->send("connected", nullptr, 0, 1000);
}

// ── Public API ────────────────────────────────────────────────────────────────
void webServerInit() {
    // NOTE: WiFi connection is fully managed by WiFiManager::begin()/handle().
    //       This function only registers routes and starts the HTTP server.

    _events.onConnect(onEventConnect);
    _server.addHandler(&_events);

    _server.on("/",            HTTP_GET, handleRoot);
    _server.on("/style.css",   HTTP_GET, handleCSS);
    _server.on("/app.js",      HTTP_GET, handleJS);
    _server.on("/status",      HTTP_GET, handleStatus);
    _server.on("/wifi",        HTTP_GET, handleWifi);
    _server.on("/wifi-config", HTTP_GET, handleWifiConfig);
    _server.on("/start",       HTTP_GET, handleStart);
    _server.on("/stop",        HTTP_GET, handleStop);
    _server.on("/csv",         HTTP_GET, handleCSV);

    _server.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "application/json", "{\"error\":\"not found\"}");
    });

    _server.begin();
    SerialCLI::interruptPrint("{\"webserver\":\"started\",\"port\":80}");
}

void webServerPushData(const SensorData& data, const SystemState& state) {
    if (_events.count() == 0) return;   // no clients, skip serialisation

    char buf[192];
    snprintf(buf, sizeof(buf),
        "{"
        "\"t\":%.1f,"
        "\"t1\":%.2f,"
        "\"t2\":%.2f,"
        "\"prom\":%.2f,"
        "\"delta\":%.2f,"
        "\"placa\":%.2f,"
        "\"falla\":%s,"
        "\"hornoValido\":%s"
        "}",
        millis() / 1000.0f,
        data.tempsHorno[0],
        data.tempsHorno[1],
        data.tempProm,
        data.delta,
        data.tempPlaca,
        state.falla       ? "true" : "false",
        state.HornoValido ? "true" : "false"
    );
    _events.send(buf, "message", millis());
}