#include "serial_commands.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "sensors.h"
#include <Arduino.h>
#include <math.h>
#include <vector>

// ── Parser internals ──────────────────────────────────────────────────────────
static const size_t LINE_BUF_MAX = 256;
static char   _lineBuf[LINE_BUF_MAX];
static size_t _lineLen = 0;
static const char PROMPT[] = "> ";

// Tokenize a null-terminated string into a vector of String tokens.
// Splits on whitespace, respects nothing (no quoting needed for our commands).
static std::vector<String> tokenize(const char* line) {
    std::vector<String> tokens;
    const char* p = line;
    while (*p) {
        // skip whitespace
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        // quoted token support
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            const char* start = p;
            while (*p && *p != quote) p++;
            tokens.push_back(String(start).substring(0, p - start));
            if (*p == quote) p++;
            continue;
        }
        // unquoted token
        const char* start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        tokens.push_back(String(start).substring(0, p - start));
    }
    return tokens;
}

// ── Response helpers ──────────────────────────────────────────────────────────
static void ok(const String& msg) {
    Serial.printf("{\"status\":\"ok\",\"msg\":\"%s\"}\n", msg.c_str());
}
static void err(const String& msg) {
    Serial.printf("{\"status\":\"error\",\"msg\":\"%s\"}\n", msg.c_str());
}
static void raw(const String& json) {
    Serial.println(json);
}

// ── Command handlers ──────────────────────────────────────────────────────────

// ayuda
static void cmdHelp(const std::vector<String>&) {
    Serial.println("Comandos disponibles:");
    Serial.println("  ayuda\t\tMuestra esta ayuda");
    Serial.println("  wifi guardar <SSID> <PASSWORD>\tGuardar credenciales y conectar");
    Serial.println("    Usa comillas si el SSID o la contraseña tienen espacios:");
    Serial.println("      wifi guardar \"Mi Red\" \"pass con espacios\"");
    Serial.println("  wifi estado\tMuestra el estado de conexión y SSID guardado");
    Serial.println("  wifi restablecer\tBorra las credenciales de WiFi");
    Serial.println("  reiniciar\t\tReinicia el dispositivo");
    Serial.println("  sensor estado\tImprime los valores actuales de los sensores en JSON");
    Serial.println("  sensor diagnostico\tComprueba si los sensores devuelven valores válidos");
    Serial.println("  log iniciar|detener\tActiva o detiene el registro periódico por serial");
}

// wifi set <SSID> <PASSWORD>
// wifi status
// wifi reset
static void cmdWifi(const std::vector<String>& tok) {
    if (tok.size() < 2) { err("uso: wifi <guardar|estado|restablecer>"); return; }

    const String& sub = tok[1];

    if (sub == "estado" || sub == "status") {
        raw(WiFiManager::statusJSON());
        return;
    }

    if (sub == "restablecer" || sub == "reset") {
        WiFiManager::resetCredentials();
        // resetCredentials() already prints a JSON line
        return;
    }

    if (sub == "guardar" || sub == "set") {
        if (tok.size() < 4) {
            err("uso: wifi guardar <SSID> <PASSWORD>");
            return;
        }
        const String& ssid = tok[2];
        const String& pass = tok[3];

        // Password with spaces: if the user typed more tokens, join them.
        String fullPass = pass;
        for (size_t i = 4; i < tok.size(); i++) {
            fullPass += " ";
            fullPass += tok[i];
        }

        if (WiFiManager::setCredentials(ssid, fullPass)) {
            ok("credenciales guardadas, conectando...");
        } else {
            err("no se pudieron guardar las credenciales");
        }
        return;
    }

    err("subcomando wifi desconocido");
}

// reboot
static void cmdReboot(const std::vector<String>&) {
    ok("reiniciando en 500 ms");
    Serial.flush();
    delay(500);
    ESP.restart();
}

