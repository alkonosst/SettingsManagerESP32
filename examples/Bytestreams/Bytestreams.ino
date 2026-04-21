/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three ByteStream settings are created, all with formattable values.
 * - Each setting stores a binary blob tagged with a format hint (Hex, Base64, JSONObject).
 *   The format tag is metadata only - it is not persisted in NVS.
 * - The loop function reads the serial input and performs the following actions:
 *  - '.' restarts the ESP32.
 *  - 'p' prints all settings, including key, hint, format, default value, and current value.
 *  - 's' sets new values for each setting.
 *  - '1' sets a new value only for the first setting.
 *  - 'f' formats all settings to their default values.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Default values (read-only views - data lives in flash)
// Each blob ends with a null byte so it can be printed as a C-string.
const uint8_t bs1_data[]          = {'A', 'A', 'A', '\0'};
const NVS::ByteStreamView bs1_def = {bs1_data, sizeof(bs1_data), NVS::ByteStream::Format::Hex};

const uint8_t bs2_data[]          = {'B', 'B', 'B', '\0'};
const NVS::ByteStreamView bs2_def = {bs2_data, sizeof(bs2_data), NVS::ByteStream::Format::Base64};

const uint8_t bs3_data[]          = {'C', 'C', 'C', '\0'};
const NVS::ByteStreamView bs3_def = {
  bs3_data, sizeof(bs3_data), NVS::ByteStream::Format::JSONObject};

// ByteStreams, all formattable
#define BYTESTREAMS(X)           \
  X(BS_1, "bs 1", bs1_def, true) \
  X(BS_2, "bs 2", bs2_def, true) \
  X(BS_3, "bs 3", bs3_def, true)

// Enum for ByteStream settings
enum class ByteStreams : uint8_t { BYTESTREAMS(SETTINGS_EXPAND_ENUM_CLASS) };

// Settings object for ByteStreams, namespace "esp32"
NVS::Settings<NVS::ByteStream, ByteStreams, SETTINGS_COUNT(BYTESTREAMS)>
  bs("esp32", {BYTESTREAMS(SETTINGS_EXPAND_SETTINGS)});

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!bs.begin()) {
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
      Serial.printf("%-8s%-8s%-12s%-12s%-12s\n", "Key", "Hint", "Format", "Default", "Value");

      static uint8_t buf[64];

      for (size_t i = 0; i < bs.getSize(); i++) {
        NVS::ByteStreamView def = bs.getDefaultValue(static_cast<ByteStreams>(i));

        // Initialize the read buffer with the format tag from the default value
        NVS::ByteStream value{buf, sizeof(buf), def.format};

        Serial.printf("%-8s", bs.getKey(static_cast<ByteStreams>(i)));
        Serial.printf("%-8s", bs.getHint(static_cast<ByteStreams>(i)));
        Serial.printf("%-12s", NVS::formatToStr(def.format));
        Serial.printf("%-12s", reinterpret_cast<const char*>(def.data));

        if (!bs.getValue(static_cast<ByteStreams>(i), value)) {
          Serial.printf("%-12s\n", "Error!");
          continue;
        }

        Serial.printf("%-12s\n", reinterpret_cast<const char*>(value.data));
      }

      Serial.println();
    } break;

    // Set new values for each setting
    case 's':
    {
      Serial.println("Setting new values...");

      static const uint8_t new1[]                 = {'X', 'X', 'X', '\0'};
      static const uint8_t new2[]                 = {'Y', 'Y', 'Y', '\0'};
      static const uint8_t new3[]                 = {'Z', 'Z', 'Z', '\0'};
      static const NVS::ByteStreamView new_vals[] = {
        {new1, sizeof(new1)},
        {new2, sizeof(new2)},
        {new3, sizeof(new3)},
      };

      for (size_t i = 0; i < bs.getSize(); i++) {
        const char* key = bs.getKey(static_cast<ByteStreams>(i));

        if (bs.setValue(static_cast<ByteStreams>(i), new_vals[i])) {
          Serial.printf("- Set %s to %s\n", key, reinterpret_cast<const char*>(new_vals[i].data));
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

      static const uint8_t payload[]           = {'1', '2', '3', '4', '\0'};
      static const NVS::ByteStreamView new_val = {payload, sizeof(payload)};

      if (bs.setValue(ByteStreams::BS_1, new_val)) {
        Serial.printf("- Set BS_1 to %s\n", reinterpret_cast<const char*>(new_val.data));
      } else {
        Serial.println("- Failed to set BS_1");
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