#pragma once
#include <Arduino.h>

// ── SerialCLI ─────────────────────────────────────────────────────────────────
// Non-blocking, line-based serial command interface.
//
//   • Accumulates incoming bytes into a line buffer (no blocking reads).
//   • On newline, tokenizes the line and dispatches to a command handler.
//   • All responses are JSON-formatted for easy machine/AI parsing.
//   • Adding new commands: just extend the dispatch table in serial_commands.cpp.
// ─────────────────────────────────────────────────────────────────────────────

namespace SerialCLI {

// Call once in setup(). Prints a boot banner.
void begin(unsigned long baudRate = 115200);

// Call every loop iteration — reads available bytes, dispatches complete lines.
void handle();

// Print an asynchronous message without losing the user's current typed line.
void interruptPrint(const char* msg);

} // namespace SerialCLI