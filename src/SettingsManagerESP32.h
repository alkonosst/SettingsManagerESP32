/**
 * SPDX-FileCopyrightText: 2024 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <initializer_list>
#include <vector>

#ifndef SETTINGS_STRING_BUFFER_SIZE
#define SETTINGS_STRING_BUFFER_SIZE 32
#endif

#ifndef SETTINGS_BYTE_STREAM_BUFFER_SIZE
#define SETTINGS_BYTE_STREAM_BUFFER_SIZE 32
#endif

// X-macros
#define SETTINGS_EXPAND_ENUM_CLASS(name, text, value, formatteable) name,
#define SETTINGS_EXPAND_SETTINGS(name, text, value, formatteable) \
  {#name, text, value, formatteable},

// NVS (non-volatile storage ESP32)
extern Preferences nvs;

namespace NVS {

// Type of Setting object. Useful when using pointers.
enum class Type : uint8_t { Bool, UInt32, Int32, Float, Double, String, StringClass, ByteStream };

// Byte Stream struct, useful for binary data (blob)
struct ByteStream {
  const uint8_t* data;
  size_t size;
};

namespace Internal {
// Different type of settings to save in NVS
template <typename T>
struct Setting {
  const char* key;
  const char* hint;
  const T default_value;
  const bool formatteable;
};

// Policy types
template <typename T>
class Policy {
  public:
  virtual bool setValue(const char* key, T value)      = 0;
  virtual T getValue(const char* key, T default_value) = 0;
};

// Policy for bool
class BoolPolicy : public Policy<bool> {
  public:
  bool setValue(const char* key, bool value) { return (nvs.putBool(key, value) == sizeof(bool)); }

  bool getValue(const char* key, bool default_value) { return nvs.getBool(key, default_value); }
};

// Policy for uint32_t
class UInt32Policy : public Policy<uint32_t> {
  public:
  bool setValue(const char* key, uint32_t value) {
    return (nvs.putUInt(key, value) == sizeof(uint32_t));
  }

  uint32_t getValue(const char* key, uint32_t default_value) {
    return nvs.getUInt(key, default_value);
  }
};

// Policy for int32_t
class Int32Policy : public Policy<int32_t> {
  public:
  bool setValue(const char* key, int32_t value) {
    return (nvs.putInt(key, value) == sizeof(int32_t));
  }

  int32_t getValue(const char* key, int32_t default_value) {
    return nvs.getInt(key, default_value);
  }
};

// Policy for float
class FloatPolicy : public Policy<float> {
  public:
  bool setValue(const char* key, float value) {
    return (nvs.putFloat(key, value)) == sizeof(float);
  }

  float getValue(const char* key, float default_value) { return nvs.getFloat(key, default_value); }
};

// Policy for double
class DoublePolicy : public Policy<double> {
  public:
  bool setValue(const char* key, double value) {
    return (nvs.putDouble(key, value) == sizeof(double));
  }
  double getValue(const char* key, double default_value) {
    return nvs.getDouble(key, default_value);
  }
};

// Policy for string
class StringPolicy : public Policy<const char*> {
  public:
  bool setValue(const char* key, const char* value) {
    return (nvs.putString(key, value) == strlen(value));
  }

  const char* getValue(const char* key, const char* default_value) {
    size_t len = nvs.getString(key, _buffer, sizeof(_buffer));
    return (len > 0 ? _buffer : nullptr);
  }

  private:
  char _buffer[SETTINGS_STRING_BUFFER_SIZE];
};

// Policy for Byte Streams
class ByteStreamPolicy : public Policy<ByteStream> {
  public:
  bool setValue(const char* key, const ByteStream value) {
    return (nvs.putBytes(key, value.data, value.size) == value.size);
  }

  ByteStream getValue(const char* key, ByteStream default_value) {
    size_t len = nvs.getBytes(key, _buffer, sizeof(_buffer));
    return {len > 0 ? _buffer : nullptr, len};
  }

  private:
  uint8_t _buffer[SETTINGS_BYTE_STREAM_BUFFER_SIZE];
};

// Type trait to map types to policies
template <typename T>
struct PolicyTrait {};

template <>
struct PolicyTrait<bool> {
  static const Type enum_type = Type::Bool;
  using policy_type           = BoolPolicy;
  using struct_type           = Setting<bool>;
};

template <>
struct PolicyTrait<uint32_t> {
  static const Type enum_type = Type::UInt32;
  using policy_type           = UInt32Policy;
  using struct_type           = Setting<uint32_t>;
};

template <>
struct PolicyTrait<int32_t> {
  static const Type enum_type = Type::Int32;
  using policy_type           = Int32Policy;
  using struct_type           = Setting<int32_t>;
};

template <>
struct PolicyTrait<float> {
  static const Type enum_type = Type::Float;
  using policy_type           = FloatPolicy;
  using struct_type           = Setting<float>;
};

template <>
struct PolicyTrait<double> {
  static const Type enum_type = Type::Double;
  using policy_type           = DoublePolicy;
  using struct_type           = Setting<double>;
};

template <>
struct PolicyTrait<const char*> {
  static const Type enum_type = Type::String;
  using policy_type           = StringPolicy;
  using struct_type           = Setting<const char*>;
};

template <>
struct PolicyTrait<ByteStream> {
  static const Type enum_type = Type::ByteStream;
  using policy_type           = ByteStreamPolicy;
  using struct_type           = Setting<ByteStream>;
};
} // namespace Internal

// Interface for Settings objects, useful when using pointers.
class ISettings {
  public:
  using GlobalOnChangeCb = typename std::function<void(
    const char* key, const Type type, const size_t index, const void* updated_value)>;

  /**
   * @brief Get the Type of the object.
   * @return const Type Enum
   */
  virtual const Type getType() = 0;

  /**
   * @brief Get the size of the setting object.
   * @return const size_t Number of settings
   */
  virtual const size_t getSize() = 0;

  /**
   * @brief Get a key string.
   * @param index Index in the list
   * @return const char* Key string, nullptr if out of bounds
   */
  virtual const char* getKey(size_t index) = 0;

  /**
   * @brief Get a hint string.
   * @param index Index in the list
   * @return const char* Hint string, nullptr if out of bounds
   */
  virtual const char* getHint(size_t index) = 0;

  /**
   * @brief Get a pointer to the default value. You need to cast it back to the correct type.
   * @param index Index in the list
   * @return const void* Pointer to the default value, nullptr if out of bounds
   */
  virtual const void* getDefaultValuePtr(size_t index) = 0;

  /**
   * @brief Get a default value casted to type passed as template parameter.
   * @tparam T Type to cast the value
   * @param index Index in the list
   * @return T Setting default value, or a default value of the type if out of bounds
   */
  template <typename T>
  T getDefaultValueAs(size_t index) {
    if (index >= getSize()) return T();
    return *(static_cast<const T*>(getDefaultValuePtr(index)));
  }

  /**
   * @brief Set a new value via pointer.
   * @param index Index in the list
   * @param value New value to store
   * @return true OK
   * @return false Failed to save or index out of bounds
   */
  virtual bool setValuePtr(size_t index, const void* value) = 0;

  /**
   * @brief Get a copy of the value stored in NVS via pointer.
   * @param index Index in the list
   * @param value Pointer to a variable to store a copy of a NVS stored value
   * @param size Size of the variable
   * @return true OK
   * @return false Failed to retrieve or index out of bounds
   */
  virtual bool getValuePtr(size_t index, void* value, size_t size) = 0;

  /**
   * @brief Set a global callback for all the settings of the object. This callback will be called
   * when a setting is changed.
   * @param callback Callback function
   * @param callable_on_format If the callback should be called when the setting is formatted
   */
  virtual void setGlobalOnChangeCallback(GlobalOnChangeCb callback, bool callable_on_format) = 0;

  /**
   * @brief Clear the global callback for all the settings of the object.
   */
  virtual void clearGlobalOnChangeCallback() = 0;

  /**
   * @brief Check if a key exists in the list.
   * @param key Key string
   * @param index_found Index of the key found
   * @return true Key found
   * @return false Key not found
   */
  virtual bool hasKey(const char* key, size_t& index_found) = 0;

  /**
   * @brief Check if a setting is formatteable.
   * @param index Index in the list
   * @return true Formatteable
   * @return false Not formatteable or index out of bounds
   */
  virtual bool isFormatteable(size_t index) = 0;

  /**
   * @brief Format a setting to its default value.
   * @param index Index in the list
   * @param force Force the format even if the setting is not formatteable
   * @return true OK
   * @return false Failed to format or index out of bounds
   */
  virtual bool format(size_t index, bool force = false) = 0;

  /**
   * @brief Format all settings to their default values.
   * @param force Force the format even if the setting is not formatteable
   * @return size_t Number of errors while trying to format
   */
  virtual size_t formatAll(bool force = false) = 0;
};

