/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

/** Explanation of the example:
 * - Three settings are created for each of: uint32_t, NVS::Str, and NVS::ByteStream.
 * - In setup(), the NVS partition is fully erased and re-initialized, so no keys exist.
 * - The loop function reads the serial input and performs the following actions:
 *   - '.' restarts the ESP32.
 *   - 'p' prints all settings using getValueOrDefault() (typed API) and getValuePtrOrDefault()
 *         (ISettings pointer API). getValuePtrOrDefault() returns true on success (NVS or default)
 *         and writes the value into the caller's buffer. Since no key exists yet, both functions
 *         return the default value as fallback. After writing values with 's', they return the
 *         actual NVS values instead.
 *   - 's' sets new values for the first two settings of each type (the third is intentionally
 *         left unwritten to keep the fallback case observable).
 */

#include <Arduino.h>

#include "SettingsManagerESP32.h"

// Three uint32_t settings, none formattable
#define MY_UINTS(X)                \
  X(Val_1, "Value 1", 100u, false) \
  X(Val_2, "Value 2", 200u, false) \
  X(Val_3, "Value 3", 300u, false)

SETTINGS_CREATE_UINT32S(MyUInts, "ordefault", MY_UINTS)

// Three string settings, none formattable
#define MY_STRS(X)                         \
  X(Str_1, "String 1", "default_1", false) \
  X(Str_2, "String 2", "default_2", false) \
  X(Str_3, "String 3", "default_3", false)

SETTINGS_CREATE_STRINGS(MyStrs, "ordefault", MY_STRS)

// Default byte blobs (stored in flash)
const uint8_t bs1_data[] = {0xAA, 0xBB, 0xCC};
const uint8_t bs2_data[] = {0x11, 0x22, 0x33};
const uint8_t bs3_data[] = {0xDE, 0xAD, 0xBE};

const NVS::ByteStreamView bs1_def{bs1_data, sizeof(bs1_data)};
const NVS::ByteStreamView bs2_def{bs2_data, sizeof(bs2_data)};
const NVS::ByteStreamView bs3_def{bs3_data, sizeof(bs3_data)};

// Three ByteStream settings, none formattable
#define MY_BYTESTREAMS(X)                 \
  X(BS_1, "ByteStream 1", bs1_def, false) \
  X(BS_2, "ByteStream 2", bs2_def, false) \
  X(BS_3, "ByteStream 3", bs3_def, false)

