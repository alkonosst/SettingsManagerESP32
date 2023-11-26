/**
 * SPDX-FileCopyrightText: 2023 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SETTINGS_MANAGER_ESP32_H
#define SETTINGS_MANAGER_ESP32_H

#include <Arduino.h>
#include <Preferences.h>
#include <initializer_list>

// X-macros
#define SETTINGS_EXPAND_ENUM_CLASS(name, text, value) name,
#define SETTINGS_EXPAND_SETTINGS(name, text, value)   {#name, text, value},

// NVS (non-volatile storage ESP32)
extern Preferences nvs;

// Different type of settings to save in NVS
struct TypeFlag {
  const char* key;
  const char* text;
  const bool default_value;
};

struct TypeUInt32 {
  const char* key;
  const char* text;
  const uint32_t default_value;
};

struct TypeInt32 {
  const char* key;
  const char* text;
  const int32_t default_value;
};

struct TypeFloat {
  const char* key;
  const char* text;
  const float default_value;
};

struct TypeDouble {
  const char* key;
  const char* text;
  const double default_value;
};

struct TypeString {
  const char* key;
  const char* text;
  const char* default_value;
};

// Type of Setting object. Useful for use with pointers.
enum class SettingsType : uint8_t { FLAG, UINT32, INT32, FLOAT, DOUBLE, STRING };

/**
 * @brief Abstract class to represent a Setting. Multiple Setting classes with different types
 * derive from this one. You can create a pointer of this class to point to a derived class and
 * access their data.
 */
class Settings {
  public:
  /**
   * @brief Get the type of the object.
   * @return SettingsType Type enum
   */
  virtual SettingsType getType() = 0;

  /**
   * @brief Get the size of the setting list.
   * @return const size_t Number of settings
   */
  virtual const size_t getSize() = 0;

  /**
   * @brief Get a key string. Returns nullptr if the index is out of bounds.
   * @param index Index in the list
   * @return const char* String
   */
  virtual const char* getKey(size_t index) = 0;

  /**
   * @brief Get a description text string. Returns nullptr if the index is out of bounds.
   * @param index Index in the list
   * @return const char* String
   */
  virtual const char* getText(size_t index) = 0;

  /**
   * @brief Get a pointer to the default value. You need to cast it back to the correct type.
   * Returns nullptr if the index is out of bounds.
   * @param index Index in the list
   * @return const void* Pointer to the value
   */
  virtual const void* getDefaultValuePtr(size_t index) = 0;

  /**
   * @brief Get a copy of the value stored in NVS. You need to provide a variable passed via
   * pointer. If the index is out of bounds, value will be 0 (empty string in case of string type).
   * @param index Index in the list
   * @param value Pointer to a variable to store a copy of a NVS stored value
   */
  virtual void getValuePtr(size_t index, void* value) = 0;

  /**
   * @brief Format all settings in the list, and store them their default values.
   * @return size_t Number of errors while trying to format
   */
  virtual size_t format() = 0;

  /**
   * @brief Get a default value casted to type passed as template parameter.
   * @tparam T Type to cast the value
   * @param index Index in the list
   * @return T Default value
   */
  template <typename T>
  T getDefaultValueAs(size_t index) {
    return (index >= getSize() ? 0 : *(static_cast<const T*>(getDefaultValuePtr(index))));
  }
};

// Template specialization for const char* (strings)
template <>
inline const char* Settings::getDefaultValueAs<const char*>(size_t index) {
  return (index >= getSize() ? nullptr : static_cast<const char*>(getDefaultValuePtr(index)));
}

template <typename ENUM, typename STRUCT, typename TYPE>
class SettingsBase : public Settings {
  public:
  const size_t getSize() { return _list.size(); }

  const char* getKey(size_t index) {
    return (index >= getSize() ? nullptr : _list.begin()[index].key);
  }

  const char* getKey(ENUM setting) { return _list.begin()[static_cast<size_t>(setting)].key; }

  /**
   * @brief Get a default value.
   * @param index Index in the list
   * @return const TYPE Type
   */
  const TYPE getDefaultValue(size_t index) {
    return (index >= getSize() ? 0 : _list.begin()[index].default_value);
  }

  /**
   * @brief Get a default value.
   * @param setting Enum class member
   * @return const TYPE Type
   */
  const TYPE getDefaultValue(ENUM setting) {
    return _list.begin()[static_cast<size_t>(setting)].default_value;
  }

  const void* getDefaultValuePtr(size_t index) {
    return (index >= getSize() ? nullptr
                               : static_cast<const void*>(&(_list.begin()[index].default_value)));
  }

  const char* getText(size_t index) {
    return (index >= getSize() ? nullptr : _list.begin()[index].text);
  }

  const char* getText(ENUM setting) { return _list.begin()[static_cast<size_t>(setting)].text; }

  virtual bool setValue(ENUM setting, TYPE value) = 0;

  size_t format() {
    size_t errors = 0;

    for (size_t i = 0; i < getSize(); i++) {
      if (setValue(static_cast<ENUM>(i), getDefaultValue(i)) == 0)
        errors++;
    }

    return errors;
  }

