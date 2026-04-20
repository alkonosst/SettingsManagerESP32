/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Two ByteStream settings are created, both with formattable values.
 * - The loop function reads the serial input and performs the following actions:
 *   - '.' restarts the ESP32.
 *   - 'p' prints all settings using typeToStr() and formatToStr().
 *   - 'e' encodes the current value of each setting to hex strings (both compact and spaced) using
 *         toHexStr() and hexStrSize().
 *   - 'd' decodes hard-coded hex strings (compact and spaced) using fromHexStr() and saves them.
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Default values (read-only views — data lives in flash)
const uint8_t bs1_data[]          = {0xDE, 0xAD, 0xBE, 0xEF};
const NVS::ByteStreamView bs1_def = {bs1_data, sizeof(bs1_data), NVS::ByteStream::Format::Hex};

const uint8_t bs2_data[]          = {0x01, 0x02, 0x03, 0x04};
const NVS::ByteStreamView bs2_def = {bs2_data, sizeof(bs2_data), NVS::ByteStream::Format::Hex};

// ByteStream settings, all formattable
#define BYTESTREAMS(X)             \
  X(BS_1, "blob 1", bs1_def, true) \
  X(BS_2, "blob 2", bs2_def, true)

SETTINGS_CREATE_BYTE_STREAMS(ByteStreams, "esp32", BYTESTREAMS)

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!st_ByteStreams.begin()) {
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
    case '.': ESP.restart(); break;

    case 'p':
    {
      Serial.printf("Settings type: %s\n\n", NVS::typeToStr(st_ByteStreams.getType()));

      static uint8_t buf[32];

      // Allocate exact buffer sizes using hexStrSize()
      static char compact_buf[NVS::hexStrSize(32)];
      static char spaced_buf[NVS::hexStrSize(32, true)];

      for (size_t i = 0; i < st_ByteStreams.getSize(); i++) {
        NVS::ByteStreamView def = st_ByteStreams.getDefaultValue(static_cast<ByteStreams>(i));

        Serial.printf("Key    : %s\n", st_ByteStreams.getKey(static_cast<ByteStreams>(i)));
        Serial.printf("Hint   : %s\n", st_ByteStreams.getHint(static_cast<ByteStreams>(i)));
        Serial.printf("Format : %s\n", NVS::formatToStr(def.format));

        if (NVS::fromHexToStr(def, compact_buf, sizeof(compact_buf))) {
          Serial.printf("Default: %s\n", compact_buf);
        }

        NVS::ByteStream value{buf, sizeof(buf), def.format};
        if (st_ByteStreams.getValue(static_cast<ByteStreams>(i), value)) {
          NVS::fromHexToStr(value, compact_buf, sizeof(compact_buf));
          NVS::fromHexToStr(value, spaced_buf, sizeof(spaced_buf), true);
          Serial.printf("Value  : %s (%s)\n", compact_buf, spaced_buf);
        } else {
          Serial.printf("Value  : (not saved)\n");
        }

        Serial.println();
      }
    } break;

    case 'e':
    {
      Serial.println("Encoding current values to hex strings:");

      static uint8_t buf[32];

      // Allocate exact buffer sizes using hexStrSize()
      static char compact_buf[NVS::hexStrSize(32)];
      static char spaced_buf[NVS::hexStrSize(32, true)];

      for (size_t i = 0; i < st_ByteStreams.getSize(); i++) {
        const char* key         = st_ByteStreams.getKey(static_cast<ByteStreams>(i));
        NVS::ByteStreamView def = st_ByteStreams.getDefaultValue(static_cast<ByteStreams>(i));
        NVS::ByteStream value{buf, sizeof(buf), def.format};

        if (!st_ByteStreams.getValue(static_cast<ByteStreams>(i), value)) {
          Serial.printf("- %s: (not saved)\n", key);
          continue;
        }

        NVS::fromHexToStr(value, compact_buf, sizeof(compact_buf));
        NVS::fromHexToStr(value, spaced_buf, sizeof(spaced_buf), true);
        Serial.printf("- %s: %s (%s)\n", key, compact_buf, spaced_buf);
      }

      Serial.println();
    } break;

    case 'd':
    {
      Serial.println("Decoding hex strings and saving...");

      // First setting: compact format ("CAFEBABE")
      // Second setting: spaced format ("0A 0B 0C 0D")
      static const char* hex_compact = "CAFEBABE";
      static const char* hex_spaced  = "0A 0B 0C 0D";

      static uint8_t buf[32];

      // Decode compact hex string
      {
        const char* key = st_ByteStreams.getKey(static_cast<ByteStreams>(0));
        NVS::ByteStream value{buf, sizeof(buf)};

        if (!NVS::fromStrToHex(hex_compact, value)) {
          Serial.printf("- %s: failed to decode \"%s\"\n", key, hex_compact);
        } else if (st_ByteStreams.setValue(static_cast<ByteStreams>(0), value)) {
          Serial.printf("- %s: saved \"%s\"\n", key, hex_compact);
        } else {
          Serial.printf("- %s: failed to save\n", key);
        }
      }

      // Decode spaced hex string
      {
        const char* key = st_ByteStreams.getKey(static_cast<ByteStreams>(1));
        NVS::ByteStream value{buf, sizeof(buf)};

        if (!NVS::fromStrToHex(hex_spaced, value, true)) {
          Serial.printf("- %s: failed to decode \"%s\"\n", key, hex_spaced);
        } else if (st_ByteStreams.setValue(static_cast<ByteStreams>(1), value)) {
          Serial.printf("- %s: saved \"%s\"\n", key, hex_spaced);
        } else {
          Serial.printf("- %s: failed to save\n", key);
        }
      }

      Serial.println();
    } break;

    default: break;
  }
}