SETTINGS_CREATE_BYTE_STREAMS(MyByteStreams, "ordefault", MY_BYTESTREAMS)

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting...");

  if (!NVS::init()) {
    Serial.println("Failed to initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!st_MyUInts.begin() || !st_MyStrs.begin() || !st_MyByteStreams.begin()) {
    Serial.println("Failed to open settings handles!");
    while (true)
      delay(1000);
  }

  // Handles must be closed before erasing the partition
  st_MyUInts.end();
  st_MyStrs.end();
  st_MyByteStreams.end();

  Serial.println("Erasing NVS partition...");
  if (!NVS::erase()) {
    Serial.println("Failed to erase NVS!");
    while (true)
      delay(1000);
  }

  Serial.println("Re-initializing NVS...");
  if (!NVS::init()) {
    Serial.println("Failed to re-initialize NVS!");
    while (true)
      delay(1000);
  }

  if (!st_MyUInts.begin() || !st_MyStrs.begin() || !st_MyByteStreams.begin()) {
    Serial.println("Failed to re-open settings handles!");
    while (true)
      delay(1000);
  }

  Serial.println("NVS is now empty. No keys exist.");
  Serial.println("Press 'p' to print values, 's' to set values.\n");
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

    case 'p':
    {
      // ---- uint32_t ----
      Serial.println("=== uint32_t ===");

      Serial.println("Typed API: getValueOrDefault()");
      Serial.printf("%-10s %-12s %-12s\n", "Key", "Default", "Result");
      for (size_t i = 0; i < st_MyUInts.getSize(); i++) {
        uint32_t out;
        uint32_t result = st_MyUInts.getValueOrDefault(static_cast<MyUInts>(i), out);
        Serial.printf("%-10s %-12" PRIu32 " %-12" PRIu32 "\n",
                      st_MyUInts.getKey(static_cast<MyUInts>(i)),
                      st_MyUInts.getDefaultValue(static_cast<MyUInts>(i)),
                      result);
      }

      Serial.println("ISettings API: getValuePtrOrDefault()");
      NVS::ISettings* ptr = &st_MyUInts;
      Serial.printf("%-10s %-12s %-12s\n", "Key", "Default", "Result");
      for (size_t i = 0; i < ptr->getSize(); i++) {
        uint32_t out;
        bool ok = ptr->getValuePtrOrDefault(i, &out, sizeof(out));
        if (ok) {
          Serial.printf("%-10s %-12" PRIu32 " %-12" PRIu32 "\n",
                        ptr->getKey(i),
                        ptr->getDefaultValueAs<uint32_t>(i),
                        out);
        }
      }

      Serial.println();

      // ---- NVS::Str ----
      Serial.println("=== NVS::Str ===");
      static char str_buf[32];

      Serial.println("Typed API: getValueOrDefault()");
      Serial.printf("%-10s %-12s %-12s\n", "Key", "Default", "Result");
      for (size_t i = 0; i < st_MyStrs.getSize(); i++) {
        NVS::Str out{str_buf, sizeof(str_buf)};
        NVS::Str result = st_MyStrs.getValueOrDefault(static_cast<MyStrs>(i), out);
        Serial.printf("%-10s %-12s %-12s\n",
                      st_MyStrs.getKey(static_cast<MyStrs>(i)),
                      st_MyStrs.getDefaultValue(static_cast<MyStrs>(i)).data,
                      result.data);
      }

      Serial.println("ISettings API: getValuePtrOrDefault()");
      ptr = &st_MyStrs;
      Serial.printf("%-10s %-12s %-12s\n", "Key", "Default", "Result");
      for (size_t i = 0; i < ptr->getSize(); i++) {
        NVS::Str out{str_buf, sizeof(str_buf)};
        bool ok = ptr->getValuePtrOrDefault(i, &out, sizeof(out));
        if (ok) {
          Serial.printf("%-10s %-12s %-12s\n",
                        ptr->getKey(i),
                        ptr->getDefaultValueAs<NVS::StrView>(i).data,
                        out.data);
        }
      }

      Serial.println();

      // ---- NVS::ByteStream ----
      Serial.println("=== NVS::ByteStream ===");
      static uint8_t bs_buf[16];
      static char hex_buf[NVS::hexStrSize(16)];

      Serial.println("Typed API: getValueOrDefault()");
      Serial.printf("%-10s %-12s %-12s\n", "Key", "Default", "Result");
      for (size_t i = 0; i < st_MyByteStreams.getSize(); i++) {
        NVS::ByteStream out{bs_buf, sizeof(bs_buf)};
        NVS::ByteStream result =
          st_MyByteStreams.getValueOrDefault(static_cast<MyByteStreams>(i), out);

        NVS::ByteStreamView def = st_MyByteStreams.getDefaultValue(static_cast<MyByteStreams>(i));
        NVS::fromHexToStr(def, hex_buf, sizeof(hex_buf));
        char def_str[NVS::hexStrSize(16)];
        memcpy(def_str, hex_buf, sizeof(def_str));

        NVS::fromHexToStr(result, hex_buf, sizeof(hex_buf));
        Serial.printf("%-10s %-12s %-12s\n",
                      st_MyByteStreams.getKey(static_cast<MyByteStreams>(i)),
                      def_str,
                      hex_buf);
      }

      Serial.println("ISettings API: getValuePtrOrDefault()");
      ptr = &st_MyByteStreams;
      Serial.printf("%-10s %-12s %-12s\n", "Key", "Default", "Result");
      for (size_t i = 0; i < ptr->getSize(); i++) {
        NVS::ByteStream out{bs_buf, sizeof(bs_buf)};
        bool ok = ptr->getValuePtrOrDefault(i, &out, sizeof(out));
        if (ok) {
          NVS::ByteStreamView def = ptr->getDefaultValueAs<NVS::ByteStreamView>(i);
          NVS::fromHexToStr(def, hex_buf, sizeof(hex_buf));
          char def_str[NVS::hexStrSize(16)];
          memcpy(def_str, hex_buf, sizeof(def_str));

          NVS::fromHexToStr(out, hex_buf, sizeof(hex_buf));
          Serial.printf("%-10s %-12s %-12s\n", ptr->getKey(i), def_str, hex_buf);
        }
      }

      Serial.println();
    } break;

    case 's':
    {
      // The third setting of each type is intentionally left unwritten
      Serial.println("Setting uint32 Val_1=999, Val_2=888...");
      st_MyUInts.setValue(MyUInts::Val_1, 999u);
      st_MyUInts.setValue(MyUInts::Val_2, 888u);

      Serial.println("Setting string Str_1=\"hello_1\", Str_2=\"hello_2\"...");
      st_MyStrs.setValue(MyStrs::Str_1, NVS::StrView{"hello_1"});
      st_MyStrs.setValue(MyStrs::Str_2, NVS::StrView{"hello_2"});

      const uint8_t new_bs1[] = {0x01, 0x02, 0x03};
      const uint8_t new_bs2[] = {0x04, 0x05, 0x06};
      Serial.println("Setting ByteStream BS_1=010203, BS_2=040506...");
      st_MyByteStreams.setValue(MyByteStreams::BS_1, NVS::ByteStreamView{new_bs1, sizeof(new_bs1)});
      st_MyByteStreams.setValue(MyByteStreams::BS_2, NVS::ByteStreamView{new_bs2, sizeof(new_bs2)});

      Serial.println("Done. Press 'p' to see the updated values.\n");
    } break;
  }
}
