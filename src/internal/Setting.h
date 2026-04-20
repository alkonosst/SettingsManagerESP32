/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "Types.h"

namespace NVS {

namespace Internal {

/**
 * @brief Metadata and default value for a single NVS setting entry.
 * @note NVS key length is limited to 15 characters.
 * @tparam T Type of the setting value.
 */
template <typename T>
struct Setting {
  const char* key;
  const char* hint;
  T default_value;
  bool formattable;
};

/// @brief Specialization for Str: stores the default as a read-only StrView.
template <>
struct Setting<Str> {
  const char* key;
  const char* hint;
  StrView default_value;
  bool formattable;
};

/// @brief Specialization for ByteStream: stores the default as a read-only ByteStreamView.
template <>
struct Setting<ByteStream> {
  const char* key;
  const char* hint;
  ByteStreamView default_value;
  bool formattable;
};

} // namespace Internal

} // namespace NVS