// sensor status
static void cmdSensor(const std::vector<String>& tok) {
    if (tok.size() < 2) { err("uso: sensor <estado|diagnostico>"); return; }
    const String& sub = tok[1];
    if (sub == "estado" || sub == "status") {
        raw(SensorManager::statusJSON());
        return;
    }
    if (sub == "diagnostico" || sub == "diagnostic") {
        SensorData data = readSensors();
        bool t1_ok = !isnan(data.tempsHorno[0]);
        bool t2_ok = !isnan(data.tempsHorno[1]);
        bool placa_ok = !isnan(data.tempPlaca);
        char buf[256];
        snprintf(buf, sizeof(buf),
            "{\"t1\":%.2f,\"t2\":%.2f,\"placa\":%.2f,\"t1_ok\":%s,\"t2_ok\":%s,\"placa_ok\":%s}",
            data.tempsHorno[0],
            data.tempsHorno[1],
            data.tempPlaca,
            t1_ok ? "true" : "false",
            t2_ok ? "true" : "false",
            placa_ok ? "true" : "false"
        );
        raw(String(buf));
        return;
    }
    err("subcomando sensor desconocido");
}

// log start / log stop
static void cmdLog(const std::vector<String>& tok) {
    if (tok.size() < 2) { err("uso: log <iniciar|detener>"); return; }
    if (tok[1] == "iniciar" || tok[1] == "start") {
        SensorManager::setLogging(true);
        ok("registro iniciado");
    } else if (tok[1] == "detener" || tok[1] == "stop") {
        SensorManager::setLogging(false);
        ok("registro detenido");
    } else {
        err("subcomando log desconocido");
    }
}

// ── Dispatch table ────────────────────────────────────────────────────────────
// Add new top-level commands here without touching anything else.
struct Command {
    const char* name;
    void (*handler)(const std::vector<String>&);
};

static const Command COMMANDS[] = {
    { "help",    cmdHelp   },
    { "ayuda",   cmdHelp   },
    { "wifi",    cmdWifi   },
    { "reiniciar", cmdReboot },
    { "reboot",  cmdReboot },
    { "sensor",  cmdSensor },
    { "log",     cmdLog    },
};
static const size_t COMMANDS_LEN = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

static void dispatch(const char* line) {
    auto tokens = tokenize(line);
    if (tokens.empty()) return;

    const String& cmd = tokens[0];
    for (size_t i = 0; i < COMMANDS_LEN; i++) {
        if (cmd.equalsIgnoreCase(COMMANDS[i].name)) {
            COMMANDS[i].handler(tokens);
            return;
        }
    }
    Serial.printf("{\"status\":\"error\",\"msg\":\"comando desconocido: %s\",\"hint\":\"escribe ayuda\"}\n",
                  cmd.c_str());
}

// ── Public API ────────────────────────────────────────────────────────────────
namespace SerialCLI {

void begin(unsigned long baudRate) {
    Serial.begin(baudRate);
    delay(200); // let USB-serial settle
    Serial.println();
    Serial.println("{\"boot\":\"Prueba de Fuego R&D\",\"cli\":\"listo\",\"hint\":\"escribe ayuda\"}");
    // provide a simple interactive prompt for humans
    Serial.print(PROMPT);
}

void handle() {
    while (Serial.available()) {
        char c = (char)Serial.read();

        // Ignore carriage returns (Windows line endings)
        if (c == '\r') continue;

        // Newline: terminate and dispatch the command
        if (c == '\n') {
            _lineBuf[_lineLen] = '\0';
            if (_lineLen > 0) {
                dispatch(_lineBuf);
            }
            _lineLen = 0;
            // finish the line and reprint prompt for interactive users
            Serial.println();
            Serial.print(PROMPT);
            continue; // keep processing any queued input
        }

        // Backspace / DEL support for interactive terminals
        if ((c == '\b' || c == 127)) {
            if (_lineLen > 0) {
                _lineLen--;
                // erase character on user's terminal
                Serial.print("\b \b");
            }
            continue;
        }

        // Store printable characters in the line buffer (no device echo).
        if (_lineLen < LINE_BUF_MAX - 1) {
            _lineBuf[_lineLen++] = c;
        }
        // silently drop overflow characters
    }
}

// Print an asynchronous message without losing the user's current typed line.
void interruptPrint(const char* msg) {
    // Emit a newline, the message, and reprint prompt + partial input.
    Serial.println();
    Serial.println(msg);
    Serial.print(PROMPT);
    if (_lineLen > 0) {
        Serial.write((const uint8_t*)_lineBuf, _lineLen);
    }
}

} // namespace SerialCLI