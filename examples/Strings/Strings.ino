/**
 * SPDX-FileCopyrightText: 2025 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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

// Strings, all formattable
#define STRINGS(X)                 \
  X(Str_1, "Str 1", "str 1", true) \
  X(Str_2, "Str 2", "str 2", true) \
  X(Str_3, "Str 3", "str 3", true)

enum class Strings : uint8_t { STRINGS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<const char*, Strings, SETTINGS_COUNT(STRINGS)> strings = {
  STRINGS(SETTINGS_EXPAND_SETTINGS)};

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

      for (size_t i = 0; i < strings.getSize(); i++) {
        Serial.printf("%s\t\t", strings.getKey(static_cast<Strings>(i)));
        Serial.printf("%s\t\t", strings.getHint(static_cast<Strings>(i)));
        Serial.printf("%s\t\t", strings.getDefaultValue(static_cast<Strings>(i)));
        Serial.printf("%s\n", strings.getValue(static_cast<Strings>(i)));

        // IMPORTANT: release the mutex when done using a value
        strings.giveMutex();
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      for (size_t i = 0; i < strings.getSize(); i++) {
        const char* key = strings.getKey(static_cast<Strings>(i));

        static char buffer[SETTINGS_STRING_BUFFER_SIZE];
        uint32_t new_value = random(0, 100);

        snprintf(buffer, sizeof(buffer), "str %u", new_value);

        if (strings.setValue(static_cast<Strings>(i), buffer)) {
          Serial.printf("- Set %s to %s\n", key, buffer);
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

      uint8_t errors = strings.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}