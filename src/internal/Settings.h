/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <initializer_list>
#include <nvs.h>
#include <string.h>
#include <type_traits>

#include "ISettings.h"
#include "Policy.h"

namespace NVS {

/**
 * @brief Typed settings container for a fixed list of NVS entries under a single namespace.
 *
 * Each instance owns its own NVS namespace handle, opened via `begin()` and closed via `end()`.
 * Multiple instances may share the same namespace (keys must then be unique within it) or use
 * independent namespaces, enabling reusable components with the same key set.
 *
 * @note NVS namespace names are limited to 15 characters.
 *
 * @tparam T Value type (`bool`, `uint32_t`, `int32_t`, `float`, `double`, `const char*`,
 * `ByteStream`).
 * @tparam ENUM Enum class whose enumerators index into the settings list.
 * @tparam N Number of settings (use `SETTINGS_COUNT` macro).
 */
template <typename T, typename ENUM, size_t N>
class Settings : public ISettings {
  public:
  using Policy    = typename Internal::PolicyTrait<T>::policy_type;
  using Struct    = typename Internal::PolicyTrait<T>::struct_type;
  using WriteType = typename Internal::PolicyTrait<T>::write_type;
  using OnChangeCb =
    std::function<void(const char* key, const ENUM setting, const WriteType updated_value)>;

  /**
   * @brief Construct a Settings object. Call `begin()` before any read/write operation.
   * @param ns_name NVS namespace name (max 15 characters).
   * @param list Initializer list of Setting structs, one per enum entry.
   */
  Settings(const char* ns_name, std::initializer_list<Struct> list)
      : _ns_name(ns_name)
      , _handle(0)
      , _is_open(false)
      , _global_on_change_cb(nullptr)
      , _global_on_change_cb_callable_on_format(false) {
    _on_change_cbs.fill(nullptr);
    _on_change_cbs_callable_on_format.fill(false);
    std::copy_n(list.begin(), N, _list.begin());
  }

  ~Settings() { end(); }

  /* ----------------------------------------- Lifecycle ---------------------------------------- */

  /**
   * @brief Open the NVS namespace handle. Must be called after `NVS::init()`.
   * @retval `true` Handle opened successfully.
   * @retval `false` Operation failed.
   */
  bool begin() override {
    if (_is_open) return true;
    _is_open = (nvs_open(_ns_name, NVS_READWRITE, &_handle) == ESP_OK);
    return _is_open;
  }

  /**
   * @brief Close the NVS namespace handle.
   */
  void end() override {
    if (!_is_open) return;
    nvs_close(_handle);
    _handle  = 0;
    _is_open = false;
  }

  /**
   * @brief Get the NVS namespace string used by this Settings object.
   * @return Namespace string.
   */
  virtual const char* getNamespace() const override { return _ns_name; }

  /**
   * @brief Check whether the NVS handle is currently open.
   * @retval `true` Handle is open.
   * @retval `false` Handle is closed.
   */
  bool isOpen() const override { return _is_open; }

  /**
   * @brief Erase all keys in the namespace.
   * @retval `true` All keys erased successfully.
   * @retval `false` Operation failed.
   */
  bool eraseAll() override {
    if (!_is_open) return false;
    if (nvs_erase_all(_handle) != ESP_OK) return false;
    return nvs_commit(_handle) == ESP_OK;
  }

  /* ------------------------------------ ISettings interface ----------------------------------- */

  /**
   * @brief Get the value type of this Settings object.
   * @return `Type` enum value.
   */
  Type getType() const override { return Internal::PolicyTrait<T>::enum_type; }

  /**
   * @brief Get the number of settings in this object.
   * @return `size_t` Count.
   */
  size_t getSize() const override { return N; }

  /**
   * @brief Get a NVS key string by index.
   * @param index Index in the list.
   * @retval `const char*` Key string.
   * @retval `nullptr` Index out of bounds.
   */
  const char* getKey(size_t index) const override {
    if (index >= N) return nullptr;
    return _list[index].key;
  }

  /**
   * @brief Get a hint string by index.
   * @param index Index in the list.
   * @retval `const char*` Hint string.
   * @retval `nullptr` Index out of bounds.
   */
  const char* getHint(size_t index) const override {
    if (index >= N) return nullptr;
    return _list[index].hint;
  }

  /**
   * @brief Get a pointer to the default value. Cast to the correct type before use.
   * @param index Index in the list.
   * @retval `const void*` Pointer to default value.
   * @retval `nullptr` Index out of bounds.
   */
  const void* getDefaultValuePtr(size_t index) const override {
    if (index >= N) return nullptr;
    return &(_list[index].default_value);
  }

