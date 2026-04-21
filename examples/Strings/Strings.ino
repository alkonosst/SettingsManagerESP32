/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three settings are created, all with formattable values.
 * - The loop function reads the serial input and performs the following actions:
 *  - '.' restarts the ESP32.
 *  - 'p' prints all settings, including key, hint, default value, and current value.
 *  - 's' sets new values for each setting.
 *  - '1' sets a new value only for the first setting.
 *  - 'f' formats all settings to their default values.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Strings, all formattable
#define STRINGS(X)                 \
  X(Str_1, "Str 1", "str 1", true) \
  X(Str_2, "Str 2", "str 2", true) \
  X(Str_3, "Str 3", "str 3", true)

// Enum for string settings
enum class Strings : uint8_t { STRINGS(SETTINGS_EXPAND_ENUM_CLASS) };

// Settings object for strings, namespace "esp32"
NVS::Settings<NVS::Str, Strings, SETTINGS_COUNT(STRINGS)>
  strings("esp32", {STRINGS(SETTINGS_EXPAND_SETTINGS)});

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  // Initialize NVS flash partition, then open the namespace handle
  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!strings.begin()) {
    Serial.println("Failed to open settings handle!");
    while (true)
      delay(1000);
  }
}

void loop() {
  // Read serial input
  if (!Serial.available()) {
    return;
  }

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
      Serial.printf("%-10s%-10s%-10s%-10s\n", "Key", "Hint", "DefVal", "Value");

      static char buf[32];
      static NVS::Str value{buf, sizeof(buf)};

      for (size_t i = 0; i < strings.getSize(); i++) {
        Serial.printf("%-10s", strings.getKey(static_cast<Strings>(i)));
        Serial.printf("%-10s", strings.getHint(static_cast<Strings>(i)));
        Serial.printf("%-10s", strings.getDefaultValue(static_cast<Strings>(i)).data);

        if (!strings.getValue(static_cast<Strings>(i), value)) {
          Serial.printf("%-10s\n", "Error!");
          continue;
        }

        Serial.printf("%-10s\n", value.data);
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      static char buffer[32];
      Serial.println("Setting new values...");

      for (size_t i = 0; i < strings.getSize(); i++) {
        const char* key = strings.getKey(static_cast<Strings>(i));

        uint32_t new_value = random(0, 100);
        snprintf(buffer, sizeof(buffer), "str %" PRIu32, new_value);

        if (strings.setValue(static_cast<Strings>(i), buffer)) {
          Serial.printf("- Set %s to %s\n", key, buffer);
        } else {
          Serial.printf("- Failed to set value for %s\n", key);
        }
      }

      Serial.println();
    } break;

    // Modify only the first setting, leaving the others unchanged
    case '1':
    {
      static char buffer[32];
      Serial.println("Modifying first setting...");

      const char* key = strings.getKey(Strings::Str_1);

      uint32_t new_value = random(0, 100);
      snprintf(buffer, sizeof(buffer), "str %" PRIu32, new_value);

      if (strings.setValue(Strings::Str_1, buffer)) {
        Serial.printf("- Set %s to %s\n", key, buffer);
      } else {
        Serial.printf("- Failed to set value for %s\n", key);
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