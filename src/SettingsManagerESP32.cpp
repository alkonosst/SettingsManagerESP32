/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "SettingsManagerESP32.h"

#include <nvs_flash.h>
#include <string.h>

namespace NVS {

static bool _initialized = false;

bool init(const char* partition_name) {
  if (_initialized) return true;

  bool success = false;

  if (partition_name) {
    success = (nvs_flash_init_partition(partition_name) == ESP_OK);
  } else {
    success = (nvs_flash_init() == ESP_OK);
  }

  if (!success) return false;

  _initialized = true;
  return true;
}

bool deinit(const char* partition_name) {
  if (!_initialized) return true;

  bool success = false;

  if (partition_name) {
    success = (nvs_flash_deinit_partition(partition_name) == ESP_OK);
  } else {
    success = (nvs_flash_deinit() == ESP_OK);
  }

  if (!success) return false;

  _initialized = false;
  return true;
}

bool erase(const char* partition_name) {
  if (!_initialized) return false;

  bool success;

  if (partition_name) {
    success = (nvs_flash_erase_partition(partition_name) == ESP_OK);
  } else {
    success = (nvs_flash_erase() == ESP_OK);
  }

  // nvs_flash_erase() implicitly deinitializes the partition
  if (success) _initialized = false;
  return success;
}

const char* typeToStr(const NVS::Type t) {
  switch (t) {
    case NVS::Type::Bool: return "Bool";
    case NVS::Type::UInt32: return "UInt32";
    case NVS::Type::Int32: return "Int32";
    case NVS::Type::Float: return "Float";
    case NVS::Type::Double: return "Double";
    case NVS::Type::String: return "String";
    case NVS::Type::ByteStream: return "ByteStream";
    default: return "Unknown";
  }
}

bool fromHexToStr(const NVS::ByteStreamView bs, char* buf, const size_t buf_size,
                  const bool with_spaces) {
  if (!bs.data || buf_size < hexStrSize(bs.size, with_spaces)) return false;

  static const char HEX_CHARS[] = "0123456789ABCDEF";

  for (size_t i = 0; i < bs.size; i++) {
    size_t pos   = with_spaces ? i * 3 : i * 2;
    buf[pos]     = HEX_CHARS[bs.data[i] >> 4];
    buf[pos + 1] = HEX_CHARS[bs.data[i] & 0x0F];
    if (with_spaces && i < bs.size - 1) buf[pos + 2] = ' ';
  }

  buf[with_spaces ? bs.size * 3 - 1 : bs.size * 2] = '\0';
  return true;
}

bool fromStrToHex(const char* hex, NVS::ByteStream& out, const bool with_spaces) {
  if (!hex || !out.data) return false;

  size_t len = strlen(hex);
  size_t byte_count;

  if (with_spaces) {
    if (len == 0) {
      out.size = 0;
      return true;
    }
    if ((len + 1) % 3 != 0) return false;
    byte_count = (len + 1) / 3;
  } else {
    if (len % 2 != 0) return false;
    byte_count = len / 2;
  }

  if (byte_count > out.max_size) return false;

  for (size_t i = 0; i < byte_count; i++) {
    size_t pos = with_spaces ? i * 3 : i * 2;
    uint8_t high, low;
    char c = hex[pos];

    if (c >= '0' && c <= '9')
      high = c - '0';
    else if (c >= 'A' && c <= 'F')
      high = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
      high = c - 'a' + 10;
    else
      return false;

    c = hex[pos + 1];

    if (c >= '0' && c <= '9')
      low = c - '0';
    else if (c >= 'A' && c <= 'F')
      low = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
      low = c - 'a' + 10;
    else
      return false;

    if (with_spaces && i < byte_count - 1 && hex[pos + 2] != ' ') return false;

    out.data[i] = (high << 4) | low;
  }

  out.size = byte_count;
  return true;
}

const char* formatToStr(const NVS::ByteStream::Format f) {
  switch (f) {
    case NVS::ByteStream::Format::Hex: return "Hex";
    case NVS::ByteStream::Format::Base64: return "Base64";
    case NVS::ByteStream::Format::JSONObject: return "JSONObject";
    case NVS::ByteStream::Format::JSONArray: return "JSONArray";
    default: return "Unknown";
  }
}

} // namespace NVS