  /**
   * @brief Write a new value via untyped pointer.
   * @param index Index in the list.
   * @param value Pointer to the new value.
   * @retval `true` Written successfully.
   * @retval `false` Handle not open, index out of bounds, or NVS write error.
   */
  bool setValuePtr(size_t index, const void* value) override {
    if (index >= N) return false;
    return setValueImpl(static_cast<ENUM>(index), *static_cast<const WriteType*>(value), false);
  }

  /**
   * @brief Read the current NVS value into a caller-provided buffer via untyped pointer.
   * @param index Index in the list.
   * @param value Destination buffer.
   * @param size Size of the destination buffer in bytes.
   * @retval `true` Read successfully.
   * @retval `false` Handle not open, index out of bounds, or buffer too small.
   */
  bool getValuePtr(size_t index, void* value, size_t size) override {
    if (index >= N) return false;
    if (size < sizeof(T)) return false;
    T& out = *static_cast<T*>(value);
    return _policy.getValue(_handle, _list[index].key, out);
  }

  /**
   * @brief Read the current NVS value into a caller-provided buffer via untyped pointer, with
   * fallback to the default value if the key is not found in NVS.
   * @param index Index in the list.
   * @param value Destination buffer.
   * @param size Size of the destination buffer in bytes.
   * @retval `true` Value read from NVS or default value.
   * @retval `false` Handle not open, index out of bounds, or buffer too small.
   */
  bool getValuePtrOrDefault(size_t index, void* value, size_t size) override {
    if (index >= N) return false;
    if (size < sizeof(T)) return false;

    T& out = *static_cast<T*>(value);
    if (!_policy.getValue(_handle, _list[index].key, out)) {
      _applyDefault(out, _list[index].default_value);
    }
    return true;
  }

  /**
   * @brief Register a callback that fires on every value change across this object.
   * @param callback Callback function.
   * @param callable_on_format Whether to invoke the callback when a format operation writes a
   * value.
   */
  void setGlobalOnChangeCallback(GlobalOnChangeCb callback, bool callable_on_format) override {
    _global_on_change_cb                    = callback;
    _global_on_change_cb_callable_on_format = callable_on_format;
  }

  /**
   * @brief Remove the global change callback.
   */
  void clearGlobalOnChangeCallback() override {
    _global_on_change_cb                    = nullptr;
    _global_on_change_cb_callable_on_format = false;
  }

