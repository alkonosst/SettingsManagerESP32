/**
 * SPDX-FileCopyrightText: 2025 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Arduino.h>

#define UNITY_INCLUDE_DOUBLE
#include <unity.h>

#include "SettingsManagerESP32.h"

/* ---------------------------------------------------------------------------------------------- */
// - 3 settings for each type. The test will write to NVS only 2 of them, the third will be
//   undefined (not written to NVS), to test the behavior of the library when trying to read a
//   non-existent key (should return the default value).
// - Made all not-formatteable to test two different outcomes (force format and normal format)

// key, hint, default value, formatteable
#define BOOLS(X)                       \
  X(Bool_1, "My Bool 1", false, false) \
  X(Bool_2, "My Bool 2", true, false)  \
  X(Bool_3, "My Bool 3", false, false)

#define UINT32S(X)                     \
  X(UInt32_1, "My UInt32 1", 1, false) \
  X(UInt32_2, "My UInt32 2", 2, false) \
  X(UInt32_3, "My UInt32 3", 3, false)

#define INT32S(X)                     \
  X(Int32_1, "My Int32 1", -1, false) \
  X(Int32_2, "My Int32 2", -2, false) \
  X(Int32_3, "My Int32 3", -3, false)

#define FLOATS(X)                      \
  X(Float_1, "My Float 1", 1.1, false) \
  X(Float_2, "My Float 2", 2.2, false) \
  X(Float_3, "My Float 3", 3.3, false)

#define DOUBLES(X)                               \
  X(Double_1, "My Double 1", 1.123456789, false) \
  X(Double_2, "My Double 2", 2.123456789, false) \
  X(Double_3, "My Double 3", 3.123456789, false)

#define STRINGS(X)                           \
  X(String_1, "My String 1", "str 1", false) \
  X(String_2, "My String 2", "str 2", false) \
  X(String_3, "My String 3", "str 3", false)

const NVS::ByteStream bytestream1_default = {reinterpret_cast<const uint8_t*>("nvs1"), 4};
const NVS::ByteStream bytestream2_default = {reinterpret_cast<const uint8_t*>("nvs2"), 4};
const NVS::ByteStream bytestream3_default = {reinterpret_cast<const uint8_t*>("nvs3"), 4};

#define BYTESTREAMS(X)                                       \
  X(Stream_1, "My ByteStream 1", bytestream1_default, false) \
  X(Stream_2, "My ByteStream 2", bytestream2_default, false) \
  X(Stream_3, "My ByteStream 3", bytestream3_default, false)

constexpr uint8_t TOTAL_VALUES = 3;
constexpr uint8_t NVS_VALUES   = 2;

// New values
bool new_bool[NVS_VALUES]                  = {true, false};
uint32_t new_uint32[NVS_VALUES]            = {11, 12};
int32_t new_int32[NVS_VALUES]              = {-11, -12};
float new_float[NVS_VALUES]                = {10.1, 10.2};
double new_double[NVS_VALUES]              = {11.123456789, 22.123456789};
const char* new_string[NVS_VALUES]         = {"hello 1", "hello 2"};
NVS::ByteStream new_bytestream[NVS_VALUES] = {
  {reinterpret_cast<const uint8_t*>("test1"), 5},
  {reinterpret_cast<const uint8_t*>("test2"), 5}
};