// Template specialization for const char* (strings)
template <>
inline const char* ISettings::getDefaultValueAs<const char*>(size_t index) {
  if (index >= getSize()) return nullptr;
  return static_cast<const char*>(getDefaultValuePtr(index));
}

/**
 * @brief Main class for setting management. Use this class to create a new setting object of the
 * selected type with its respective enum class.
 *
 * @tparam T Type of the setting
 * @tparam ENUM Enum class with all the settings
 */
template <typename T, typename ENUM>
class Settings : public ISettings {
  public:
  using Policy = typename Internal::PolicyTrait<T>::policy_type;
  using Struct = typename Internal::PolicyTrait<T>::struct_type;
  using OnChangeCb =
    typename std::function<void(const char* key, const ENUM setting, const T updated_value)>;

  Settings(std::initializer_list<Struct> list)
      : _list(list)
      , _global_on_change_cb(nullptr)
      , _global_on_change_cb_callable_on_format(false)
      , _on_change_cbs(_list.size(), nullptr)
      , _on_change_cbs_callable_on_format(_list.size(), false) {}

  const Type getType() override { return Internal::PolicyTrait<T>::enum_type; }

  const size_t getSize() override { return _list.size(); }

  /**
   * @brief Get a key string.
   * @param index Index in the list
   * @return const char* Key string, nullptr if out of bounds
   */
  const char* getKey(size_t index) override {
    if (index >= getSize()) return nullptr;
    return _list.begin()[index].key;
  }

