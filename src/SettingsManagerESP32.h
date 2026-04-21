/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "internal/ISettings.h"
#include "internal/Policy.h"
#include "internal/Setting.h"
#include "internal/Settings.h"
#include "internal/Types.h"

/* ---------------------------------- X-macro expansion helpers --------------------------------- */

// Expands one X-macro entry into an enum class enumerator
#define SETTINGS_EXPAND_ENUM_CLASS(name, text, value, formattable) name,

// Expands one X-macro entry into a NVS::Internal::Setting initializer
#define SETTINGS_EXPAND_SETTINGS(name, text, value, formattable) {#name, text, value, formattable},

// Counts one X-macro entry (used internally by SETTINGS_COUNT)
#define SETTINGS_ADD_ELEMENT(...) +1

/**
 * @brief Counts the number of entries in an X-macro list.
 *
 * Usage: `SETTINGS_COUNT(MY_SETTINGS_MACRO)`
 */
#define SETTINGS_COUNT(settings_macro) (0 settings_macro(SETTINGS_ADD_ELEMENT))

/* --------------------------------- Convenience creation macros -------------------------------- */

/**
 * Each macro declares an enum class and a matching NVS::Settings<> object in one step.
 * The object is named st_<name> and must be opened with st_<name>.begin() after NVS::init().
 *
 * Parameters:
 * - name           : identifier used for the enum class and the st_<name> object
 * - ns             : NVS namespace string (max 15 chars)
 * - settings_macro : X-macro list macro
 */

#define SETTINGS_CREATE_BOOLS(name, ns, settings_macro)                     \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  NVS::Settings<bool, name, SETTINGS_COUNT(settings_macro)> st_##name(      \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

#define SETTINGS_CREATE_UINT32S(name, ns, settings_macro)                   \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  NVS::Settings<uint32_t, name, SETTINGS_COUNT(settings_macro)> st_##name(  \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

#define SETTINGS_CREATE_INT32S(name, ns, settings_macro)                    \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  NVS::Settings<int32_t, name, SETTINGS_COUNT(settings_macro)> st_##name(   \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

#define SETTINGS_CREATE_FLOATS(name, ns, settings_macro)                    \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  NVS::Settings<float, name, SETTINGS_COUNT(settings_macro)> st_##name(     \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

#define SETTINGS_CREATE_DOUBLES(name, ns, settings_macro)                   \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  NVS::Settings<double, name, SETTINGS_COUNT(settings_macro)> st_##name(    \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

#define SETTINGS_CREATE_STRINGS(name, ns, settings_macro)                   \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  NVS::Settings<NVS::Str, name, SETTINGS_COUNT(settings_macro)> st_##name(  \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

#define SETTINGS_CREATE_BYTE_STREAMS(name, ns, settings_macro)                    \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) };       \
  NVS::Settings<NVS::ByteStream, name, SETTINGS_COUNT(settings_macro)> st_##name( \
    ns, {settings_macro(SETTINGS_EXPAND_SETTINGS)});

/* ----------------------------------- NVS partition lifecycle ---------------------------------- */

namespace NVS {

/**
 * @brief Initialize the default NVS flash partition. Call once in `setup()` before any `begin()`.
 * @param partition_name Optional custom partition name. If `nullptr`, the default partition is
 * used.
 * @return Initialized successfully or already initialized, false otherwise.
 */
bool init(const char* partition_name = nullptr);

/**
 * @brief Deinitialize the default NVS flash partition.
 * @param partition_name Optional custom partition name. If `nullptr`, the default partition is
 * used.
 * @return Deinitialized successfully or already deinitialized, false otherwise.
 */
bool deinit(const char* partition_name = nullptr);

/**
 * @brief Erase the entire default NVS flash partition. All namespaces and keys are lost. After
 * calling this, you must call `init()` again before using any Settings objects.
 * @param partition_name Optional custom partition name. If `nullptr`, the default partition is
 * used.
 * @return Erased successfully, false otherwise.
 */
bool erase(const char* partition_name = nullptr);

/* ------------------------------------------ Utilities ----------------------------------------- */

/**
 * @brief Get NVS storage statistics for the default or a custom partition.
 * @param stats Output parameter for the stats struct.
 * @param partition_name Optional custom partition name. If `nullptr`, the default partition is
 * used.
 * @retval `true` Stats retrieved successfully.
 * @retval `false` Operation failed (e.g. not initialized).
 */
bool getStats(nvs_stats_t& stats, const char* partition_name = nullptr);

/**
 * @brief Convert a `NVS::Type` enum value to a string representation.
 * @param t `NVS::Type` value.
 * @return A string representation of the type.
 */
const char* typeToStr(const NVS::Type t);

/**
 * @brief Calculate the buffer size required by `hexStrSize()` for a given number of bytes.
 * @param byte_count Number of bytes to encode.
 * @param with_spaces Whether spaces will be inserted between bytes (e.g. `"DE AD BE EF"`).
 * @return Required buffer size in bytes, including the null terminator.
 */
constexpr size_t hexStrSize(const size_t byte_count, const bool with_spaces = false) {
  if (byte_count == 0) return 1;
  return with_spaces ? byte_count * 3 : byte_count * 2 + 1;
}

/**
 * @brief Encode a `ByteStreamView` as a null-terminated hex string into a caller-provided buffer.
 * @param bs The byte data to encode.
 * @param buf Output buffer. Must hold at least `hexStrSize(bs.size, with_spaces)` bytes.
 * @param buf_size Size of the output buffer in bytes.
 * @param with_spaces If `true`, bytes are separated by spaces (e.g. `"DE AD BE EF"`). If `false`,
 * bytes are concatenated (e.g. `"DEADBEEF"`).
 * @retval `true` Encoded successfully.
 * @retval `false` Buffer too small or `bs.data` is null.
 */
bool fromHexToStr(const NVS::ByteStreamView bs, char* buf, const size_t buf_size,
                  const bool with_spaces = false);

/**
 * @brief Decode a hex string into a `ByteStream` buffer.
 * @param hex Null-terminated hex string. With `with_spaces = false`, must have even length (e.g.
 * `"DEADBEEF"`). With `with_spaces = true`, bytes must be separated by single spaces
 * (e.g. `"DE AD BE EF"`).
 * @param out Destination `ByteStream`. `out.data` must point to a buffer large enough to hold the
 * decoded bytes. On success, `out.size` is updated.
 * @param with_spaces Whether the input string uses space-separated bytes.
 * @retval `true` Decoded successfully.
 * @retval `false` Invalid hex characters, malformed string, or buffer too small.
 */
bool fromStrToHex(const char* hex, NVS::ByteStream& out, const bool with_spaces = false);

/**
 * @brief Convert a `ByteStream::Format` enum value to a string representation.
 * @param f The `ByteStream::Format` value.
 * @return A string representation of the format.
 */
const char* formatToStr(const NVS::ByteStream::Format f);

} // namespace NVS
