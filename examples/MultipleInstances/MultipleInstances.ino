/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Two separate Settings instances are created with the same settings list but different
 * namespaces.
 * - The loop function reads the serial input and performs the following actions:
 * - '.' restarts the ESP32.
 * - 'p' prints all settings for both instances, demonstrating that they operate independently.
 * - 's' sets new random values for each setting in both instances.
 * - '1' sets a new random value only for the first setting in both instances.
 * - 'f' formats all settings in both instances to their default values.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Integers, all formattable
#define UINTS(X)               \
  X(UInt_1, "UInt 1", 1, true) \
  X(UInt_2, "UInt 2", 2, true) \
  X(UInt_3, "UInt 3", 3, true)

// Enum for integer settings
enum class UInts : uint8_t { UINTS(SETTINGS_EXPAND_ENUM_CLASS) };

// Two instances using the same settings list but different namespaces, demonstrating that they
// operate independently
// clang-format off
NVS::Settings<uint32_t, UInts, SETTINGS_COUNT(UINTS)>
  uints1("uints1",{UINTS(SETTINGS_EXPAND_SETTINGS)});

NVS::Settings<uint32_t, UInts, SETTINGS_COUNT(UINTS)>
  uints2("uints2",{UINTS(SETTINGS_EXPAND_SETTINGS)});
// clang-format on

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!uints1.begin()) {
    Serial.println("Failed to open uints1 settings handle!");
    while (true)
      delay(1000);
  }

  if (!uints2.begin()) {
    Serial.println("Failed to open uints2 settings handle!");
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
      Serial.printf("%-10s%-10s%-10s%-10s%-10s\n", "Namespace", "Key", "Hint", "Default", "Value");

      for (size_t i = 0; i < uints1.getSize(); i++) {
        Serial.printf("%-10s", uints1.getNamespace());
        Serial.printf("%-10s", uints1.getKey(static_cast<UInts>(i)));
        Serial.printf("%-10s", uints1.getHint(static_cast<UInts>(i)));
        Serial.printf("%-10" PRIu32, uints1.getDefaultValue(static_cast<UInts>(i)));

        uint32_t val;
        if (!uints1.getValue(static_cast<UInts>(i), val)) {
          Serial.printf("%-10s\n", "Error!");
          continue;
        }

        Serial.printf("%-10" PRIu32 "\n", val);
      }

      for (size_t i = 0; i < uints2.getSize(); i++) {
        Serial.printf("%-10s", uints2.getNamespace());
        Serial.printf("%-10s", uints2.getKey(static_cast<UInts>(i)));
        Serial.printf("%-10s", uints2.getHint(static_cast<UInts>(i)));
        Serial.printf("%-10" PRIu32, uints2.getDefaultValue(static_cast<UInts>(i)));

        uint32_t val;
        if (!uints2.getValue(static_cast<UInts>(i), val)) {
          Serial.printf("%-10s\n", "Error!");
          continue;
        }

        Serial.printf("%-10" PRIu32 "\n", val);
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      for (size_t i = 0; i < uints1.getSize(); i++) {
        const char* key    = uints1.getKey(static_cast<UInts>(i));
        uint32_t new_value = random(0, 100);

        if (uints1.setValue(static_cast<UInts>(i), new_value)) {
          Serial.printf("- Set %s/%s to %" PRIu32 "\n", uints1.getNamespace(), key, new_value);
        } else {
          Serial.printf("- Failed to set value for %s/%s\n", uints1.getNamespace(), key);
        }
      }

      for (size_t i = 0; i < uints2.getSize(); i++) {
        const char* key    = uints2.getKey(static_cast<UInts>(i));
        uint32_t new_value = random(0, 100);

        if (uints2.setValue(static_cast<UInts>(i), new_value)) {
          Serial.printf("- Set %s/%s to %" PRIu32 "\n", uints2.getNamespace(), key, new_value);
        } else {
          Serial.printf("- Failed to set value for %s/%s\n", uints2.getNamespace(), key);
        }
      }

      Serial.println();
    } break;

    // Modify only the first setting, leaving the others unchanged
    case '1':
    {
      Serial.println("Modifying first setting...");

      // Instance 1
      const char* key    = uints1.getKey(UInts::UInt_1);
      uint32_t new_value = random(0, 100);

      if (uints1.setValue(UInts::UInt_1, new_value)) {
        Serial.printf("- Set %s/%s to %" PRIu32 "\n", uints1.getNamespace(), key, new_value);
      } else {
        Serial.printf("- Failed to set value for %s/%s\n", uints1.getNamespace(), key);
      }

      // Instance 2
      key       = uints2.getKey(UInts::UInt_1);
      new_value = random(0, 100);

      if (uints2.setValue(UInts::UInt_1, new_value)) {
        Serial.printf("- Set %s/%s to %" PRIu32 "\n", uints2.getNamespace(), key, new_value);
      } else {
        Serial.printf("- Failed to set value for %s/%s\n", uints2.getNamespace(), key);
      }

      Serial.println();
    } break;

    // Format all settings to default values
    case 'f':
    {
      Serial.print("Formatting settings... ");

      uint8_t errors = uints1.formatAll();
      errors += uints2.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}