  /**
   * @brief Get a hint string.
   * @param index Index in the list
   * @return const char* Hint string, nullptr if out of bounds
   */
  const char* getHint(size_t index) override {
    if (index >= getSize()) return nullptr;
    return _list.begin()[index].hint;
  }

  /**
   * @brief Get a pointer to the default value. You need to cast it back to the correct type.
   * @param index Index in the list
   * @return const void* Pointer to the default value, nullptr if out of bounds
   */
  const void* getDefaultValuePtr(size_t index) override {
    if (index >= getSize()) return nullptr;
    return getDefaultValuePtrImpl(index, std::is_same<T, const char*>());
  }

  /**
   * @brief Set a new value via pointer.
   * @param index Index in the list
   * @param value New value to store
   * @return true OK
   * @return false Failed to save or index out of bounds
   */
  bool setValuePtr(size_t index, const void* value) override {
    if (index >= getSize()) return false;
    return setValuePtrImpl(index, value, std::is_same<T, const char*>());
  }

  /**
   * @brief Get a copy of the value stored in NVS via pointer.
   * @param index Index in the list
   * @param value Pointer to a variable to store a copy of a NVS stored value
   * @param size Size of the variable
   * @return true OK
   * @return false Failed to retrieve or index out of bounds
   */
  bool getValuePtr(size_t index, void* value, size_t size) override {
    if (index >= getSize()) return false;
    return getValuePtrImpl(index, value, size, std::is_same<T, const char*>());
  }

