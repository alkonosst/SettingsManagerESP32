/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - This example demonstrates how to use Settings objects within a custom class. This is useful for
 *   encapsulating related settings and logic together.
 * - A class `MyDevice` is defined, which contains a Settings object for uint32_t values. The
 *   constructor takes a namespace name, allowing multiple instances of `MyDevice` to coexist with
 *   their own separate settings.
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

// Example class demonstrating usage of Settings within a custom class.
// Each instance instantiates its own Settings object with a different namespace, demonstrating that
// multiple instances can coexist without interference.
class MyDevice {
  public:
  using Prefs = NVS::Settings<uint32_t, UInts, SETTINGS_COUNT(UINTS)>;

  MyDevice(const char* ns_name)
      : _uints(ns_name, {UINTS(SETTINGS_EXPAND_SETTINGS)}) {}

  Prefs& getSettings() { return _uints; }

  private:
  Prefs _uints;
};

// Create two devices with different namespaces
MyDevice device1("device1");
MyDevice device2("device2");

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!device1.getSettings().begin()) {
    Serial.println("Failed to open device1 settings handle!");
    while (true)
      delay(1000);
  }

  if (!device2.getSettings().begin()) {
    Serial.println("Failed to open device2 settings handle!");
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

      for (size_t i = 0; i < device1.getSettings().getSize(); i++) {
        Serial.printf("%-10s", device1.getSettings().getNamespace());
        Serial.printf("%-10s", device1.getSettings().getKey(static_cast<UInts>(i)));
        Serial.printf("%-10s", device1.getSettings().getHint(static_cast<UInts>(i)));
        Serial.printf("%-10" PRIu32, device1.getSettings().getDefaultValue(static_cast<UInts>(i)));

        uint32_t val;
        if (!device1.getSettings().getValue(static_cast<UInts>(i), val)) {
          Serial.printf("%-10s\n", "Error!");
          continue;
        }

        Serial.printf("%-10" PRIu32 "\n", val);
      }

      for (size_t i = 0; i < device2.getSettings().getSize(); i++) {
        Serial.printf("%-10s", device2.getSettings().getNamespace());
        Serial.printf("%-10s", device2.getSettings().getKey(static_cast<UInts>(i)));
        Serial.printf("%-10s", device2.getSettings().getHint(static_cast<UInts>(i)));
        Serial.printf("%-10" PRIu32, device2.getSettings().getDefaultValue(static_cast<UInts>(i)));

        uint32_t val;
        if (!device2.getSettings().getValue(static_cast<UInts>(i), val)) {
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

      for (size_t i = 0; i < device1.getSettings().getSize(); i++) {
        const char* key    = device1.getSettings().getKey(static_cast<UInts>(i));
        uint32_t new_value = random(0, 100);

        if (device1.getSettings().setValue(static_cast<UInts>(i), new_value)) {
          Serial.printf(
            "- Set %s/%s to %" PRIu32 "\n", device1.getSettings().getNamespace(), key, new_value);
        } else {
          Serial.printf(
            "- Failed to set value for %s/%s\n", device1.getSettings().getNamespace(), key);
        }
      }

      for (size_t i = 0; i < device2.getSettings().getSize(); i++) {
        const char* key    = device2.getSettings().getKey(static_cast<UInts>(i));
        uint32_t new_value = random(0, 100);

        if (device2.getSettings().setValue(static_cast<UInts>(i), new_value)) {
          Serial.printf(
            "- Set %s/%s to %" PRIu32 "\n", device2.getSettings().getNamespace(), key, new_value);
        } else {
          Serial.printf(
            "- Failed to set value for %s/%s\n", device2.getSettings().getNamespace(), key);
        }
      }

      Serial.println();
    } break;

    // Modify only the first setting, leaving the others unchanged
    case '1':
    {
      Serial.println("Modifying first setting...");

      // Instance 1
      const char* key    = device1.getSettings().getKey(UInts::UInt_1);
      uint32_t new_value = random(0, 100);

      if (device1.getSettings().setValue(UInts::UInt_1, new_value)) {
        Serial.printf(
          "- Set %s/%s to %" PRIu32 "\n", device1.getSettings().getNamespace(), key, new_value);
      } else {
        Serial.printf(
          "- Failed to set value for %s/%s\n", device1.getSettings().getNamespace(), key);
      }

      // Instance 2
      key       = device2.getSettings().getKey(UInts::UInt_1);
      new_value = random(0, 100);

      if (device2.getSettings().setValue(UInts::UInt_1, new_value)) {
        Serial.printf(
          "- Set %s/%s to %" PRIu32 "\n", device2.getSettings().getNamespace(), key, new_value);
      } else {
        Serial.printf(
          "- Failed to set value for %s/%s\n", device2.getSettings().getNamespace(), key);
      }

      Serial.println();
    } break;

    // Format all settings to default values
    case 'f':
    {
      Serial.print("Formatting settings... ");

      uint8_t errors = device1.getSettings().formatAll();
      errors += device2.getSettings().formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}