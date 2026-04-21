/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three float settings are created, all with formattable values.
 * - The loop function reads the serial input and performs the following actions:
 *  - '.' restarts the ESP32.
 *  - 'p' prints all settings, including key, hint, default value, and current value.
 *  - 's' sets each setting to a random float value.
 *  - '1' sets a new value only for the first setting.
 *  - 'f' formats all settings to their default values.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Floats, all formattable
#define FLOATS(X)                   \
  X(Float_1, "Float 1", 1.0f, true) \
  X(Float_2, "Float 2", 2.0f, true) \
  X(Float_3, "Float 3", 3.0f, true)

// Enum for float settings
enum class Floats : uint8_t { FLOATS(SETTINGS_EXPAND_ENUM_CLASS) };

// Settings object for floats, namespace "esp32"
NVS::Settings<float, Floats, SETTINGS_COUNT(FLOATS)> floats("esp32",
                                                            {FLOATS(SETTINGS_EXPAND_SETTINGS)});

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!floats.begin()) {
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

      for (size_t i = 0; i < floats.getSize(); i++) {
        Serial.printf("%-10s", floats.getKey(static_cast<Floats>(i)));
        Serial.printf("%-10s", floats.getHint(static_cast<Floats>(i)));
        Serial.printf("%-10.2f", floats.getDefaultValue(static_cast<Floats>(i)));

        float val;
        if (!floats.getValue(static_cast<Floats>(i), val)) {
          Serial.printf("%-10s\n", "Error!");
          continue;
        }

        Serial.printf("%-10.2f\n", val);
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      for (size_t i = 0; i < floats.getSize(); i++) {
        const char* key = floats.getKey(static_cast<Floats>(i));
        float new_value = random(0, 10000) / 100.0f; // 0.00 to 99.99

        if (floats.setValue(static_cast<Floats>(i), new_value)) {
          Serial.printf("- Set %s to %.2f\n", key, new_value);
        } else {
          Serial.printf("- Failed to set value for %s\n", key);
        }
      }

      Serial.println();
    } break;

    // Modify only the first setting, leaving the others unchanged
    case '1':
    {
      Serial.println("Modifying first setting...");

      const char* key = floats.getKey(Floats::Float_1);
      float new_value = random(0, 10000) / 100.0f;

      if (floats.setValue(Floats::Float_1, new_value)) {
        Serial.printf("- Set %s to %.2f\n", key, new_value);
      } else {
        Serial.printf("- Failed to set value for %s\n", key);
      }

      Serial.println();
    } break;

    // Format all settings to default values
    case 'f':
    {
      Serial.print("Formatting settings... ");

      uint8_t errors = floats.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}