  /**
   * @brief Set a global callback for all the settings of the object. This callback will be called
   * when a setting is changed.
   * @param callback Callback function
   * @param callable_on_format If the callback should be called when the setting is formatted
   */
  void setGlobalOnChangeCallback(GlobalOnChangeCb callback, bool callable_on_format) override {
    _global_on_change_cb                    = callback;
    _global_on_change_cb_callable_on_format = callable_on_format;
  }

  /**
   * @brief Clear the global callback for all the settings of the object.
   */
  void clearGlobalOnChangeCallback() override {
    _global_on_change_cb                    = nullptr;
    _global_on_change_cb_callable_on_format = false;
  }

  /**
   * @brief Check if a key exists in the list.
   * @param key Key string
   * @param index_found Index of the key found
   * @return true Key found
   * @return false Key not found
   */
  bool hasKey(const char* key, size_t& index_found) override {
    for (size_t i = 0; i < getSize(); i++) {
      if (strcmp(_list.begin()[i].key, key) == 0) {
        index_found = i;
        return true;
      }
    }

    return false;
  }

  /**
   * @brief Check if a setting is formatteable.
   * @param index Index in the list
   * @return true Formatteable
   * @return false Not formatteable or index out of bounds
   */
  bool isFormatteable(size_t index) override {
    if (index >= getSize()) return false;
    return _list.begin()[index].formatteable;
  }

  /**
   * @brief Format a setting to its default value.
   * @param index Index in the list
   * @param force Force the format even if the setting is not formatteable
   * @return true OK
   * @return false Failed to format or index out of bounds
   */
  bool format(size_t index, bool force = false) override {
    if (index >= getSize()) return false;

    if (!isFormatteable(index) && !force) return false;

    return setValueImpl(static_cast<ENUM>(index), getDefaultValue(index), true);
  }

  /**
   * @brief Format all settings to their default values.
   * @param force Force the format even if the setting is not formatteable
   * @return size_t Number of errors while trying to format
   */
  size_t formatAll(bool force = false) override {
    size_t errors = 0;

    for (size_t i = 0; i < getSize(); i++) {
      if (!isFormatteable(i) && !force) continue;

      if (!setValueImpl(static_cast<ENUM>(i), getDefaultValue(i), true)) errors++;
    }

    return errors;
  }

  /**
   * @brief Format a setting to its default value.
   * @param ENUM Selected setting
   * @param force Force the format even if the setting is not formatteable
   * @return true OK
   * @return false Failed to format
   */
  bool format(ENUM setting, bool force = false) {
    return format(static_cast<size_t>(setting), force);
  }

  /**
   * @brief Get a key string.
   * @param index Index in the list
   * @return const char* Key string
   */
  const char* getKey(ENUM setting) { return _list.begin()[static_cast<size_t>(setting)].key; }

  /**
   * @brief Get a hint string.
   * @param index Index in the list
   * @return const char* Hint string
   */
  const char* getHint(ENUM setting) { return _list.begin()[static_cast<size_t>(setting)].hint; }

  /**
   * @brief Get a default value.
   * @param index Index in the list
   * @return T Setting default value or a default value of the type if out of bounds
   */
  T getDefaultValue(size_t index) {
    if (index >= getSize()) return T();
    return _list.begin()[index].default_value;
  }

  /**
   * @brief Get a default value.
   * @param ENUM Selected setting
   * @return T Setting default value
   */
  T getDefaultValue(ENUM setting) {
    return _list.begin()[static_cast<size_t>(setting)].default_value;
  }

  /**
   * @brief Set a new value.
   * @param ENUM Selected setting
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM setting, const T value) { return setValueImpl(setting, value, false); }

  /**
   * @brief Get a current value stored in NVS.
   * @param ENUM Selected setting
   * @return T Current value
   */
  T getValue(ENUM setting) {
    return _policy.getValue(getKey(static_cast<size_t>(setting)), getDefaultValue(setting));
  }