  /**
   * @brief Check whether the given key string exists in this object's list.
   * @param key Key string to search.
   * @param index_found Set to the matching index if found.
   * @retval `true` Found.
   * @retval `false` Not found.
   */
  bool hasKey(const char* key, size_t& index_found) const override {
    for (size_t i = 0; i < N; i++) {
      if (strcmp(_list[i].key, key) == 0) {
        index_found = i;
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Check whether a setting is marked as formattable.
   * @param index Index in the list.
   * @retval `true` Formattable.
   * @retval `false` Not formattable or out of bounds.
   */
  bool isFormattable(size_t index) const override {
    if (index >= N) return false;
    return _list[index].formattable;
  }

  /**
   * @brief Write the default value back to NVS for a single setting.
   * @param index Index in the list.
   * @param force Ignore the formattable flag and write regardless.
   * @retval `true` Written successfully.
   * @retval `false` Not formattable (without force), out of bounds, or NVS error.
   */
  bool format(size_t index, bool force = false) override {
    if (index >= N) return false;
    if (!isFormattable(index) && !force) return false;
    return setValueImpl(static_cast<ENUM>(index), getDefaultValue(index), true);
  }

  /**
   * @brief Write the default value back to NVS for all settings.
   * @param force Ignore the formattable flag for all entries.
   * @return `size_t` Number of entries that failed to write.
   */
  size_t formatAll(bool force = false) override {
    size_t errors = 0;
    for (size_t i = 0; i < N; i++) {
      if (!isFormattable(i) && !force) continue;
      if (!setValueImpl(static_cast<ENUM>(i), getDefaultValue(i), true)) errors++;
    }
    return errors;
  }

  /* ----------------------------------------- Typed API ---------------------------------------- */

  /**
   * @brief Get a NVS key string by enum entry.
   * @param setting Enum entry.
   * @return `const char*` Key string.
   */
  const char* getKey(ENUM setting) const { return _list[static_cast<size_t>(setting)].key; }

  /**
   * @brief Get a hint string by enum entry.
   * @param setting Enum entry.
   * @return `const char*` Hint string.
   */
  const char* getHint(ENUM setting) const { return _list[static_cast<size_t>(setting)].hint; }

  /**
   * @brief Get the default value by index.
   * @param index Index in the list.
   * @return `WriteType` Default value.
   */
  WriteType getDefaultValue(size_t index) const {
    if (index >= N) return WriteType();
    return _list[index].default_value;
  }

  /**
   * @brief Get the default value by enum entry.
   * @param setting Enum entry.
   * @return `WriteType` Default value.
   */
  WriteType getDefaultValue(ENUM setting) const {
    return _list[static_cast<size_t>(setting)].default_value;
  }

  /**
   * @brief Write a new value to NVS.
   * @param setting Enum entry.
   * @param value Value to write.
   * @retval `true` Written successfully.
   * @retval `false` Handle not open or NVS write error.
   */
  bool setValue(ENUM setting, const WriteType value) { return setValueImpl(setting, value, false); }

  /**
   * @brief Read the current value from NVS into `out`.
   *
   * For NVS::Str: set `out.data` to a caller-owned buffer and `out.size` to its capacity
   * before calling.
   *
   * For NVS::ByteStream: set `out.data` to a caller-owned buffer and `out.size` to its capacity
   * before calling.
   *
   * @retval `true` Value was read from NVS.
   * @retval `false` Value not found in NVS, handle not open, or buffer too small.
   */
  bool getValue(ENUM setting, T& out) {
    return _policy.getValue(_handle, getKey(static_cast<size_t>(setting)), out);
  }

  /**
   * @brief Read the current value from NVS into `out`, with fallback to the default value if the
   * key is not found in NVS.
   *
   * For NVS::Str: set `out.data` to a caller-owned buffer and `out.size` to its capacity
   * before calling.
   *
   * For NVS::ByteStream: set `out.data` to a caller-owned buffer and `out.size` to its capacity
   * before calling.
   *
   * @return The value read from NVS, or the default value if not found in NVS or on error.
   */
  T getValueOrDefault(ENUM setting, T& out) {
    if (!_policy.getValue(_handle, getKey(static_cast<size_t>(setting)), out)) {
      _applyDefault(out, _list[static_cast<size_t>(setting)].default_value);
    }
    return out;
  }

  /**
   * @brief Format a single setting to its default value.
   * @param force Ignore the formattable flag and write regardless.
   * @retval `true` Written successfully.
   * @retval `false` Not formattable (without force), out of bounds, or NVS error.
   */
  bool format(ENUM setting, bool force = false) {
    return format(static_cast<size_t>(setting), force);
  }

  /**
   * @brief Check whether a setting is marked as formattable.
   * @retval `true` Setting is formattable.
   * @retval `false` Setting is not formattable.
   */
  bool isFormattable(ENUM setting) const { return _list[static_cast<size_t>(setting)].formattable; }

  /**
   * @brief Register a callback for a specific setting.
   * @param callable_on_format Whether to invoke when a format operation writes this setting.
   */
  void setOnChangeCallback(ENUM setting, OnChangeCb callback, bool callable_on_format) {
    size_t index                             = static_cast<size_t>(setting);
    _on_change_cbs[index]                    = callback;
    _on_change_cbs_callable_on_format[index] = callable_on_format;
  }

  /**
   * @brief Remove the callback for a specific setting.
   */
  void clearOnChangeCallback(ENUM setting) {
    size_t index                             = static_cast<size_t>(setting);
    _on_change_cbs[index]                    = nullptr;
    _on_change_cbs_callable_on_format[index] = false;
  }

  private:
  const char* _ns_name;
  nvs_handle_t _handle;
  bool _is_open;

  GlobalOnChangeCb _global_on_change_cb;
  bool _global_on_change_cb_callable_on_format;

  std::array<OnChangeCb, N> _on_change_cbs;
  std::array<bool, N> _on_change_cbs_callable_on_format;

  std::array<Struct, N> _list;
  Policy _policy;

  /* -------------------------------------- Private helpers ------------------------------------- */

  static void _applyDefault(T& out, const WriteType& default_val) {
    if constexpr (std::is_same_v<T, Str>) {
      if (out.data && out.max_size > 0 && default_val.data) {
        strncpy(out.data, default_val.data, out.max_size - 1);
        out.data[out.max_size - 1] = '\0';
      }
    } else if constexpr (std::is_same_v<T, ByteStream>) {
      if (out.data && default_val.data && out.max_size >= default_val.size) {
        memcpy(out.data, default_val.data, default_val.size);
        out.size = default_val.size;
      }
    } else {
      out = default_val;
    }
  }

  bool setValueImpl(ENUM setting, const WriteType value, bool called_from_format) {
    if (!_is_open) return false;

    size_t index = static_cast<size_t>(setting);

    if (!_policy.setValue(_handle, getKey(index), value)) return false;

    bool call_global = called_from_format ? _global_on_change_cb_callable_on_format : true;
    bool call_local  = called_from_format ? _on_change_cbs_callable_on_format[index] : true;

    if (call_global && _global_on_change_cb) {
      _global_on_change_cb(getKey(setting), getType(), index, &value);
    }

    if (call_local && _on_change_cbs[index]) {
      _on_change_cbs[index](getKey(setting), setting, value);
    }

    return true;
  }
};

} // namespace NVS
