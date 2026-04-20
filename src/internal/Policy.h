/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <nvs.h>
#include <string.h>

#include "Setting.h"
#include "Types.h"

namespace NVS {

namespace Internal {

/// @brief Policy for bool values. Stored as uint8_t (0 or 1).
class BoolPolicy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, bool value) {
    if (nvs_set_u8(handle, key, value ? 1u : 0u) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, bool& value) {
    uint8_t val;
    if (nvs_get_u8(handle, key, &val) != ESP_OK) return false;
    value = (val != 0);
    return true;
  }
};

/// @brief Policy for uint32_t values.
class UInt32Policy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, uint32_t value) {
    if (nvs_set_u32(handle, key, value) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, uint32_t& value) {
    if (nvs_get_u32(handle, key, &value) != ESP_OK) return false;
    return true;
  }
};

/// @brief Policy for int32_t values.
class Int32Policy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, int32_t value) {
    if (nvs_set_i32(handle, key, value) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, int32_t& value) {
    if (nvs_get_i32(handle, key, &value) != ESP_OK) return false;
    return true;
  }
};

/// @brief Policy for float values. Stored as uint32_t via memcpy to preserve bit representation.
class FloatPolicy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    if (nvs_set_u32(handle, key, bits) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, float& value) {
    uint32_t bits;
    if (nvs_get_u32(handle, key, &bits) != ESP_OK) return false;
    memcpy(&value, &bits, sizeof(value));
    return true;
  }
};

/**
 * @brief Policy for double values. Stored as uint64_t via memcpy to preserve bit representation.
 */
class DoublePolicy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, double value) {
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    if (nvs_set_u64(handle, key, bits) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, double& value) {
    uint64_t bits;
    if (nvs_get_u64(handle, key, &bits) != ESP_OK) return false;
    memcpy(&value, &bits, sizeof(value));
    return true;
  }
};

/// @brief Policy for string values. Reads directly into a caller-provided buffer.
class StringPolicy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, StrView value) {
    if (nvs_set_str(handle, key, value.data) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, Str& value) {
    if (!value.data || value.max_size == 0) return false;

    // Try to read and fit the value in the provided buffer
    size_t required_size = 0;
    if (nvs_get_str(handle, key, nullptr, &required_size) != ESP_OK) return false;
    if (required_size > value.max_size) return false;
    if (nvs_get_str(handle, key, value.data, &required_size) != ESP_OK) return false;

    return true;
  }
};

/// @brief Policy for binary blob values. Reads directly into a caller-provided buffer.
class ByteStreamPolicy {
  public:
  bool setValue(nvs_handle_t handle, const char* key, ByteStreamView value) {
    if (nvs_set_blob(handle, key, value.data, value.size) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
  }

  bool getValue(nvs_handle_t handle, const char* key, ByteStream& value) {
    if (!value.data || value.max_size == 0) return false;

    // Try to read and fit the value in the provided buffer
    size_t required_size = 0;
    if (nvs_get_blob(handle, key, nullptr, &required_size) != ESP_OK) return false;
    if (required_size > value.max_size) return false;
    if (nvs_get_blob(handle, key, value.data, &required_size) != ESP_OK) return false;

    value.size = required_size;
    return true;
  }
};

/**
 * @brief Maps C++ types to their corresponding policy, NVS::Type enum, and Setting struct.
 * @tparam T Value type.
 */
template <typename T>
struct PolicyTrait {};

template <>
struct PolicyTrait<bool> {
  static const Type enum_type = Type::Bool;
  using policy_type           = BoolPolicy;
  using struct_type           = Setting<bool>;
  using write_type            = bool;
};

template <>
struct PolicyTrait<uint32_t> {
  static const Type enum_type = Type::UInt32;
  using policy_type           = UInt32Policy;
  using struct_type           = Setting<uint32_t>;
  using write_type            = uint32_t;
};

template <>
struct PolicyTrait<int32_t> {
  static const Type enum_type = Type::Int32;
  using policy_type           = Int32Policy;
  using struct_type           = Setting<int32_t>;
  using write_type            = int32_t;
};

template <>
struct PolicyTrait<float> {
  static const Type enum_type = Type::Float;
  using policy_type           = FloatPolicy;
  using struct_type           = Setting<float>;
  using write_type            = float;
};

template <>
struct PolicyTrait<double> {
  static const Type enum_type = Type::Double;
  using policy_type           = DoublePolicy;
  using struct_type           = Setting<double>;
  using write_type            = double;
};

template <>
struct PolicyTrait<Str> {
  static const Type enum_type = Type::String;
  using policy_type           = StringPolicy;
  using struct_type           = Setting<Str>;
  using write_type            = StrView;
};

template <>
struct PolicyTrait<ByteStream> {
  static const Type enum_type = Type::ByteStream;
  using policy_type           = ByteStreamPolicy;
  using struct_type           = Setting<ByteStream>;
  using write_type            = ByteStreamView;
};

} // namespace Internal

} // namespace NVS