  /**
   * @brief Set a callback for a specific setting.
   * @param ENUM Selected setting
   * @param callback Callback function
   * @param callable_on_format If the callback should be called when the setting is formatted
   */
  void setOnChangeCallback(ENUM setting, OnChangeCb callback, bool callable_on_format) {
    _on_change_cbs[static_cast<size_t>(setting)]                    = callback;
    _on_change_cbs_callable_on_format[static_cast<size_t>(setting)] = callable_on_format;
  }

  /**
   * @brief Clear the callback for a specific setting.
   * @param ENUM Selected setting
   */
  void clearOnChangeCallback(ENUM setting) {
    _on_change_cbs[static_cast<size_t>(setting)]                    = nullptr;
    _on_change_cbs_callable_on_format[static_cast<size_t>(setting)] = false;
  }

  /**
   * @brief Check if a setting is formatteable.
   * @param ENUM Selected setting
   * @return true Formatteable
   * @return false Not formatteable
   */
  bool isFormatteable(ENUM setting) {
    return _list.begin()[static_cast<size_t>(setting)].formatteable;
  }

  private:
  std::initializer_list<Struct> _list;

  GlobalOnChangeCb _global_on_change_cb;
  bool _global_on_change_cb_callable_on_format;

  std::vector<OnChangeCb> _on_change_cbs;
  std::vector<bool> _on_change_cbs_callable_on_format;

  Policy _policy;

  bool setValueImpl(ENUM setting, const T value, bool called_from_format) {
    size_t index = static_cast<size_t>(setting);

    if (!_policy.setValue(getKey(index), value)) return false;

    bool should_call_global_callback =
      called_from_format ? _global_on_change_cb_callable_on_format : true;

    bool should_call_local_callback =
      called_from_format ? _on_change_cbs_callable_on_format[index] : true;

    if (should_call_global_callback && _global_on_change_cb) {
      runGlobalCallback(getKey(setting), getType(), index, value, std::is_same<T, const char*>());
    }

    if (should_call_local_callback && _on_change_cbs[index]) {
      _on_change_cbs[index](getKey(setting), setting, value);
    }

    return true;
  }

  // Template specializations for const char* (strings)
  template <typename U = T>
  const void* getDefaultValuePtrImpl(size_t index, std::true_type) {
    return static_cast<const void*>(_list.begin()[index].default_value);
  }

  template <typename U = T>
  bool setValuePtrImpl(size_t index, const void* value, std::true_type) {
    return setValueImpl(static_cast<ENUM>(index), static_cast<U>(value), false);
  }

  template <typename U = T>
  bool getValuePtrImpl(size_t index, void* value, size_t size, std::true_type) {
    const char* temp = getValue(static_cast<ENUM>(index));
    size_t len       = strlen(temp);

    if (size < len + 1) return false;

    strncpy(static_cast<char*>(value), temp, len);
    static_cast<char*>(value)[len] = '\0';
    return true;
  }

  template <typename U = T>
  void runGlobalCallback(const char* key, Type type, size_t index, U value, std::true_type) {
    _global_on_change_cb(key, type, index, value);
  }

  // Template specializations for all other types
  template <typename U = T>
  const void* getDefaultValuePtrImpl(size_t index, std::false_type) {
    return static_cast<const void*>(&(_list.begin()[index].default_value));
  }

  template <typename U = T>
  bool setValuePtrImpl(size_t index, const void* value, std::false_type) {
    return setValueImpl(static_cast<ENUM>(index), *static_cast<const U*>(value), false);
  }

  template <typename U = T>
  bool getValuePtrImpl(size_t index, void* value, size_t size, std::false_type) {
    U temp = getValue(static_cast<ENUM>(index));

    if (size < sizeof(U)) return false;

    memcpy(value, &temp, sizeof(U));
    return true;
  }

  template <typename U = T>
  void runGlobalCallback(const char* key, Type type, size_t index, U value, std::false_type) {
    _global_on_change_cb(key, type, index, &value);
  }
};
} // namespace NVS