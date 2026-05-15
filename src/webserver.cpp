#include "webserver.h"
#include "html.h"
#include "css.h"
#include "js.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

// ─── Server & SSE ────────────────────────────────────────────────────────────
static AsyncWebServer server(80);
static AsyncEventSource events("/events");
static Preferences prefs;
static String configuredSSID;
static String configuredPass;

static void saveWifiConfig(const String& ssid, const String& pass) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass); 
    prefs.end();
    configuredSSID = ssid;
    configuredPass = pass;
}

static void loadWifiConfig(const char* defaultSsid, const char* defaultPass) {
    prefs.begin("wifi", true);
    String savedSsid = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");
    prefs.end();

    if (savedSsid.length() > 0) {
        configuredSSID = savedSsid;
        configuredPass = savedPass;
    } else {
        configuredSSID = defaultSsid;
        configuredPass = defaultPass;
    }
}

static String makeWifiJson() {
    const bool staConnected = WiFi.status() == WL_CONNECTED;
    const String staSsid = WiFi.SSID();
    const String staIp = staConnected ? WiFi.localIP().toString() : String("0.0.0.0");
    const String apSsid = WiFi.softAPSSID();
    const String json = String("{"
        "\"staConnected\":" ) + (staConnected ? "true" : "false") +
        ",\"staSsid\":\"" + staSsid +
        "\",\"staIp\":\"" + staIp +
        "\",\"apSsid\":\"" + apSsid +
        "\",\"configuredSsid\":\"" + configuredSSID +
        "\"}";
    return json;
}

// ─── Experiment state ─────────────────────────────────────────────────────────
static bool  experimentRunning = false;

// Ring buffer — 600 pts @ 1 Hz = 10 min
#define MAX_POINTS 600
struct DataPoint { float t, t1, t2, prom, delta, placa; };
static DataPoint buffer[MAX_POINTS];
static int  bufHead  = 0;
static int  bufCount = 0;

static unsigned long startMs = 0;

// ─── Routes ──────────────────────────────────────────────────────────────────
static void setupRoutes() {

    // Static assets (served from PROGMEM)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "text/html", INDEX_HTML);
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "text/css", STYLE_CSS);
    });
    server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "application/javascript", APP_JS);
    });

    // Start experiment
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest* req) {
        experimentRunning = true;
        bufHead  = 0;
        bufCount = 0;
        startMs  = millis();          // reset experiment timer on START
        req->send(200, "text/plain", "STARTED");
    });

    // Stop experiment
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest* req) {
        experimentRunning = false;
        req->send(200, "text/plain", "STOPPED");
    });

    // CSV download — streams the ring buffer
    server.on("/csv", HTTP_GET, [](AsyncWebServerRequest* req) {
        AsyncResponseStream* resp = req->beginResponseStream("text/csv");
        resp->addHeader("Content-Disposition",
                        "attachment; filename=\"experimento.csv\"");
        resp->print("time,T1,T2,Prom,Delta,Placa\n");
        int start = (bufCount < MAX_POINTS) ? 0 : bufHead;
        for (int i = 0; i < bufCount; i++) {
            int idx = (start + i) % MAX_POINTS;
            DataPoint& p = buffer[idx];
            char row[80];
            snprintf(row, sizeof(row), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                     p.t, p.t1, p.t2, p.prom, p.delta, p.placa);
            resp->print(row);
        }
        req->send(resp);
    });

    server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", makeWifiJson());
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", String("{\"running\":") + (experimentRunning ? "true" : "false") + "}");
    });

    server.on("/wifi-config", HTTP_GET, [](AsyncWebServerRequest* req) {
        const AsyncWebParameter* ssidParam = req->getParam("ssid");
        const AsyncWebParameter* passParam = req->getParam("pass");
        if (ssidParam && passParam) {
            const String ssid = ssidParam->value();
            const String pass = passParam->value();
            if (ssid.length() > 0) {
                saveWifiConfig(ssid, pass);
                WiFi.disconnect();
                WiFi.begin(configuredSSID.c_str(), configuredPass.c_str());
                req->send(200, "application/json", String("{\"ok\":true,\"ssid\":\"") + ssid + "\"}");
                return;
            }
        }
        req->send(400, "application/json", "{\"ok\":false,\"error\":\"ssid required\"}");
    });

    // SSE endpoint — AsyncEventSource handles reconnects automatically
    events.onConnect([](AsyncEventSourceClient* client) {
        Serial.printf("[SSE] Cliente conectado, id=%u, reconexion=%s\n",
                      client->lastId(),
                      client->lastId() ? "SI" : "NO");
        // Send a ping so the browser knows it's live
        client->send("ping", NULL, millis(), 1000);
    });
    server.addHandler(&events);

    // 404
    server.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "text/plain", "Not found");
    });
}