// Instantiation of settings
enum class Bools : uint8_t { BOOLS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<bool, Bools> bools = {BOOLS(SETTINGS_EXPAND_SETTINGS)};

enum class UInt32s : uint8_t { UINT32S(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<uint32_t, UInt32s> uint32s = {UINT32S(SETTINGS_EXPAND_SETTINGS)};

enum class Int32s : uint8_t { INT32S(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<int32_t, Int32s> int32s = {INT32S(SETTINGS_EXPAND_SETTINGS)};

enum class Floats : uint8_t { FLOATS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<float, Floats> floats = {FLOATS(SETTINGS_EXPAND_SETTINGS)};

enum class Doubles : uint8_t { DOUBLES(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<double, Doubles> doubles = {DOUBLES(SETTINGS_EXPAND_SETTINGS)};

enum class Strings : uint8_t { STRINGS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<const char*, Strings> strings = {STRINGS(SETTINGS_EXPAND_SETTINGS)};

enum class ByteStreams : uint8_t { BYTESTREAMS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<NVS::ByteStream, ByteStreams> bytestreams = {BYTESTREAMS(SETTINGS_EXPAND_SETTINGS)};

NVS::ISettings* settings[] = {&bools, &uint32s, &int32s, &floats, &doubles, &strings, &bytestreams};
constexpr size_t settings_size = sizeof(settings) / sizeof(settings[0]);

// Indexes for pointers
enum SettingIndex {
  ID_Bools = 0,
  ID_UInt32s,
  ID_Int32s,
  ID_Floats,
  ID_Doubles,
  ID_Strings,
  ID_ByteStreams
};

// Callbacks
// 3 entries for each setting (when formatting the list)
// 2 entries for each setting (when setting a value)
constexpr uint8_t expected_global_callback_entries = settings_size * (TOTAL_VALUES + NVS_VALUES);

// One entry for each setting (only in setValue; callback disabled on format)
constexpr uint8_t expected_individual_callback_entries = NVS_VALUES;

uint8_t global_callback_entries = 0;
void globalCallback(const char* key, const NVS::Type type, const size_t index,
                    const void* const value) {
  global_callback_entries++;
}

uint8_t bool_callback_entries = 0;
void boolCallback(const char* key, const Bools setting, const bool value) {
  bool_callback_entries++;
}

uint8_t uint32_callback_entries = 0;
void uint32Callback(const char* key, const UInt32s setting, const uint32_t value) {
  uint32_callback_entries++;
}

uint8_t int32_callback_entries = 0;
void int32Callback(const char* key, const Int32s setting, const int32_t value) {
  int32_callback_entries++;
}

uint8_t float_callback_entries = 0;
void floatCallback(const char* key, const Floats setting, const float value) {
  float_callback_entries++;
}

uint8_t double_callback_entries = 0;
void doubleCallback(const char* key, const Doubles setting, const double value) {
  double_callback_entries++;
}

uint8_t string_callback_entries = 0;
void stringCallback(const char* key, const Strings setting, const char* const value) {
  string_callback_entries++;
}

uint8_t bytestream_callback_entries = 0;
void bytestreamCallback(const char* key, const ByteStreams setting, const NVS::ByteStream value) {
  bytestream_callback_entries++;
}

// Test functions
void test_initializeNVS();
void test_clearNVS();
void test_getType();
void test_getSize();

void test_bools_getKey();
void test_bools_getHint();
void test_bools_getDefaultValue();
void test_bools_setValue();
void test_bools_getValue();
void test_bools_hasKey();
void test_bools_pointer();
void test_bools_format();
void test_bools_forceFormat();

void test_uint32s_getKey();
void test_uint32s_getHint();
void test_uint32s_getDefaultValue();
void test_uint32s_setValue();
void test_uint32s_getValue();
void test_uint32s_hasKey();
void test_uint32s_pointer();
void test_uint32s_format();
void test_uint32s_forceFormat();

void test_int32s_getKey();
void test_int32s_getHint();
void test_int32s_getDefaultValue();
void test_int32s_setValue();
void test_int32s_getValue();
void test_int32s_hasKey();
void test_int32s_pointer();
void test_int32s_format();
void test_int32s_forceFormat();

void test_floats_getKey();
void test_floats_getHint();
void test_floats_getDefaultValue();
void test_floats_setValue();
void test_floats_getValue();
void test_floats_hasKey();
void test_floats_pointer();
void test_floats_format();
void test_floats_forceFormat();

void test_doubles_getKey();
void test_doubles_getHint();
void test_doubles_getDefaultValue();
void test_doubles_setValue();
void test_doubles_getValue();
void test_doubles_hasKey();
void test_doubles_pointer();
void test_doubles_format();
void test_doubles_forceFormat();

void test_strings_getKey();
void test_strings_getHint();
void test_strings_getDefaultValue();
void test_strings_setValue();
void test_strings_getValue();
void test_strings_hasKey();
void test_strings_pointer();
void test_strings_format();
void test_strings_forceFormat();

void test_bytestreams_getKey();
void test_bytestreams_getHint();
void test_bytestreams_getDefaultValue();
void test_bytestreams_setValue();
void test_bytestreams_getValue();
void test_bytestreams_hasKey();
void test_bytestreams_pointer();
void test_bytestreams_format();
void test_bytestreams_forceFormat();

void test_validate_global_callback_entries();
void test_validate_individual_callback_entries();
/* ---------------------------------------------------------------------------------------------- */

void setup() {
  delay(2000);

  // Set callbacks
  for (size_t i = 0; i < settings_size; i++) {
    settings[i]->setGlobalOnChangeCallback(globalCallback, true);
  }

  for (size_t i = 0; i < NVS_VALUES; i++) {
    bools.setOnChangeCallback(static_cast<Bools>(i), boolCallback, false);
    uint32s.setOnChangeCallback(static_cast<UInt32s>(i), uint32Callback, false);
    int32s.setOnChangeCallback(static_cast<Int32s>(i), int32Callback, false);
    floats.setOnChangeCallback(static_cast<Floats>(i), floatCallback, false);
    doubles.setOnChangeCallback(static_cast<Doubles>(i), doubleCallback, false);
    strings.setOnChangeCallback(static_cast<Strings>(i), stringCallback, false);
    bytestreams.setOnChangeCallback(static_cast<ByteStreams>(i), bytestreamCallback, false);
  }

  UNITY_BEGIN();

  RUN_TEST(test_initializeNVS);
  RUN_TEST(test_clearNVS);
  RUN_TEST(test_getType);
  RUN_TEST(test_getSize);

  RUN_TEST(test_bools_getKey);
  RUN_TEST(test_bools_getHint);
  RUN_TEST(test_bools_getDefaultValue);
  RUN_TEST(test_bools_setValue);
  RUN_TEST(test_bools_getValue);
  RUN_TEST(test_bools_hasKey);
  RUN_TEST(test_bools_pointer);
  RUN_TEST(test_bools_format);
  RUN_TEST(test_bools_forceFormat);

  RUN_TEST(test_uint32s_getKey);
  RUN_TEST(test_uint32s_getHint);
  RUN_TEST(test_uint32s_getDefaultValue);
  RUN_TEST(test_uint32s_setValue);
  RUN_TEST(test_uint32s_getValue);
  RUN_TEST(test_uint32s_hasKey);
  RUN_TEST(test_uint32s_pointer);
  RUN_TEST(test_uint32s_format);
  RUN_TEST(test_uint32s_forceFormat);

  RUN_TEST(test_int32s_getKey);
  RUN_TEST(test_int32s_getHint);
  RUN_TEST(test_int32s_getDefaultValue);
  RUN_TEST(test_int32s_setValue);
  RUN_TEST(test_int32s_getValue);
  RUN_TEST(test_int32s_hasKey);
  RUN_TEST(test_int32s_pointer);
  RUN_TEST(test_int32s_format);
  RUN_TEST(test_int32s_forceFormat);

  RUN_TEST(test_floats_getKey);
  RUN_TEST(test_floats_getHint);
  RUN_TEST(test_floats_getDefaultValue);
  RUN_TEST(test_floats_setValue);
  RUN_TEST(test_floats_getValue);
  RUN_TEST(test_floats_hasKey);
  RUN_TEST(test_floats_pointer);
  RUN_TEST(test_floats_format);
  RUN_TEST(test_floats_forceFormat);

  RUN_TEST(test_doubles_getKey);
  RUN_TEST(test_doubles_getHint);
  RUN_TEST(test_doubles_getDefaultValue);
  RUN_TEST(test_doubles_setValue);
  RUN_TEST(test_doubles_getValue);
  RUN_TEST(test_doubles_hasKey);
  RUN_TEST(test_doubles_pointer);
  RUN_TEST(test_doubles_format);
  RUN_TEST(test_doubles_forceFormat);

  RUN_TEST(test_strings_getKey);
  RUN_TEST(test_strings_getHint);
  RUN_TEST(test_strings_getDefaultValue);
  RUN_TEST(test_strings_setValue);
  RUN_TEST(test_strings_getValue);
  RUN_TEST(test_strings_hasKey);
  RUN_TEST(test_strings_pointer);
  RUN_TEST(test_strings_format);
  RUN_TEST(test_strings_forceFormat);

  RUN_TEST(test_bytestreams_getKey);
  RUN_TEST(test_bytestreams_getHint);
  RUN_TEST(test_bytestreams_getDefaultValue);
  RUN_TEST(test_bytestreams_setValue);
  RUN_TEST(test_bytestreams_getValue);
  RUN_TEST(test_bytestreams_hasKey);
  RUN_TEST(test_bytestreams_pointer);
  RUN_TEST(test_bytestreams_format);
  RUN_TEST(test_bytestreams_forceFormat);

  RUN_TEST(test_validate_global_callback_entries);
  RUN_TEST(test_validate_individual_callback_entries);

  UNITY_END();
}

void loop() {}

/* ---------------------------------------------------------------------------------------------- */
void test_initializeNVS() { TEST_ASSERT(nvs.begin("esp32")); }

void test_clearNVS() { TEST_ASSERT(nvs.clear()); }

void test_getType() {
  TEST_ASSERT(bools.getType() == NVS::Type::Bool);
  TEST_ASSERT(uint32s.getType() == NVS::Type::UInt32);
  TEST_ASSERT(int32s.getType() == NVS::Type::Int32);
  TEST_ASSERT(floats.getType() == NVS::Type::Float);
  TEST_ASSERT(doubles.getType() == NVS::Type::Double);
  TEST_ASSERT(strings.getType() == NVS::Type::String);
  TEST_ASSERT(bytestreams.getType() == NVS::Type::ByteStream);
}

void test_getSize() {
  constexpr size_t TOTAL_VALUES = NVS_VALUES + 1;
  TEST_ASSERT(bools.getSize() == TOTAL_VALUES);
  TEST_ASSERT(uint32s.getSize() == TOTAL_VALUES);
  TEST_ASSERT(int32s.getSize() == TOTAL_VALUES);
  TEST_ASSERT(floats.getSize() == TOTAL_VALUES);
  TEST_ASSERT(doubles.getSize() == TOTAL_VALUES);
  TEST_ASSERT(strings.getSize() == TOTAL_VALUES);
  TEST_ASSERT(bytestreams.getSize() == TOTAL_VALUES);
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_bools_getKey() {
  TEST_ASSERT_EQUAL_STRING("Bool_1", bools.getKey(Bools::Bool_1));
  TEST_ASSERT_EQUAL_STRING("Bool_2", bools.getKey(Bools::Bool_2));
}

void test_bools_getHint() {
  TEST_ASSERT_EQUAL_STRING("My Bool 1", bools.getHint(Bools::Bool_1));
  TEST_ASSERT_EQUAL_STRING("My Bool 2", bools.getHint(Bools::Bool_2));
}

void test_bools_getDefaultValue() {
  TEST_ASSERT_EQUAL(false, bools.getDefaultValue(Bools::Bool_1));
  TEST_ASSERT_EQUAL(true, bools.getDefaultValue(Bools::Bool_2));
}

void test_bools_setValue() {
  TEST_ASSERT(bools.setValue(Bools::Bool_1, new_bool[0]));
  TEST_ASSERT(bools.setValue(Bools::Bool_2, new_bool[1]));
}

void test_bools_getValue() {
  TEST_ASSERT_EQUAL(new_bool[0], bools.getValue(Bools::Bool_1));
  TEST_ASSERT_EQUAL(new_bool[1], bools.getValue(Bools::Bool_2));
}

void test_bools_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(bools.hasKey(bools.getKey(static_cast<Bools>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_bools_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::Bool, settings[ID_Bools]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "Bool_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Bools]->getKey(i));

    // Hint
    sprintf(buffer, "My Bool %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Bools]->getHint(i));

    // Value
    bool value;
    TEST_ASSERT_TRUE(settings[ID_Bools]->getValuePtr(i, &value, sizeof(value)));
    TEST_ASSERT_EQUAL(bools.getValue(static_cast<Bools>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL(bools.getDefaultValue(static_cast<Bools>(i)),
                      settings[ID_Bools]->getDefaultValueAs<bool>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_Bools]->getSize();

  TEST_ASSERT_NULL(settings[ID_Bools]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_Bools]->getHint(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_Bools]->getDefaultValueAs<bool>(idx_out));

  bool value;
  TEST_ASSERT_FALSE(settings[ID_Bools]->getValuePtr(idx_out, &value, sizeof(bool)));

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_Bools]->getValuePtr(0, &value, 0));

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_Bools]->getValuePtr(idx_nullptr, &value, sizeof(value)));
  TEST_ASSERT_EQUAL(bools.getDefaultValue(static_cast<Bools>(idx_nullptr)), value);
}

void test_bools_format() {
  TEST_ASSERT_EQUAL(0, bools.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL(new_bool[i], bools.getValue(static_cast<Bools>(i)));
  }
}

void test_bools_forceFormat() {
  TEST_ASSERT_EQUAL(0, bools.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL(bools.getDefaultValue(static_cast<Bools>(i)),
                      bools.getValue(static_cast<Bools>(i)));
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_uint32s_getKey() {
  TEST_ASSERT_EQUAL_STRING("UInt32_1", uint32s.getKey(UInt32s::UInt32_1));
  TEST_ASSERT_EQUAL_STRING("UInt32_2", uint32s.getKey(UInt32s::UInt32_2));
}

void test_uint32s_getHint() {
  TEST_ASSERT_EQUAL_STRING("My UInt32 1", uint32s.getHint(UInt32s::UInt32_1));
  TEST_ASSERT_EQUAL_STRING("My UInt32 2", uint32s.getHint(UInt32s::UInt32_2));
}

void test_uint32s_getDefaultValue() {
  TEST_ASSERT_EQUAL_UINT32(1, uint32s.getDefaultValue(UInt32s::UInt32_1));
  TEST_ASSERT_EQUAL_UINT32(2, uint32s.getDefaultValue(UInt32s::UInt32_2));
}

void test_uint32s_setValue() {
  TEST_ASSERT(uint32s.setValue(UInt32s::UInt32_1, new_uint32[0]));
  TEST_ASSERT(uint32s.setValue(UInt32s::UInt32_2, new_uint32[1]));
}

void test_uint32s_getValue() {
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], uint32s.getValue(UInt32s::UInt32_1));
  TEST_ASSERT_EQUAL_UINT32(new_uint32[1], uint32s.getValue(UInt32s::UInt32_2));
}

void test_uint32s_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(uint32s.hasKey(uint32s.getKey(static_cast<UInt32s>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_uint32s_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::UInt32, settings[ID_UInt32s]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "UInt32_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_UInt32s]->getKey(i));

    // Hint
    sprintf(buffer, "My UInt32 %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_UInt32s]->getHint(i));

    // Value
    uint32_t value;
    TEST_ASSERT_TRUE(settings[ID_UInt32s]->getValuePtr(i, &value, sizeof(value)));
    TEST_ASSERT_EQUAL_UINT32(uint32s.getValue(static_cast<UInt32s>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(static_cast<UInt32s>(i)),
                             settings[ID_UInt32s]->getDefaultValueAs<bool>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_UInt32s]->getSize();

  TEST_ASSERT_NULL(settings[ID_UInt32s]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_UInt32s]->getHint(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_UInt32s]->getDefaultValueAs<uint32_t>(idx_out));

  uint32_t value;
  TEST_ASSERT_FALSE(settings[ID_UInt32s]->getValuePtr(idx_out, &value, sizeof(value)));

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_UInt32s]->getValuePtr(0, &value, 0));

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_UInt32s]->getValuePtr(idx_nullptr, &value, sizeof(value)));
  TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(static_cast<UInt32s>(idx_nullptr)), value);
}

void test_uint32s_format() {
  TEST_ASSERT_EQUAL(0, uint32s.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_UINT32(new_uint32[i], uint32s.getValue(static_cast<UInt32s>(i)));
  }
}

void test_uint32s_forceFormat() {
  TEST_ASSERT_EQUAL(0, uint32s.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(static_cast<UInt32s>(i)),
                             uint32s.getValue(static_cast<UInt32s>(i)));
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_int32s_getKey() {
  TEST_ASSERT_EQUAL_STRING("Int32_1", int32s.getKey(Int32s::Int32_1));
  TEST_ASSERT_EQUAL_STRING("Int32_2", int32s.getKey(Int32s::Int32_2));
}

void test_int32s_getHint() {
  TEST_ASSERT_EQUAL_STRING("My Int32 1", int32s.getHint(Int32s::Int32_1));
  TEST_ASSERT_EQUAL_STRING("My Int32 2", int32s.getHint(Int32s::Int32_2));
}

void test_int32s_getDefaultValue() {
  TEST_ASSERT_EQUAL_INT32(1, uint32s.getDefaultValue(UInt32s::UInt32_1));
  TEST_ASSERT_EQUAL_INT32(2, uint32s.getDefaultValue(UInt32s::UInt32_2));
}

void test_int32s_setValue() {
  TEST_ASSERT(int32s.setValue(Int32s::Int32_1, new_int32[0]));
  TEST_ASSERT(int32s.setValue(Int32s::Int32_2, new_int32[1]));
}

void test_int32s_getValue() {
  TEST_ASSERT_EQUAL_INT32(new_int32[0], int32s.getValue(Int32s::Int32_1));
  TEST_ASSERT_EQUAL_INT32(new_int32[1], int32s.getValue(Int32s::Int32_2));
}

void test_int32s_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(int32s.hasKey(int32s.getKey(static_cast<Int32s>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_int32s_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::Int32, settings[ID_Int32s]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "Int32_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Int32s]->getKey(i));

    // Hint
    sprintf(buffer, "My Int32 %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Int32s]->getHint(i));

    // Value
    int32_t value;
    TEST_ASSERT_TRUE(settings[ID_Int32s]->getValuePtr(i, &value, sizeof(value)));
    TEST_ASSERT_EQUAL_INT32(int32s.getValue(static_cast<Int32s>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(static_cast<Int32s>(i)),
                            settings[ID_Int32s]->getDefaultValueAs<int32_t>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_Int32s]->getSize();

  TEST_ASSERT_NULL(settings[ID_Int32s]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_Int32s]->getHint(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_Int32s]->getDefaultValueAs<int32_t>(idx_out));

  int32_t value;
  TEST_ASSERT_FALSE(settings[ID_Int32s]->getValuePtr(idx_out, &value, sizeof(value)));

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_Int32s]->getValuePtr(0, &value, 0));

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_Int32s]->getValuePtr(idx_nullptr, &value, sizeof(value)));
  TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(static_cast<Int32s>(idx_nullptr)), value);
}

void test_int32s_format() {
  TEST_ASSERT_EQUAL(0, int32s.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_INT32(new_int32[i], int32s.getValue(static_cast<Int32s>(i)));
  }
}

void test_int32s_forceFormat() {
  TEST_ASSERT_EQUAL(0, int32s.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(static_cast<Int32s>(i)),
                            int32s.getValue(static_cast<Int32s>(i)));
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_floats_getKey() {
  TEST_ASSERT_EQUAL_STRING("Float_1", floats.getKey(Floats::Float_1));
  TEST_ASSERT_EQUAL_STRING("Float_2", floats.getKey(Floats::Float_2));
}

void test_floats_getHint() {
  TEST_ASSERT_EQUAL_STRING("My Float 1", floats.getHint(Floats::Float_1));
  TEST_ASSERT_EQUAL_STRING("My Float 2", floats.getHint(Floats::Float_2));
}

void test_floats_getDefaultValue() {
  TEST_ASSERT_EQUAL_FLOAT(1.1, floats.getDefaultValue(Floats::Float_1));
  TEST_ASSERT_EQUAL_FLOAT(2.2, floats.getDefaultValue(Floats::Float_2));
}

void test_floats_setValue() {
  TEST_ASSERT(floats.setValue(Floats::Float_1, new_float[0]));
  TEST_ASSERT(floats.setValue(Floats::Float_2, new_float[1]));
}

void test_floats_getValue() {
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], floats.getValue(Floats::Float_1));
  TEST_ASSERT_EQUAL_FLOAT(new_float[1], floats.getValue(Floats::Float_2));
}

void test_floats_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(floats.hasKey(floats.getKey(static_cast<Floats>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_floats_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::Float, settings[ID_Floats]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "Float_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Floats]->getKey(i));

    // Hint
    sprintf(buffer, "My Float %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Floats]->getHint(i));

    // Value
    float value;
    TEST_ASSERT_TRUE(settings[ID_Floats]->getValuePtr(i, &value, sizeof(value)));
    TEST_ASSERT_EQUAL_FLOAT(floats.getValue(static_cast<Floats>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(static_cast<Floats>(i)),
                            settings[ID_Floats]->getDefaultValueAs<float>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_Floats]->getSize();

  TEST_ASSERT_NULL(settings[ID_Floats]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_Floats]->getHint(idx_out));
  TEST_ASSERT_EQUAL_FLOAT(0, settings[ID_Floats]->getDefaultValueAs<float>(idx_out));

  float value;
  TEST_ASSERT_FALSE(settings[ID_Floats]->getValuePtr(idx_out, &value, sizeof(value)));

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_Floats]->getValuePtr(0, &value, 0));

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_Floats]->getValuePtr(idx_nullptr, &value, sizeof(value)));
  TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(static_cast<Floats>(idx_nullptr)), value);
}

void test_floats_format() {
  TEST_ASSERT_EQUAL(0, floats.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_FLOAT(new_float[i], floats.getValue(static_cast<Floats>(i)));
  }
}

void test_floats_forceFormat() {
  TEST_ASSERT_EQUAL(0, floats.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(static_cast<Floats>(i)),
                            floats.getValue(static_cast<Floats>(i)));
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_doubles_getKey() {
  TEST_ASSERT_EQUAL_STRING("Double_1", doubles.getKey(Doubles::Double_1));
  TEST_ASSERT_EQUAL_STRING("Double_2", doubles.getKey(Doubles::Double_2));
}

void test_doubles_getHint() {
  TEST_ASSERT_EQUAL_STRING("My Double 1", doubles.getHint(Doubles::Double_1));
  TEST_ASSERT_EQUAL_STRING("My Double 2", doubles.getHint(Doubles::Double_2));
}

void test_doubles_getDefaultValue() {
  TEST_ASSERT_EQUAL_DOUBLE(1.123456789, doubles.getDefaultValue(Doubles::Double_1));
  TEST_ASSERT_EQUAL_DOUBLE(2.123456789, doubles.getDefaultValue(Doubles::Double_2));
}

void test_doubles_setValue() {
  TEST_ASSERT(doubles.setValue(Doubles::Double_1, new_double[0]));
  TEST_ASSERT(doubles.setValue(Doubles::Double_2, new_double[1]));
}

void test_doubles_getValue() {
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], doubles.getValue(Doubles::Double_1));
  TEST_ASSERT_EQUAL_DOUBLE(new_double[1], doubles.getValue(Doubles::Double_2));
}

void test_doubles_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(doubles.hasKey(doubles.getKey(static_cast<Doubles>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_doubles_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::Double, settings[ID_Doubles]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "Double_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Doubles]->getKey(i));

    // Hint
    sprintf(buffer, "My Double %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Doubles]->getHint(i));

    // Value
    double value;
    TEST_ASSERT_TRUE(settings[ID_Doubles]->getValuePtr(i, &value, sizeof(value)));
    TEST_ASSERT_EQUAL_DOUBLE(doubles.getValue(static_cast<Doubles>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(static_cast<Doubles>(i)),
                             settings[ID_Doubles]->getDefaultValueAs<double>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_Doubles]->getSize();

  TEST_ASSERT_NULL(settings[ID_Doubles]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_Doubles]->getHint(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_Doubles]->getDefaultValueAs<double>(idx_out));

  double value;
  TEST_ASSERT_FALSE(settings[ID_Doubles]->getValuePtr(idx_out, &value, sizeof(value)));

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_Doubles]->getValuePtr(0, &value, 0));

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_Doubles]->getValuePtr(idx_nullptr, &value, sizeof(value)));
  TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(static_cast<Doubles>(idx_nullptr)), value);
}

void test_doubles_format() {
  TEST_ASSERT_EQUAL(0, doubles.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_DOUBLE(new_double[i], doubles.getValue(static_cast<Doubles>(i)));
  }
}

void test_doubles_forceFormat() {
  TEST_ASSERT_EQUAL(0, doubles.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(static_cast<Doubles>(i)),
                             doubles.getValue(static_cast<Doubles>(i)));
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_strings_getKey() {
  TEST_ASSERT_EQUAL_STRING("String_1", strings.getKey(Strings::String_1));
  TEST_ASSERT_EQUAL_STRING("String_2", strings.getKey(Strings::String_2));
}

void test_strings_getHint() {
  TEST_ASSERT_EQUAL_STRING("My String 1", strings.getHint(Strings::String_1));
  TEST_ASSERT_EQUAL_STRING("My String 2", strings.getHint(Strings::String_2));
}

void test_strings_getDefaultValue() {
  TEST_ASSERT_EQUAL_STRING("str 1", strings.getDefaultValue(Strings::String_1));
  TEST_ASSERT_EQUAL_STRING("str 2", strings.getDefaultValue(Strings::String_2));
}

void test_strings_setValue() {
  TEST_ASSERT(strings.setValue(Strings::String_1, new_string[0]));
  TEST_ASSERT(strings.setValue(Strings::String_2, new_string[1]));
}

void test_strings_getValue() {
  TEST_ASSERT_EQUAL_STRING(new_string[0], strings.getValue(Strings::String_1));
  strings.giveMutex();
  TEST_ASSERT_EQUAL_STRING(new_string[1], strings.getValue(Strings::String_2));
  strings.giveMutex();
}

void test_strings_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(strings.hasKey(strings.getKey(static_cast<Strings>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_strings_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::String, settings[ID_Strings]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "String_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Strings]->getKey(i));

    // Text
    sprintf(buffer, "My String %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_Strings]->getHint(i));

    // Value
    TEST_ASSERT_TRUE(settings[ID_Strings]->getValuePtr(i, buffer, sizeof(buffer)));
    settings[ID_Strings]->giveMutex();

    TEST_ASSERT_EQUAL_STRING(strings.getValue(static_cast<Strings>(i)), buffer);
    settings[ID_Strings]->giveMutex();

    // Default value
    TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(static_cast<Strings>(i)),
                             settings[ID_Strings]->getDefaultValueAs<const char*>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_Strings]->getSize();

  TEST_ASSERT_NULL(settings[ID_Strings]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_Strings]->getHint(idx_out));
  TEST_ASSERT_NULL(settings[ID_Strings]->getDefaultValueAs<const char*>(idx_out));

  TEST_ASSERT_FALSE(settings[ID_Strings]->getValuePtr(idx_out, buffer, sizeof(buffer)));
  settings[ID_Strings]->giveMutex();

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_Strings]->getValuePtr(0, buffer, 0));
  settings[ID_Strings]->giveMutex();

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_Strings]->getValuePtr(idx_nullptr, buffer, sizeof(buffer)));
  settings[ID_Strings]->giveMutex();
  TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(static_cast<Strings>(idx_nullptr)), buffer);
}

