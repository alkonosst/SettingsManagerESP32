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

// Example of Bytestream values with string data (size includes null terminator)

// Default values
// - Format options: Hex, Base64, JSONObject, JSONArray
// - Each one has a null terminator included in the size for easier printing
const uint8_t bs1_default_data[]  = {0x31, 0x31, 0x31, 0x00}; // "111"
const NVS::ByteStream bs1_default = {
  bs1_default_data, sizeof(bs1_default_data), NVS::ByteStream::Format::Hex};

const char bs2_default_data[]     = "MTEx"; // "111" in Base64
const NVS::ByteStream bs2_default = {reinterpret_cast<const uint8_t*>(bs2_default_data),
                                     sizeof(bs2_default_data),
                                     NVS::ByteStream::Format::Base64};

const char bs3_default_data[]     = "{\"value\": 111}";
const NVS::ByteStream bs3_default = {reinterpret_cast<const uint8_t*>(bs3_default_data),
                                     sizeof(bs3_default_data),
                                     NVS::ByteStream::Format::JSONObject};

// New values
const uint8_t bs1_new_data[]  = {0x32, 0x32, 0x32, 0x00}; // "222"
const NVS::ByteStream bs1_new = {reinterpret_cast<const uint8_t*>(bs1_new_data),
                                 sizeof(bs1_new_data),
                                 NVS::ByteStream::Format::Hex};

const char bs2_new_data[]     = "MjIy"; // "222" in Base64
const NVS::ByteStream bs2_new = {reinterpret_cast<const uint8_t*>(bs2_new_data),
                                 sizeof(bs2_new_data),
                                 NVS::ByteStream::Format::Base64};

const char bs3_new_data[]     = "{\"value\": 222}";
const NVS::ByteStream bs3_new = {reinterpret_cast<const uint8_t*>(bs3_new_data),
                                 sizeof(bs3_new_data),
                                 NVS::ByteStream::Format::JSONObject};

const NVS::ByteStream* bs_new_values[] = {&bs1_new, &bs2_new, &bs3_new};

// Bytestreams, all formattable
#define BYTESTREAMS(X)               \
  X(BS_1, "bs 1", bs1_default, true) \
  X(BS_2, "bs 2", bs2_default, true) \
  X(BS_3, "bs 3", bs3_default, true)

enum class ByteStreams : uint8_t { BYTESTREAMS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<NVS::ByteStream, ByteStreams, SETTINGS_COUNT(BYTESTREAMS)> bs = {
  BYTESTREAMS(SETTINGS_EXPAND_SETTINGS)};

const char* bytestreamFormatToString(NVS::ByteStream::Format format) {
  switch (format) {
    case NVS::ByteStream::Format::Hex: return "Hex";
    case NVS::ByteStream::Format::Base64: return "Base64";
    case NVS::ByteStream::Format::JSONObject: return "JsonObject";
    case NVS::ByteStream::Format::JSONArray: return "JsonArray";
    default: return "Unknown";
  }
}

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

      for (size_t i = 0; i < bs.getSize(); i++) {
        Serial.println("-------------------------");
        Serial.printf("ByteStream %u:\n", i + 1);
        Serial.printf("- Key: %s\n", bs.getKey(static_cast<ByteStreams>(i)));
        Serial.printf("- Hint: %s\n", bs.getHint(static_cast<ByteStreams>(i)));

        NVS::ByteStream def_value = bs.getDefaultValue(static_cast<ByteStreams>(i));
        Serial.printf("- Format: %s\n", bytestreamFormatToString(def_value.format));
        Serial.printf("- Default value: %s (%u)\n", def_value.data, def_value.size);

        NVS::ByteStream value = bs.getValue(static_cast<ByteStreams>(i));
        Serial.printf("- Current value: %s (%u)\n", value.data, value.size);

        // IMPORTANT: release the mutex when done using a value
        bs.giveMutex();
      }

      Serial.println("-------------------------");
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

    // Print Bytestream 1 (hex format)
    case '1':
    {
      NVS::ByteStream value = bs.getValue(ByteStreams::BS_1);
      Serial.printf("Bytestream 1 (%s): %s (%u)\n\n",
                    bytestreamFormatToString(value.format),
                    value.data,
                    value.size);
      bs.giveMutex();
    } break;

    // Print Bytestream 2 (base64 format)
    case '2':
    {
      NVS::ByteStream value = bs.getValue(ByteStreams::BS_2);
      Serial.printf("Bytestream 2 (%s): %s (%u)\n\n",
                    bytestreamFormatToString(value.format),
                    value.data,
                    value.size);
      bs.giveMutex();
    } break;

    // Print Bytestream 3 (JSON object format)
    case '3':
    {
      NVS::ByteStream value = bs.getValue(ByteStreams::BS_3);
      Serial.printf("Bytestream 3 (%s): %s (%u)\n\n",
                    bytestreamFormatToString(value.format),
                    value.data,
                    value.size);
      bs.giveMutex();
    } break;
  }
}