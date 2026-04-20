/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three boolean settings are created, all with formattable values.
 * - The loop function reads the serial input and performs the following actions:
 *  - '.' restarts the ESP32.
 *  - 'p' prints all settings, including key, hint, default value, and current value.
 *  - 's' sets each setting to a random boolean value.
 *  - '1' toggles the first setting.
 *  - 'f' formats all settings to their default values.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Booleans, all formattable
#define BOOLS(X)                   \
  X(Flag_1, "Flag 1", false, true) \
  X(Flag_2, "Flag 2", true, true)  \
  X(Flag_3, "Flag 3", false, true)

// Enum for boolean settings
enum class Bools : uint8_t { BOOLS(SETTINGS_EXPAND_ENUM_CLASS) };

// Settings object for booleans, namespace "esp32"
NVS::Settings<bool, Bools, SETTINGS_COUNT(BOOLS)> bools("esp32", {BOOLS(SETTINGS_EXPAND_SETTINGS)});

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!bools.begin()) {
    Serial.println("Failed to open settings handle!");
    while (true)
      delay(1000);
  }
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
      Serial.println("List of settings:");
      Serial.printf("%-10s%-10s%-10s%-10s\n", "Key", "Hint", "Default", "Value");

      for (size_t i = 0; i < bools.getSize(); i++) {
        Serial.printf("%-10s", bools.getKey(static_cast<Bools>(i)));
        Serial.printf("%-10s", bools.getHint(static_cast<Bools>(i)));
        Serial.printf("%-10s", bools.getDefaultValue(static_cast<Bools>(i)) ? "true" : "false");

        bool val;
        if (!bools.getValue(static_cast<Bools>(i), val)) {
          Serial.printf("%-10s\n", "Error!");
          continue;
        }

        Serial.printf("%-10s\n", val ? "true" : "false");
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      for (size_t i = 0; i < bools.getSize(); i++) {
        const char* key = bools.getKey(static_cast<Bools>(i));
        bool new_value  = random(0, 2);

        if (bools.setValue(static_cast<Bools>(i), new_value)) {
          Serial.printf("- Set %s to %s\n", key, new_value ? "true" : "false");
        } else {
          Serial.printf("- Failed to set value for %s\n", key);
        }
      }

      Serial.println();
    } break;

    // Modify only the first setting, leaving the others unchanged
    case '1':
    {
      Serial.println("Toggling first setting...");

      bool current;
      if (!bools.getValue(Bools::Flag_1, current)) {
        Serial.println("- Failed to read Flag_1");
        break;
      }

      bool toggled = !current;
      if (bools.setValue(Bools::Flag_1, toggled)) {
        Serial.printf(
          "- Flag_1: %s -> %s\n", current ? "true" : "false", toggled ? "true" : "false");
      } else {
        Serial.println("- Failed to set Flag_1");
      }

      Serial.println();
    } break;

    // Format all settings to default values
    case 'f':
    {
      Serial.print("Formatting settings... ");

      uint8_t errors = bools.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}
