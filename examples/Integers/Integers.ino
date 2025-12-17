/**
 * SPDX-FileCopyrightText: 2025 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three settings are created, all with formattable values.
 * - The loop function reads the serial input and performs the following actions:
 *  - '.' restarts the ESP32.
 *  - 'p' prints all settings, including key, hint, default value, and current value.
 *  - 's' sets new values for each setting.
 *  - 'f' formats all settings to their default values.
 */

#include "SettingsManagerESP32.h"

// Integers, all formattable
#define UINTS(X)               \
  X(UInt_1, "UInt 1", 1, true) \
  X(UInt_2, "UInt 2", 2, true) \
  X(UInt_3, "UInt 3", 3, true)

enum class UInts : uint8_t { UINTS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<uint32_t, UInts, SETTINGS_COUNT(UINTS)> uints = {UINTS(SETTINGS_EXPAND_SETTINGS)};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // Initialize NVS namespace
  nvs.begin("esp32");
}

void loop() {
  // Read serial input
  if (!Serial.available()) {
    return;
  }

  char c = Serial.read();

  switch (c) {
    // Restart ESP32
    case '.': ESP.restart(); break;

    // Print all settings
    case 'p':
    {
      Serial.println("List of settings:");

      Serial.printf("Key\t\tHint\t\tDefVal\t\tValue\n");

      for (size_t i = 0; i < uints.getSize(); i++) {
        Serial.printf("%s\t\t", uints.getKey(static_cast<UInts>(i)));
        Serial.printf("%s\t\t", uints.getHint(static_cast<UInts>(i)));
        Serial.printf("%" PRIu32 "\t\t", uints.getDefaultValue(static_cast<UInts>(i)));
        Serial.printf("%" PRIu32 "\n", uints.getValue(static_cast<UInts>(i)));
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      for (size_t i = 0; i < uints.getSize(); i++) {
        const char* key    = uints.getKey(static_cast<UInts>(i));
        uint32_t new_value = random(0, 100);

        if (uints.setValue(static_cast<UInts>(i), new_value)) {
          Serial.printf("- Set %s to %" PRIu32 "\n", key, new_value + i);
        } else {
          Serial.printf("- Failed to set value for %s\n", key);
        }
      }

      Serial.println();
    } break;

    // Format all settings to default values
    case 'f':
    {
      Serial.print("Formatting settings... ");

      uint8_t errors = uints.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}