void test_strings_format() {
  TEST_ASSERT_EQUAL(0, strings.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_STRING(new_string[i], strings.getValue(static_cast<Strings>(i)));
    strings.giveMutex();
  }
}

void test_strings_forceFormat() {
  TEST_ASSERT_EQUAL(0, strings.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(static_cast<Strings>(i)),
                             strings.getValue(static_cast<Strings>(i)));
    strings.giveMutex();
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_bytestreams_getKey() {
  TEST_ASSERT_EQUAL_STRING("Stream_1", bytestreams.getKey(ByteStreams::Stream_1));
  TEST_ASSERT_EQUAL_STRING("Stream_2", bytestreams.getKey(ByteStreams::Stream_2));
}

void test_bytestreams_getHint() {
  TEST_ASSERT_EQUAL_STRING("My ByteStream 1", bytestreams.getHint(ByteStreams::Stream_1));
  TEST_ASSERT_EQUAL_STRING("My ByteStream 2", bytestreams.getHint(ByteStreams::Stream_2));
}

void test_bytestreams_getDefaultValue() {
  NVS::ByteStream default_value;

  default_value = bytestreams.getDefaultValue(ByteStreams::Stream_1);
  TEST_ASSERT_EQUAL_UINT32(bytestream1_default.size, default_value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(bytestream1_default.data, default_value.data, default_value.size);

  default_value = bytestreams.getDefaultValue(ByteStreams::Stream_2);
  TEST_ASSERT_EQUAL_UINT32(bytestream2_default.size, default_value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(bytestream2_default.data, default_value.data, default_value.size);
}

void test_bytestreams_setValue() {
  TEST_ASSERT(bytestreams.setValue(ByteStreams::Stream_1, new_bytestream[0]));
  TEST_ASSERT(bytestreams.setValue(ByteStreams::Stream_2, new_bytestream[1]));
}

void test_bytestreams_getValue() {
  NVS::ByteStream value;

  value = bytestreams.getValue(ByteStreams::Stream_1);
  bytestreams.giveMutex();
  TEST_ASSERT_EQUAL_UINT32(new_bytestream[0].size, value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[0].data, value.data, value.size);

  value = bytestreams.getValue(ByteStreams::Stream_2);
  bytestreams.giveMutex();
  TEST_ASSERT_EQUAL_UINT32(new_bytestream[1].size, value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[1].data, value.data, value.size);
}

void test_bytestreams_hasKey() {
  size_t index_found;

  for (size_t i = 0; i < NVS_VALUES; i++) {
    TEST_ASSERT(bytestreams.hasKey(bytestreams.getKey(static_cast<ByteStreams>(i)), index_found));
    TEST_ASSERT_EQUAL(i, index_found);
  }
}

void test_bytestreams_pointer() {
  TEST_ASSERT_EQUAL(NVS::Type::ByteStream, settings[ID_ByteStreams]->getType());

  char buffer[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(buffer, "Stream_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_ByteStreams]->getKey(i));

    // Hint
    sprintf(buffer, "My ByteStream %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_ByteStreams]->getHint(i));

    // Value
    NVS::ByteStream value;
    TEST_ASSERT_TRUE(settings[ID_ByteStreams]->getValuePtr(i, &value, sizeof(value)));
    settings[ID_ByteStreams]->giveMutex();

    TEST_ASSERT_EQUAL_UINT32(bytestreams.getValue(static_cast<ByteStreams>(i)).size, value.size);
    settings[ID_ByteStreams]->giveMutex();

    TEST_ASSERT_EQUAL_HEX8_ARRAY(
      bytestreams.getValue(static_cast<ByteStreams>(i)).data, value.data, value.size);
    settings[ID_ByteStreams]->giveMutex();

    // Default value
    TEST_ASSERT_EQUAL_UINT32(bytestreams.getDefaultValue(static_cast<ByteStreams>(i)).size,
                             settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStream>(i).size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(
      bytestreams.getDefaultValue(static_cast<ByteStreams>(i)).data,
      settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStream>(i).data,
      settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStream>(i).size);
  }

  // Index out of bounds
  size_t idx_out = settings[ID_ByteStreams]->getSize();

  TEST_ASSERT_NULL(settings[ID_ByteStreams]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_ByteStreams]->getHint(idx_out));

  NVS::ByteStream def_value;
  def_value = settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStream>(idx_out);
  TEST_ASSERT_NULL(def_value.data);
  TEST_ASSERT_EQUAL_UINT32(0, def_value.size);

  NVS::ByteStream value;
  TEST_ASSERT_FALSE(settings[ID_ByteStreams]->getValuePtr(idx_out, &value, sizeof(value)));
  settings[ID_ByteStreams]->giveMutex();

  // Buffer too small
  TEST_ASSERT_FALSE(settings[ID_ByteStreams]->getValuePtr(0, &value, 0));
  settings[ID_ByteStreams]->giveMutex();

  // NVS key not saved in the NVS yet, returns default value
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_TRUE(settings[ID_ByteStreams]->getValuePtr(idx_nullptr, &value, sizeof(value)));
  settings[ID_ByteStreams]->giveMutex();

  TEST_ASSERT_EQUAL_UINT32(bytestreams.getDefaultValue(static_cast<ByteStreams>(idx_nullptr)).size,
                           value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(
    bytestreams.getDefaultValue(static_cast<ByteStreams>(idx_nullptr)).data,
    value.data,
    value.size);
}

void test_bytestreams_format() {
  TEST_ASSERT_EQUAL(0, bytestreams.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    NVS::ByteStream value = bytestreams.getValue(static_cast<ByteStreams>(i));
    bytestreams.giveMutex();
    TEST_ASSERT_EQUAL_UINT32(new_bytestream[i].size, value.size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[i].data, value.data, value.size);
  }
}

void test_bytestreams_forceFormat() {
  TEST_ASSERT_EQUAL(0, bytestreams.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    NVS::ByteStream value = bytestreams.getValue(static_cast<ByteStreams>(i));
    bytestreams.giveMutex();
    TEST_ASSERT_EQUAL_UINT32(bytestreams.getDefaultValue(static_cast<ByteStreams>(i)).size,
                             value.size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(
      bytestreams.getDefaultValue(static_cast<ByteStreams>(i)).data, value.data, value.size);
  }
}
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
void test_validate_global_callback_entries() {
  TEST_ASSERT_EQUAL(expected_global_callback_entries, global_callback_entries);
}

void test_validate_individual_callback_entries() {
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, bool_callback_entries);
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, uint32_callback_entries);
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, int32_callback_entries);
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, float_callback_entries);
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, double_callback_entries);
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, string_callback_entries);
  TEST_ASSERT_EQUAL(expected_individual_callback_entries, bytestream_callback_entries);
}

/* ---------------------------------------------------------------------------------------------- */
