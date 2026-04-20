/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace NVS {

/// @brief Type of a Settings object. Useful when using ISettings pointers.
enum class Type : uint8_t { Bool, UInt32, Int32, Float, Double, String, ByteStream };

/// @brief Read-only view of a string. Used for default values and write operations.
struct StrView {
  // Pointer to a null-terminated string. Must be valid for the lifetime of the Settings object.
  const char* data;

  // Size of the string in bytes, including the null terminator. Set by the library when reading, or
  // by the user when creating a StrView for default values.
  size_t size;

  StrView(const char* d = nullptr)
      : data(d)
      , size(d ? (strlen(d) + 1) : 0) {}
};

/// @brief Mutable string buffer for read operations. Caller must allocate the buffer.
struct Str {
  char* data;      // Pointer to char buffer. Must be of max_size bytes allocated by the user.
  size_t max_size; // Max capacity of the buffer in bytes, including space for null terminator.

  Str(char* d = nullptr, const size_t s = 0)
      : data(d)
      , max_size(s) {}

  // Implicit conversion to StrView for convenience in default values and setValue calls.
  operator StrView() const { return StrView{data}; }
};

struct ByteStreamView;

/// @brief Mutable byte buffer for read operations. Caller must allocate the buffer.
struct ByteStream {
  // Optional data format metadata. Not persisted in NVS.
  enum class Format : uint8_t { Hex, Base64, JSONObject, JSONArray } format;

  uint8_t* data;   // Pointer to byte buffer. Must be of max_size bytes allocated by the user.
  size_t size;     // Size of the valid data in bytes. Set by the library when reading.
  size_t max_size; // Max capacity of the buffer in bytes. Set by the user.

  ByteStream(uint8_t* d = nullptr, size_t max_s = 0, Format f = Format::Hex)
      : format(f)
      , data(d)
      , size(0)
      , max_size(max_s) {}

  // Implicit conversion to ByteStreamView for convenience in default values and setValue calls.
  operator ByteStreamView() const;
};

/// @brief Read-only view of a byte blob. Used for default values and write operations.
struct ByteStreamView {
  // Pointer to byte data. Must be valid for the lifetime of the Settings object.
  const uint8_t* data;

  // Size of the byte blob in bytes. Set by the user for default values, or by the library when
  // reading.
  size_t size;

  // Optional data format metadata. Not persisted in NVS.
  ByteStream::Format format;

  ByteStreamView(const uint8_t* d = nullptr, size_t s = 0,
                 ByteStream::Format f = ByteStream::Format::Hex)
      : data(d)
      , size(s)
      , format(f) {}
};

inline ByteStream::operator ByteStreamView() const { return ByteStreamView{data, size, format}; }

} // namespace NVS
