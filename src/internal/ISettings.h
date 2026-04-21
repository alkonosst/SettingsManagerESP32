/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <functional>
#include <stddef.h>

#include "Types.h"

namespace NVS {

/**
 * @brief Type-erased interface for Settings objects. Useful for storing heterogeneous settings
 * in arrays or passing by pointer without knowing the value type at the call site.
 */
class ISettings {
  public:
  using GlobalOnChangeCb = std::function<void(const char* key, const Type type, const size_t index,
                                              const void* updated_value)>;

  /**
   * @brief Open the NVS namespace handle. Must be called after `NVS::init()`.
   * @retval `true` Handle opened successfully.
   * @retval `false` Operation failed.
   */
  virtual bool begin() = 0;

  /**
   * @brief Close the NVS namespace handle.
   */
  virtual void end() = 0;

  /**
   * @brief Get the NVS namespace string used by this Settings object.
   * @return Namespace string.
   */
  virtual const char* getNamespace() const = 0;

  /**
   * @brief Check whether the NVS handle is currently open.
   * @retval `true` Handle is open.
   * @retval `false` Handle is closed.
   */
  virtual bool isOpen() const = 0;

  /**
   * @brief Erase all keys in the namespace.
   * @retval `true` All keys erased successfully.
   * @retval `false` Operation failed.
   */
  virtual bool eraseAll() = 0;

  /**
   * @brief Get the value type of this Settings object.
   * @return `Type` enum value.
   */
  virtual Type getType() const = 0;

  /**
   * @brief Get the number of settings in this object.
   * @return `size_t` Count.
   */
  virtual size_t getSize() const = 0;

  /**
   * @brief Get a NVS key string by index.
   * @param index Index in the list.
   * @retval `const char*` Key string.
   * @retval `nullptr` Index out of bounds.
   */
  virtual const char* getKey(size_t index) const = 0;

  /**
   * @brief Get a hint string by index.
   * @param index Index in the list.
   * @retval `const char*` Hint string.
   * @retval `nullptr` Index out of bounds.
   */
  virtual const char* getHint(size_t index) const = 0;

  /**
   * @brief Get a pointer to the default value. Cast to the correct type before use.
   * @param index Index in the list.
   * @retval `const void*` Pointer to default value.
   * @retval `nullptr` Index out of bounds.
   */
  virtual const void* getDefaultValuePtr(size_t index) const = 0;

  /**
   * @brief Get the default value cast to type `T`.
   * @tparam T Type to cast to.
   * @param index Index in the list.
   * @return `T` Default value, or a default-constructed `T` if out of bounds.
   */
  template <typename T>
  T getDefaultValueAs(size_t index) const {
    if (index >= getSize()) return T();
    return *(static_cast<const T*>(getDefaultValuePtr(index)));
  }

  /**
   * @brief Write a new value via untyped pointer.
   * @param index Index in the list.
   * @param value Pointer to the new value.
   * @retval `true` Written successfully.
   * @retval `false` Handle not open, index out of bounds, or NVS write error.
   */
  virtual bool setValuePtr(size_t index, const void* value) = 0;

  /**
   * @brief Read the current NVS value into a caller-provided buffer via untyped pointer.
   * @param index Index in the list.
   * @param value Destination buffer.
   * @param size Size of the destination buffer in bytes.
   * @retval `true` Read successfully.
   * @retval `false` Handle not open, index out of bounds, or buffer too small.
   */
  virtual bool getValuePtr(size_t index, void* value, size_t size) = 0;

  /**
   * @brief Read the current NVS value into a caller-provided buffer via untyped pointer, with
   * fallback to the default value if the key is not found in NVS.
   * @param index Index in the list.
   * @param value Destination buffer.
   * @param size Size of the destination buffer in bytes.
   * @return `const void*` Pointer to the value read from NVS, or pointer to the default value if
   * not found in NVS or on error.
   */
  virtual const void* getValuePtrOrDefault(size_t index, void* value, size_t size) = 0;

  /**
   * @brief Register a callback that fires on every value change across this object.
   * @param callback Callback function.
   * @param callable_on_format Whether to invoke the callback when a format operation writes a
   * value.
   */
  virtual void setGlobalOnChangeCallback(GlobalOnChangeCb callback, bool callable_on_format) = 0;

  /**
   * @brief Remove the global change callback.
   */
  virtual void clearGlobalOnChangeCallback() = 0;

  /**
   * @brief Check whether the given key string exists in this object's list.
   * @param key Key string to search.
   * @param index_found Set to the matching index if found.
   * @retval `true` Found.
   * @retval `false` Not found.
   */
  virtual bool hasKey(const char* key, size_t& index_found) const = 0;

  /**
   * @brief Check whether a setting is marked as formattable.
   * @param index Index in the list.
   * @retval `true` Formattable.
   * @retval `false` Not formattable or out of bounds.
   */
  virtual bool isFormattable(size_t index) const = 0;

  /**
   * @brief Write the default value back to NVS for a single setting.
   * @param index Index in the list.
   * @param force Ignore the formattable flag and write regardless.
   * @retval `true` Written successfully.
   * @retval `false` Not formattable (without force), out of bounds, or NVS error.
   */
  virtual bool format(size_t index, bool force = false) = 0;

  /**
   * @brief Write the default value back to NVS for all settings.
   * @param force Ignore the formattable flag for all entries.
   * @return `size_t` Number of entries that failed to write.
   */
  virtual size_t formatAll(bool force = false) = 0;
};

} // namespace NVS