// ─── WiFi with auto-reconnect ─────────────────────────────────────────────────
static const char* _ssid;
static const char* _pass;

static void wifiOnEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("[WiFi] Desconectado — reconectando...");
            WiFi.reconnect();
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[WiFi] IP: ");
            Serial.println(WiFi.localIP());
            break;
        default:
            break;
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────
void webServerInit(const char* ssid, const char* password) {
    _ssid = ssid;
    _pass = password;

    loadWifiConfig(ssid, password);

    WiFi.onEvent(wifiOnEvent);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.softAP("PruebaDeFuego-Setup", "prueba123");
    WiFi.begin(configuredSSID.c_str(), configuredPass.c_str());

    Serial.print("[WiFi] Conectando a ");
    Serial.println(configuredSSID);

    // Wait up to 15 s — but don't block forever; server starts regardless
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
    } else {
        // fallback: if stored config failed, try secrets values once
        if (configuredSSID != ssid || configuredPass != password) {
            Serial.println("[WiFi] Credenciales guardadas fallaron, probando secrets...");
            configuredSSID = ssid;
            configuredPass = password;
            WiFi.disconnect(true);
            WiFi.begin(configuredSSID.c_str(), configuredPass.c_str());
            unsigned long t1 = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - t1 < 15000) {
                delay(250);
                Serial.print(".");
            }
            Serial.println();
            if (WiFi.status() == WL_CONNECTED) {
                Serial.print("[WiFi] IP: ");
                Serial.println(WiFi.localIP());
            } else {
                Serial.println("[WiFi] Sin red aun — esperando reconexion automatica");
            }
        } else {
            Serial.println("[WiFi] Sin red aun — esperando reconexion automatica");
        }
    }

    startMs = millis();
    setupRoutes();
    server.begin();
    Serial.println("[HTTP] AsyncWebServer en puerto 80");
}

void webServerPushData(SensorData data, SystemState state) {
    float t = (millis() - startMs) / 1000.0f;

    if (experimentRunning) {
        buffer[bufHead] = { t,
                            data.tempsHorno[0], data.tempsHorno[1],
                            data.tempProm, data.delta, data.tempPlaca };
        bufHead = (bufHead + 1) % MAX_POINTS;
        if (bufCount < MAX_POINTS) bufCount++;
    }

    // Push to ALL connected SSE clients — AsyncEventSource handles
    // reconnected clients, multiple tabs, etc. automatically
    if (events.count() > 0) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "{\"t\":%.2f,\"t1\":%.2f,\"t2\":%.2f,\"prom\":%.2f,"
            "\"delta\":%.2f,\"placa\":%.2f,\"falla\":%s,"
            "\"hornoValido\":%s,\"running\":%s}",
            t,
            data.tempsHorno[0], data.tempsHorno[1],
            data.tempProm, data.delta, data.tempPlaca,
            state.falla       ? "true" : "false",
            state.HornoValido ? "true" : "false",
            experimentRunning ? "true" : "false");
        events.send(msg, "message", millis());
    }
}