  protected:
  SettingsBase(std::initializer_list<STRUCT> list)
      : _list(list) {}

  std::initializer_list<STRUCT> _list;
};

/* ---------------------------------------- SettingsFlag ---------------------------------------- */
template <typename ENUM_CLASS>
class SettingsFlag : public SettingsBase<ENUM_CLASS, TypeFlag, bool> {
  public:
  SettingsFlag(std::initializer_list<TypeFlag> list)
      : SettingsBase<ENUM_CLASS, TypeFlag, bool>(list) {}

  SettingsType getType() { return SettingsType::FLAG; }

  /**
   * @brief Set a new value.
   * @param setting Enum class member
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM_CLASS setting, bool value) {
    return nvs.putBool(this->_list.begin()[static_cast<size_t>(setting)].key, value) == 0 ? false
                                                                                          : true;
  }

  /**
   * @brief Get a current value stored in NVS.
   * @param setting Enum class member
   * @return true
   * @return false
   */
  bool getValue(ENUM_CLASS setting) {
    return nvs.getBool(this->_list.begin()[static_cast<size_t>(setting)].key,
                       this->_list.begin()[static_cast<size_t>(setting)].default_value);
  }

  void getValuePtr(size_t index, void* value) {
    bool temp = index >= this->getSize() ? 0 : getValue(static_cast<ENUM_CLASS>(index));
    memcpy(value, &temp, sizeof(bool));
  }
};

#define SETTINGS_CREATE_FLAGS(name, settings_macro)                         \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  SettingsFlag<name> name##_list = {settings_macro(SETTINGS_EXPAND_SETTINGS)};
/* ---------------------------------------- SettingsFlag ---------------------------------------- */

/* --------------------------------------- SettingsUInt32 --------------------------------------- */
template <typename ENUM_CLASS>
class SettingsUInt32 : public SettingsBase<ENUM_CLASS, TypeUInt32, uint32_t> {
  public:
  SettingsUInt32(std::initializer_list<TypeUInt32> list)
      : SettingsBase<ENUM_CLASS, TypeUInt32, uint32_t>(list) {}

  SettingsType getType() { return SettingsType::UINT32; }

  /**
   * @brief Set a new value.
   * @param setting Enum class member
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM_CLASS setting, uint32_t value) {
    return nvs.putUInt(this->_list.begin()[static_cast<size_t>(setting)].key, value) == 0 ? false
                                                                                          : true;
  }

  /**
   * @brief Get a current value stored in NVS.
   * @param setting Enum class member
   * @return uint32_t Current value
   */
  uint32_t getValue(ENUM_CLASS setting) {
    return nvs.getUInt(this->_list.begin()[static_cast<size_t>(setting)].key,
                       this->_list.begin()[static_cast<size_t>(setting)].default_value);
  }

  void getValuePtr(size_t index, void* value) {
    uint32_t temp = index >= this->getSize() ? 0 : getValue(static_cast<ENUM_CLASS>(index));
    memcpy(value, &temp, sizeof(uint32_t));
  }
};

#define SETTINGS_CREATE_UINT32S(name, settings_macro)                       \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  SettingsUInt32<name> name##_list = {settings_macro(SETTINGS_EXPAND_SETTINGS)};
/* --------------------------------------- SettingsUInt32 --------------------------------------- */

/* ---------------------------------------- SettingsInt32 --------------------------------------- */
template <typename ENUM_CLASS>
class SettingsInt32 : public SettingsBase<ENUM_CLASS, TypeInt32, int32_t> {
  public:
  SettingsInt32(std::initializer_list<TypeInt32> list)
      : SettingsBase<ENUM_CLASS, TypeInt32, int32_t>(list) {}

  SettingsType getType() { return SettingsType::INT32; }

  /**
   * @brief Set a new value.
   * @param setting Enum class member
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM_CLASS setting, int32_t value) {
    return nvs.putInt(this->_list.begin()[static_cast<size_t>(setting)].key, value) == 0 ? false
                                                                                         : true;
  }

  /**
   * @brief Get a current value stored in NVS.
   * @param setting Enum class member
   * @return int32_t Current value
   */
  int32_t getValue(ENUM_CLASS setting) {
    return nvs.getInt(this->_list.begin()[static_cast<size_t>(setting)].key,
                      this->_list.begin()[static_cast<size_t>(setting)].default_value);
  }

  void getValuePtr(size_t index, void* value) {
    int32_t temp = index >= this->getSize() ? 0 : getValue(static_cast<ENUM_CLASS>(index));
    memcpy(value, &temp, sizeof(int32_t));
  }
};

#define SETTINGS_CREATE_INT32S(name, settings_macro)                        \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  SettingsInt32<name> name##_list = {settings_macro(SETTINGS_EXPAND_SETTINGS)};
/* ---------------------------------------- SettingsInt32 --------------------------------------- */

