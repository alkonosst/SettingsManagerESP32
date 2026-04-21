/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three uint32_t settings are created, all formattable.
 * - onChange callbacks are registered at two levels:
 *   - A global callback that fires on setValue for all settings (not on format).
 *   - A per-setting callback for Counter_1 that does NOT fire on format.
 *   - A per-setting callback for Counter_2 that DOES fire on format.
 * - The loop function reads the serial input and performs the following actions:
 *  - '.' restarts the ESP32.
 *  - 'p' prints the current value of each setting.
 *  - 's' sets all settings to random values — all callbacks fire.
 *  - 'f' formats all settings to their default values — only Counter_2's callback fires.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Counters, all formattable
#define COUNTERS(X)                  \
  X(Counter_1, "Counter 1", 0, true) \
  X(Counter_2, "Counter 2", 0, true) \
  X(Counter_3, "Counter 3", 0, true)

// Enum for counter settings
enum class Counters : uint8_t { COUNTERS(SETTINGS_EXPAND_ENUM_CLASS) };

// Settings object for counters, namespace "esp32"
NVS::Settings<uint32_t, Counters, SETTINGS_COUNT(COUNTERS)>
  counters("esp32", {COUNTERS(SETTINGS_EXPAND_SETTINGS)});

// Per-setting callback for Counter_1. Registered with callable_on_format=false, so it only fires
// when setValue() is called directly.
void onCounter1Change(const char* key, const Counters setting, const uint32_t value) {
  Serial.printf("  [Counter_1] %s = %" PRIu32 "\n", key, value);
}

// Per-setting callback for Counter_2. Registered with callable_on_format=true, so it fires on
// both setValue() and format operations.
void onCounter2Change(const char* key, const Counters setting, const uint32_t value) {
  Serial.printf("  [Counter_2] %s = %" PRIu32 "\n", key, value);
}

// Global callback. Fires for any setting when setValue() is called (callable_on_format=false).
void onAnyChange(const char* key, const NVS::Type type, const size_t index, const void* value) {
  Serial.printf("  [global] index=%u, key=%s, value=%" PRIu32 "\n",
                index,
                key,
                *static_cast<const uint32_t*>(value));
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!counters.begin()) {
    Serial.println("Failed to open settings handle!");
    while (true)
      delay(1000);
  }

  // Global callback: fires on setValue, silent during format
  counters.setGlobalOnChangeCallback(onAnyChange, false);

  // Counter_1: fires on setValue only
  counters.setOnChangeCallback(Counters::Counter_1, onCounter1Change, false);

  // Counter_2: fires on setValue AND format
  counters.setOnChangeCallback(Counters::Counter_2, onCounter2Change, true);
}

void loop() {
  if (!Serial.available()) return;

  char c = Serial.read();
  if (c == '\r') return;

  if (c == '\n') {
    Serial.println(">");
  } else {
    Serial.printf("> %c\r\n", c);
  }

  switch (c) {
    // Restart ESP32
    case '.': ESP.restart(); break;

    // Print all settings
    case 'p':
    {
      Serial.println("Current values:");
      Serial.printf("%-12s%-12s\n", "Key", "Value");

      for (size_t i = 0; i < counters.getSize(); i++) {
        Serial.printf("%-12s", counters.getKey(static_cast<Counters>(i)));

        uint32_t val;
        if (!counters.getValue(static_cast<Counters>(i), val)) {
          Serial.printf("%-12s\n", "Error!");
          continue;
        }

        Serial.printf("%-12" PRIu32 "\n", val);
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      // Fires: global (x3), Counter_1 per-setting (x1), Counter_2 per-setting (x1)
      Serial.println("Setting new values (global + Counter_1 + Counter_2 callbacks expected):");

      for (size_t i = 0; i < counters.getSize(); i++) {
        counters.setValue(static_cast<Counters>(i), random(1, 1000));
      }

      Serial.println();
    } break;

    // Format all settings to default values
    case 'f':
    {
      // Fires: Counter_2 per-setting only (callable_on_format=true)
      // Global and Counter_1 are silent (callable_on_format=false)
      Serial.println("Formatting settings (only Counter_2 callback expected):");

      uint8_t errors = counters.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}
