#include "wifi_manager.h"
#include <Preferences.h>
#include <WiFi.h>
#include "serial_commands.h"

// ── Internal constants ────────────────────────────────────────────────────────
static const char* NVS_NAMESPACE  = "wifi_cfg";
static const char* NVS_KEY_SSID   = "ssid";
static const char* NVS_KEY_PASS   = "pass";

// How long to wait before retrying a disconnected Wi-Fi (ms)
static const unsigned long WIFI_RETRY_MS    = 15000UL;
// How long to wait for initial association before giving up (ms)
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 12000UL;

// ── Module-private state ──────────────────────────────────────────────────────
static Preferences _prefs;
static String      _ssid;
static String      _pass;
static bool        _hasCredentials  = false;
static unsigned long _lastAttemptMs = 0;
static bool        _connecting      = false;
static bool        _justConnected   = false;  // single-tick edge flag

// ── Internal helpers ──────────────────────────────────────────────────────────
static void _loadCredentials() {
    // Open read-write so the namespace is created on first boot instead of
    // returning NOT_FOUND. Reading values works identically either way.
    _prefs.begin(NVS_NAMESPACE, /*readOnly=*/false);
    _ssid = _prefs.getString(NVS_KEY_SSID, "");
    _pass = _prefs.getString(NVS_KEY_PASS, "");
    _prefs.end();
    _hasCredentials = (_ssid.length() > 0);
}

static void _startConnect() {
    if (!_hasCredentials) return;
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _pass.c_str());
    _lastAttemptMs = millis();
    _connecting    = true;
    char buf[128];
    snprintf(buf, sizeof(buf), "{\"wifi\":\"connecting\",\"ssid\":\"%s\"}", _ssid.c_str());
    SerialCLI::interruptPrint(buf);
}

// ── Public API ────────────────────────────────────────────────────────────────
namespace WiFiManager {

void begin() {
    WiFi.setAutoReconnect(false);   // we manage reconnect ourselves
    WiFi.persistent(false);         // don't let the SDK overwrite NVS partition
    _loadCredentials();
    if (_hasCredentials) {
        _startConnect();
    } else {
        SerialCLI::interruptPrint("{\"wifi\":\"no_credentials\",\"hint\":\"use: wifi set <SSID> <PASSWORD>\"}");
    }
}

void handle() {
    if (!_hasCredentials) return;

    if (_connecting) {
        // Check for successful association
        if (WiFi.status() == WL_CONNECTED) {
            _connecting    = false;
            _justConnected = true;   // edge flag — consumed once by wasJustConnected()
            char connbuf[128];
            snprintf(connbuf, sizeof(connbuf), "{\"wifi\":\"connected\",\"ssid\":\"%s\",\"ip\":\"%s\"}",
                     _ssid.c_str(), WiFi.localIP().toString().c_str());
            SerialCLI::interruptPrint(connbuf);
            // Print a human-friendly dashboard URL via SerialCLI to avoid corrupting input
            char urlbuf[64];
            snprintf(urlbuf, sizeof(urlbuf), "Open dashboard: http://%s/", WiFi.localIP().toString().c_str());
            SerialCLI::interruptPrint(urlbuf);
            return;
        }
        // Connection timeout — stop waiting, schedule retry
        if (millis() - _lastAttemptMs > WIFI_CONNECT_TIMEOUT_MS) {
            _connecting = false;
            WiFi.disconnect(/*wifioff=*/false);
            char tbuf[96];
            snprintf(tbuf, sizeof(tbuf), "{\"wifi\":\"timeout\",\"ssid\":\"%s\"}", _ssid.c_str());
            SerialCLI::interruptPrint(tbuf);
        }
        return;
    }

    // Not currently connecting — retry if disconnected
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - _lastAttemptMs > WIFI_RETRY_MS) {
            _startConnect();
        }
    }
}

bool setCredentials(const String& ssid, const String& password) {
    if (ssid.length() == 0) return false;

    _prefs.begin(NVS_NAMESPACE, /*readOnly=*/false);
    bool ok = _prefs.putString(NVS_KEY_SSID, ssid) &&
              _prefs.putString(NVS_KEY_PASS, password);
    _prefs.end();

    if (ok) {
        _ssid = ssid;
        _pass = password;
        _hasCredentials = true;
        WiFi.disconnect(true);
        _connecting = false;
        _startConnect();
    }
    return ok;
}

void resetCredentials() {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.clear();
    _prefs.end();
    _ssid = "";
    _pass = "";
    _hasCredentials = false;
    _connecting     = false;
    WiFi.disconnect(/*wifioff=*/true);
    SerialCLI::interruptPrint("{\"wifi\":\"reset\",\"msg\":\"credentials erased, wifi disabled\"}");
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String getIP() {
    return WiFi.localIP().toString();
}

String getStoredSSID() {
    return _ssid;
}

String statusJSON() {
    String s = "{";
    if (isConnected()) {
        s += "\"wifi\":\"connected\"";
        s += ",\"ssid\":\""  + _ssid + "\"";
        s += ",\"ip\":\""    + getIP() + "\"";
    } else if (_connecting) {
        s += "\"wifi\":\"connecting\"";
        s += ",\"ssid\":\"" + _ssid + "\"";
    } else if (_hasCredentials) {
        s += "\"wifi\":\"disconnected\"";
        s += ",\"ssid\":\"" + _ssid + "\"";
    } else {
        s += "\"wifi\":\"no_credentials\"";
    }
    s += "}";
    return s;
}

bool wasJustConnected() {
    if (_justConnected) {
        _justConnected = false;  // consume — returns true only once per connection
        return true;
    }
    return false;
}

} // namespace WiFiManager