/* ---------------------------------------- SettingsFloat --------------------------------------- */
template <typename ENUM_CLASS>
class SettingsFloat : public SettingsBase<ENUM_CLASS, TypeFloat, float> {
  public:
  SettingsFloat(std::initializer_list<TypeFloat> list)
      : SettingsBase<ENUM_CLASS, TypeFloat, float>(list) {}

  SettingsType getType() { return SettingsType::FLOAT; }

  /**
   * @brief Set a new value.
   * @param setting Enum class member
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM_CLASS setting, float value) {
    return nvs.putFloat(this->_list.begin()[static_cast<size_t>(setting)].key, value) == 0 ? false
                                                                                           : true;
  }

  /**
   * @brief Get a current value stored in NVS.
   * @param setting Enum class member
   * @return float Current value
   */
  float getValue(ENUM_CLASS setting) {
    return nvs.getFloat(this->_list.begin()[static_cast<size_t>(setting)].key,
                        this->_list.begin()[static_cast<size_t>(setting)].default_value);
  }

  void getValuePtr(size_t index, void* value) {
    float temp = index >= this->getSize() ? 0 : getValue(static_cast<ENUM_CLASS>(index));
    memcpy(value, &temp, sizeof(float));
  }
};

#define SETTINGS_CREATE_FLOATS(name, settings_macro)                        \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  SettingsFloat<name> name##_list = {settings_macro(SETTINGS_EXPAND_SETTINGS)};
/* ---------------------------------------- SettingsFloat --------------------------------------- */

/* --------------------------------------- SettingsDouble --------------------------------------- */
template <typename ENUM_CLASS>
class SettingsDouble : public SettingsBase<ENUM_CLASS, TypeDouble, double> {
  public:
  SettingsDouble(std::initializer_list<TypeDouble> list)
      : SettingsBase<ENUM_CLASS, TypeDouble, double>(list) {}

  SettingsType getType() { return SettingsType::DOUBLE; }

  /**
   * @brief Set a new value.
   * @param setting Enum class member
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM_CLASS setting, double value) {
    return nvs.putDouble(this->_list.begin()[static_cast<size_t>(setting)].key, value) == 0 ? false
                                                                                            : true;
  }

  /**
   * @brief Get a current value stored in NVS.
   * @param setting Enum class member
   * @return double Current value
   */
  double getValue(ENUM_CLASS setting) {
    return nvs.getDouble(this->_list.begin()[static_cast<size_t>(setting)].key,
                         this->_list.begin()[static_cast<size_t>(setting)].default_value);
  }

  void getValuePtr(size_t index, void* value) {
    double temp = index >= this->getSize() ? 0 : getValue(static_cast<ENUM_CLASS>(index));
    memcpy(value, &temp, sizeof(double));
  }
};

#define SETTINGS_CREATE_DOUBLES(name, settings_macro)                       \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  SettingsDouble<name> name##_list = {settings_macro(SETTINGS_EXPAND_SETTINGS)};
/* --------------------------------------- SettingsDouble --------------------------------------- */

/* --------------------------------------- SettingsString --------------------------------------- */
template <typename ENUM_CLASS>
class SettingsString : public SettingsBase<ENUM_CLASS, TypeString, const char*> {
  public:
  SettingsString(std::initializer_list<TypeString> list)
      : SettingsBase<ENUM_CLASS, TypeString, const char*>(list) {}

  SettingsType getType() { return SettingsType::STRING; }

  /**
   * @brief Set a new value.
   * @param setting Enum class member
   * @param value New value to store
   * @return true OK
   * @return false Failed to save
   */
  bool setValue(ENUM_CLASS setting, const char* value) {
    return nvs.putString(this->_list.begin()[static_cast<size_t>(setting)].key, value) == 0 ? false
                                                                                            : true;
  }

  /**
   * @brief Get a current value stored in NVS.
   * @param setting Enum class member
   * @return String Current value
   */
  String getValue(ENUM_CLASS setting) {
    return nvs.getString(this->_list.begin()[static_cast<size_t>(setting)].key,
                         this->_list.begin()[static_cast<size_t>(setting)].default_value);
  }

  void getValuePtr(size_t index, void* value) {
    String temp = index >= this->getSize() ? "" : getValue(static_cast<ENUM_CLASS>(index));
    strcpy(static_cast<char*>(value), temp.c_str());
  }

  /**
   * @brief Get a pointer to the default value. You need to cast it back to const char*. Returns
   * nullptr if the index is out of bounds.
   * @param index Index in the list
   * @return const void* Pointer to the value
   */
  const void* getDefaultValuePtr(size_t index) {
    return (index >= this->getSize() ? nullptr : this->_list.begin()[index].default_value);
  }
};

#define SETTINGS_CREATE_STRINGS(name, settings_macro)                       \
  enum class name : uint8_t { settings_macro(SETTINGS_EXPAND_ENUM_CLASS) }; \
  SettingsString<name> name##_list = {settings_macro(SETTINGS_EXPAND_SETTINGS)};
/* --------------------------------------- SettingsString --------------------------------------- */

#endif // SETTINGS_MANAGER_ESP32_H