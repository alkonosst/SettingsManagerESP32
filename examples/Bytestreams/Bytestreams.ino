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

// Default values
const NVS::ByteStream bs1_default = {reinterpret_cast<const uint8_t*>("111"), 3};
const NVS::ByteStream bs2_default = {reinterpret_cast<const uint8_t*>("222"), 3};
const NVS::ByteStream bs3_default = {reinterpret_cast<const uint8_t*>("333"), 3};

// New values
const NVS::ByteStream bs1_new = {reinterpret_cast<const uint8_t*>("aaaaaa"), 6};
const NVS::ByteStream bs2_new = {reinterpret_cast<const uint8_t*>("bbbbbb"), 6};
const NVS::ByteStream bs3_new = {reinterpret_cast<const uint8_t*>("cccccc"), 6};

const NVS::ByteStream* bs_new_values[] = {&bs1_new, &bs2_new, &bs3_new};

// Bytestreams, all formattable
#define BYTESTREAMS(X)               \
  X(BS_1, "bs 1", bs1_default, true) \
  X(BS_2, "bs 2", bs2_default, true) \
  X(BS_3, "bs 3", bs3_default, true)

enum class ByteStreams : uint8_t { BYTESTREAMS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<NVS::ByteStream, ByteStreams> bs = {BYTESTREAMS(SETTINGS_EXPAND_SETTINGS)};

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

      for (size_t i = 0; i < bs.getSize(); i++) {
        Serial.printf("%s\t\t", bs.getKey(static_cast<ByteStreams>(i)));
        Serial.printf("%s\t\t", bs.getHint(static_cast<ByteStreams>(i)));

        NVS::ByteStream def_value = bs.getDefaultValue(static_cast<ByteStreams>(i));
        Serial.printf("%s (%u)\t\t", def_value.data, def_value.size);

        NVS::ByteStream value = bs.getValue(static_cast<ByteStreams>(i));
        Serial.printf("%s (%u)\n", value.data, value.size);

        // IMPORTANT: release the mutex when done using a value
        bs.giveMutex();
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      for (size_t i = 0; i < bs.getSize(); i++) {
        const char* key = bs.getKey(static_cast<ByteStreams>(i));

        if (bs.setValue(static_cast<ByteStreams>(i), *bs_new_values[i])) {
          Serial.printf("- Set %s to %s\n", key, bs_new_values[i]->data);
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

      uint8_t errors = bs.formatAll();

      if (errors == 0) {
        Serial.println("done.\n");
      } else {
        Serial.printf("failed with %u errors!\n\n", errors);
      }
    } break;
  }
}