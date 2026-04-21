/**
 * SPDX-FileCopyrightText: 2026 Maximiliano Ramirez <maximiliano.ramirezbravo@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Arduino.h>

#define UNITY_INCLUDE_DOUBLE
#include <unity.h>

#include <SettingsManagerESP32.h>

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

const NVS::ByteStreamView bytestream1_default = {
  reinterpret_cast<const uint8_t*>("nvs1"), 5, NVS::ByteStream::Format::Hex};
const NVS::ByteStreamView bytestream2_default = {
  reinterpret_cast<const uint8_t*>("nvs2"), 5, NVS::ByteStream::Format::Base64};
const NVS::ByteStreamView bytestream3_default = {
  reinterpret_cast<const uint8_t*>("nvs3"), 5, NVS::ByteStream::Format::JSONObject};
#define BYTESTREAMS(X)                                       \
  X(Stream_1, "My ByteStream 1", bytestream1_default, false) \
  X(Stream_2, "My ByteStream 2", bytestream2_default, false) \
  X(Stream_3, "My ByteStream 3", bytestream3_default, false)

constexpr uint8_t TOTAL_VALUES = 3;
constexpr uint8_t NVS_VALUES   = 2;

// New values
bool new_bool[NVS_VALUES]          = {true, false};
uint32_t new_uint32[NVS_VALUES]    = {11, 12};
int32_t new_int32[NVS_VALUES]      = {-11, -12};
float new_float[NVS_VALUES]        = {10.1, 10.2};
double new_double[NVS_VALUES]      = {11.123456789, 22.123456789};
const char* new_string[NVS_VALUES] = {"hello 1", "hello 2"};

// Bytestream new values
NVS::ByteStreamView new_bytestream[NVS_VALUES] = {
  {reinterpret_cast<const uint8_t*>("test1"), 5, NVS::ByteStream::Format::JSONArray},
  {reinterpret_cast<const uint8_t*>("test2"), 5, NVS::ByteStream::Format::Base64   }
};

// Instantiation of settings
enum class Bools : uint8_t { BOOLS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<bool, Bools, SETTINGS_COUNT(BOOLS)> bools("test", {BOOLS(SETTINGS_EXPAND_SETTINGS)});

enum class UInt32s : uint8_t { UINT32S(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<uint32_t, UInt32s, SETTINGS_COUNT(UINT32S)>
  uint32s("test", {UINT32S(SETTINGS_EXPAND_SETTINGS)});

enum class Int32s : uint8_t { INT32S(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<int32_t, Int32s, SETTINGS_COUNT(INT32S)> int32s("test",
                                                              {INT32S(SETTINGS_EXPAND_SETTINGS)});

enum class Floats : uint8_t { FLOATS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<float, Floats, SETTINGS_COUNT(FLOATS)> floats("test",
                                                            {FLOATS(SETTINGS_EXPAND_SETTINGS)});

enum class Doubles : uint8_t { DOUBLES(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<double, Doubles, SETTINGS_COUNT(DOUBLES)>
  doubles("test", {DOUBLES(SETTINGS_EXPAND_SETTINGS)});

enum class Strings : uint8_t { STRINGS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<NVS::Str, Strings, SETTINGS_COUNT(STRINGS)>
  strings("test", {STRINGS(SETTINGS_EXPAND_SETTINGS)});

enum class ByteStreams : uint8_t { BYTESTREAMS(SETTINGS_EXPAND_ENUM_CLASS) };
NVS::Settings<NVS::ByteStream, ByteStreams, SETTINGS_COUNT(BYTESTREAMS)>
  bytestreams("test", {BYTESTREAMS(SETTINGS_EXPAND_SETTINGS)});

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
void stringCallback(const char* key, const Strings setting, const NVS::StrView value) {
  string_callback_entries++;
}

uint8_t bytestream_callback_entries = 0;
void bytestreamCallback(const char* key, const ByteStreams setting,
                        const NVS::ByteStreamView value) {
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
void test_bools_getValueOrDefault();
void test_bools_getValuePtrOrDefault();
void test_bools_format();
void test_bools_forceFormat();

void test_uint32s_getKey();
void test_uint32s_getHint();
void test_uint32s_getDefaultValue();
void test_uint32s_setValue();
void test_uint32s_getValue();
void test_uint32s_hasKey();
void test_uint32s_pointer();
void test_uint32s_getValueOrDefault();
void test_uint32s_getValuePtrOrDefault();
void test_uint32s_format();
void test_uint32s_forceFormat();

void test_int32s_getKey();
void test_int32s_getHint();
void test_int32s_getDefaultValue();
void test_int32s_setValue();
void test_int32s_getValue();
void test_int32s_hasKey();
void test_int32s_pointer();
void test_int32s_getValueOrDefault();
void test_int32s_getValuePtrOrDefault();
void test_int32s_format();
void test_int32s_forceFormat();

void test_floats_getKey();
void test_floats_getHint();
void test_floats_getDefaultValue();
void test_floats_setValue();
void test_floats_getValue();
void test_floats_hasKey();
void test_floats_pointer();
void test_floats_getValueOrDefault();
void test_floats_getValuePtrOrDefault();
void test_floats_format();
void test_floats_forceFormat();

void test_doubles_getKey();
void test_doubles_getHint();
void test_doubles_getDefaultValue();
void test_doubles_setValue();
void test_doubles_getValue();
void test_doubles_hasKey();
void test_doubles_pointer();
void test_doubles_getValueOrDefault();
void test_doubles_getValuePtrOrDefault();
void test_doubles_format();
void test_doubles_forceFormat();

void test_strings_getKey();
void test_strings_getHint();
void test_strings_getDefaultValue();
void test_strings_setValue();
void test_strings_getValue();
void test_strings_hasKey();
void test_strings_pointer();
void test_strings_getValueOrDefault();
void test_strings_getValuePtrOrDefault();
void test_strings_format();
void test_strings_forceFormat();

void test_bytestreams_getKey();
void test_bytestreams_getHint();
void test_bytestreams_getDefaultValue();
void test_bytestreams_setValue();
void test_bytestreams_getValue();
void test_bytestreams_hasKey();
void test_bytestreams_pointer();
void test_bytestreams_getValueOrDefault();
void test_bytestreams_getValuePtrOrDefault();
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
  RUN_TEST(test_bools_getValueOrDefault);
  RUN_TEST(test_bools_getValuePtrOrDefault);
  RUN_TEST(test_bools_format);
  RUN_TEST(test_bools_forceFormat);

  RUN_TEST(test_uint32s_getKey);
  RUN_TEST(test_uint32s_getHint);
  RUN_TEST(test_uint32s_getDefaultValue);
  RUN_TEST(test_uint32s_setValue);
  RUN_TEST(test_uint32s_getValue);
  RUN_TEST(test_uint32s_hasKey);
  RUN_TEST(test_uint32s_pointer);
  RUN_TEST(test_uint32s_getValueOrDefault);
  RUN_TEST(test_uint32s_getValuePtrOrDefault);
  RUN_TEST(test_uint32s_format);
  RUN_TEST(test_uint32s_forceFormat);

  RUN_TEST(test_int32s_getKey);
  RUN_TEST(test_int32s_getHint);
  RUN_TEST(test_int32s_getDefaultValue);
  RUN_TEST(test_int32s_setValue);
  RUN_TEST(test_int32s_getValue);
  RUN_TEST(test_int32s_hasKey);
  RUN_TEST(test_int32s_pointer);
  RUN_TEST(test_int32s_getValueOrDefault);
  RUN_TEST(test_int32s_getValuePtrOrDefault);
  RUN_TEST(test_int32s_format);
  RUN_TEST(test_int32s_forceFormat);

  RUN_TEST(test_floats_getKey);
  RUN_TEST(test_floats_getHint);
  RUN_TEST(test_floats_getDefaultValue);
  RUN_TEST(test_floats_setValue);
  RUN_TEST(test_floats_getValue);
  RUN_TEST(test_floats_hasKey);
  RUN_TEST(test_floats_pointer);
  RUN_TEST(test_floats_getValueOrDefault);
  RUN_TEST(test_floats_getValuePtrOrDefault);
  RUN_TEST(test_floats_format);
  RUN_TEST(test_floats_forceFormat);

  RUN_TEST(test_doubles_getKey);
  RUN_TEST(test_doubles_getHint);
  RUN_TEST(test_doubles_getDefaultValue);
  RUN_TEST(test_doubles_setValue);
  RUN_TEST(test_doubles_getValue);
  RUN_TEST(test_doubles_hasKey);
  RUN_TEST(test_doubles_pointer);
  RUN_TEST(test_doubles_getValueOrDefault);
  RUN_TEST(test_doubles_getValuePtrOrDefault);
  RUN_TEST(test_doubles_format);
  RUN_TEST(test_doubles_forceFormat);

  RUN_TEST(test_strings_getKey);
  RUN_TEST(test_strings_getHint);
  RUN_TEST(test_strings_getDefaultValue);
  RUN_TEST(test_strings_setValue);
  RUN_TEST(test_strings_getValue);
  RUN_TEST(test_strings_hasKey);
  RUN_TEST(test_strings_pointer);
  RUN_TEST(test_strings_getValueOrDefault);
  RUN_TEST(test_strings_getValuePtrOrDefault);
  RUN_TEST(test_strings_format);
  RUN_TEST(test_strings_forceFormat);

  RUN_TEST(test_bytestreams_getKey);
  RUN_TEST(test_bytestreams_getHint);
  RUN_TEST(test_bytestreams_getDefaultValue);
  RUN_TEST(test_bytestreams_setValue);
  RUN_TEST(test_bytestreams_getValue);
  RUN_TEST(test_bytestreams_hasKey);
  RUN_TEST(test_bytestreams_pointer);
  RUN_TEST(test_bytestreams_getValueOrDefault);
  RUN_TEST(test_bytestreams_getValuePtrOrDefault);
  RUN_TEST(test_bytestreams_format);
  RUN_TEST(test_bytestreams_forceFormat);

  RUN_TEST(test_validate_global_callback_entries);
  RUN_TEST(test_validate_individual_callback_entries);

  UNITY_END();
}

void loop() {}

/* ---------------------------------------------------------------------------------------------- */
void test_initializeNVS() {
  TEST_ASSERT(NVS::init());
  for (size_t i = 0; i < settings_size; i++) {
    TEST_ASSERT(settings[i]->begin());
  }
}

void test_clearNVS() {
  // Erase all keys in the shared "test" namespace via the first open handle.
  // All other objects share the same namespace, so their keys are erased too.
  TEST_ASSERT(settings[0]->eraseAll());
}

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
  bool val;
  TEST_ASSERT(bools.getValue(Bools::Bool_1, val));
  TEST_ASSERT_EQUAL(new_bool[0], val);
  TEST_ASSERT(bools.getValue(Bools::Bool_2, val));
  TEST_ASSERT_EQUAL(new_bool[1], val);
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
    bool bval;
    bools.getValue(static_cast<Bools>(i), bval);
    TEST_ASSERT_EQUAL(bval, value);

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

  // NVS key not saved yet - getValuePtr returns false and leaves value unmodified
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_FALSE(settings[ID_Bools]->getValuePtr(idx_nullptr, &value, sizeof(value)));
}

void test_bools_getValueOrDefault() {
  bool out;

  // Key exists in NVS (Bool_1 after setValue): should return the NVS value
  bool result = bools.getValueOrDefault(Bools::Bool_1, out);
  TEST_ASSERT_EQUAL(new_bool[0], result);
  TEST_ASSERT_EQUAL(new_bool[0], out);

  // Key not in NVS (Bool_3 was never written): should return the default value
  result = bools.getValueOrDefault(Bools::Bool_3, out);
  TEST_ASSERT_EQUAL(bools.getDefaultValue(Bools::Bool_3), result);
  TEST_ASSERT_EQUAL(bools.getDefaultValue(Bools::Bool_3), out);
}

void test_bools_getValuePtrOrDefault() {
  bool out;
  const void* result;

  // Key exists in NVS (index 0 = Bool_1): returns non-null pointer to the NVS value
  result = settings[ID_Bools]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL(new_bool[0], *static_cast<const bool*>(result));
  TEST_ASSERT_EQUAL(new_bool[0], out);

  // Key not in NVS (index 2 = Bool_3): returns non-null pointer to the default value
  result = settings[ID_Bools]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL(bools.getDefaultValue(Bools::Bool_3), *static_cast<const bool*>(result));
  TEST_ASSERT_EQUAL(bools.getDefaultValue(Bools::Bool_3), out);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(
    settings[ID_Bools]->getValuePtrOrDefault(settings[ID_Bools]->getSize(), &out, sizeof(out)));

  // Buffer too small: returns nullptr
  TEST_ASSERT_NULL(settings[ID_Bools]->getValuePtrOrDefault(0, &out, 0));
}

void test_bools_format() {
  TEST_ASSERT_EQUAL(0, bools.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    bool val;
    bools.getValue(static_cast<Bools>(i), val);
    TEST_ASSERT_EQUAL(new_bool[i], val);
  }
}

void test_bools_forceFormat() {
  TEST_ASSERT_EQUAL(0, bools.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    bool val;
    bools.getValue(static_cast<Bools>(i), val);
    TEST_ASSERT_EQUAL(bools.getDefaultValue(static_cast<Bools>(i)), val);
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
  uint32_t val;
  TEST_ASSERT(uint32s.getValue(UInt32s::UInt32_1, val));
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], val);
  TEST_ASSERT(uint32s.getValue(UInt32s::UInt32_2, val));
  TEST_ASSERT_EQUAL_UINT32(new_uint32[1], val);
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
    uint32_t uval;
    uint32s.getValue(static_cast<UInt32s>(i), uval);
    TEST_ASSERT_EQUAL_UINT32(uval, value);

    // Default value
    TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(static_cast<UInt32s>(i)),
                             settings[ID_UInt32s]->getDefaultValueAs<uint32_t>(i));
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

  // NVS key not saved yet - getValuePtr returns false and leaves value unmodified
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_FALSE(settings[ID_UInt32s]->getValuePtr(idx_nullptr, &value, sizeof(value)));
}

void test_uint32s_getValueOrDefault() {
  uint32_t out;

  // Key exists in NVS (UInt32_1 = 11 after setValue): should return the NVS value
  uint32_t result = uint32s.getValueOrDefault(UInt32s::UInt32_1, out);
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], result);
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], out);

  // Key not in NVS (UInt32_3 was never written): should return the default value
  result = uint32s.getValueOrDefault(UInt32s::UInt32_3, out);
  TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(UInt32s::UInt32_3), result);
  TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(UInt32s::UInt32_3), out);
}

void test_uint32s_getValuePtrOrDefault() {
  uint32_t out;
  const void* result;

  // Key exists in NVS (index 0 = UInt32_1 = 11): returns non-null pointer to the NVS value
  result = settings[ID_UInt32s]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], *static_cast<const uint32_t*>(result));
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], out);

  // Key not in NVS (index 2 = UInt32_3): returns non-null pointer to the default value
  result = settings[ID_UInt32s]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(UInt32s::UInt32_3),
                           *static_cast<const uint32_t*>(result));
  TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(UInt32s::UInt32_3), out);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(
    settings[ID_UInt32s]->getValuePtrOrDefault(settings[ID_UInt32s]->getSize(), &out, sizeof(out)));

  // Buffer too small: returns nullptr
  TEST_ASSERT_NULL(settings[ID_UInt32s]->getValuePtrOrDefault(0, &out, 0));
}

void test_uint32s_format() {
  TEST_ASSERT_EQUAL(0, uint32s.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    uint32_t val;
    uint32s.getValue(static_cast<UInt32s>(i), val);
    TEST_ASSERT_EQUAL_UINT32(new_uint32[i], val);
  }
}

void test_uint32s_forceFormat() {
  TEST_ASSERT_EQUAL(0, uint32s.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    uint32_t val;
    uint32s.getValue(static_cast<UInt32s>(i), val);
    TEST_ASSERT_EQUAL_UINT32(uint32s.getDefaultValue(static_cast<UInt32s>(i)), val);
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
  TEST_ASSERT_EQUAL_INT32(-1, int32s.getDefaultValue(Int32s::Int32_1));
  TEST_ASSERT_EQUAL_INT32(-2, int32s.getDefaultValue(Int32s::Int32_2));
}

void test_int32s_setValue() {
  TEST_ASSERT(int32s.setValue(Int32s::Int32_1, new_int32[0]));
  TEST_ASSERT(int32s.setValue(Int32s::Int32_2, new_int32[1]));
}

void test_int32s_getValue() {
  int32_t val;
  TEST_ASSERT(int32s.getValue(Int32s::Int32_1, val));
  TEST_ASSERT_EQUAL_INT32(new_int32[0], val);
  TEST_ASSERT(int32s.getValue(Int32s::Int32_2, val));
  TEST_ASSERT_EQUAL_INT32(new_int32[1], val);
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
    int32_t ival;
    int32s.getValue(static_cast<Int32s>(i), ival);
    TEST_ASSERT_EQUAL_INT32(ival, value);

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

  // NVS key not saved yet - getValuePtr returns false and leaves value unmodified
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_FALSE(settings[ID_Int32s]->getValuePtr(idx_nullptr, &value, sizeof(value)));
}

void test_int32s_getValueOrDefault() {
  int32_t out;

  // Key exists in NVS (Int32_1 after setValue): should return the NVS value
  int32_t result = int32s.getValueOrDefault(Int32s::Int32_1, out);
  TEST_ASSERT_EQUAL_INT32(new_int32[0], result);
  TEST_ASSERT_EQUAL_INT32(new_int32[0], out);

  // Key not in NVS (Int32_3 was never written): should return the default value
  result = int32s.getValueOrDefault(Int32s::Int32_3, out);
  TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(Int32s::Int32_3), result);
  TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(Int32s::Int32_3), out);
}

void test_int32s_getValuePtrOrDefault() {
  int32_t out;
  const void* result;

  // Key exists in NVS (index 0 = Int32_1): returns non-null pointer to the NVS value
  result = settings[ID_Int32s]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_INT32(new_int32[0], *static_cast<const int32_t*>(result));
  TEST_ASSERT_EQUAL_INT32(new_int32[0], out);

  // Key not in NVS (index 2 = Int32_3): returns non-null pointer to the default value
  result = settings[ID_Int32s]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(Int32s::Int32_3),
                          *static_cast<const int32_t*>(result));
  TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(Int32s::Int32_3), out);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(
    settings[ID_Int32s]->getValuePtrOrDefault(settings[ID_Int32s]->getSize(), &out, sizeof(out)));

  // Buffer too small: returns nullptr
  TEST_ASSERT_NULL(settings[ID_Int32s]->getValuePtrOrDefault(0, &out, 0));
}

void test_int32s_format() {
  TEST_ASSERT_EQUAL(0, int32s.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    int32_t val;
    int32s.getValue(static_cast<Int32s>(i), val);
    TEST_ASSERT_EQUAL_INT32(new_int32[i], val);
  }
}

void test_int32s_forceFormat() {
  TEST_ASSERT_EQUAL(0, int32s.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    int32_t val;
    int32s.getValue(static_cast<Int32s>(i), val);
    TEST_ASSERT_EQUAL_INT32(int32s.getDefaultValue(static_cast<Int32s>(i)), val);
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
  float val;
  TEST_ASSERT(floats.getValue(Floats::Float_1, val));
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], val);
  TEST_ASSERT(floats.getValue(Floats::Float_2, val));
  TEST_ASSERT_EQUAL_FLOAT(new_float[1], val);
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
    float fval;
    floats.getValue(static_cast<Floats>(i), fval);
    TEST_ASSERT_EQUAL_FLOAT(fval, value);

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

  // NVS key not saved yet - getValuePtr returns false and leaves value unmodified
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_FALSE(settings[ID_Floats]->getValuePtr(idx_nullptr, &value, sizeof(value)));
}

void test_floats_getValueOrDefault() {
  float out;

  // Key exists in NVS (Float_1 after setValue): should return the NVS value
  float result = floats.getValueOrDefault(Floats::Float_1, out);
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], result);
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], out);

  // Key not in NVS (Float_3 was never written): should return the default value
  result = floats.getValueOrDefault(Floats::Float_3, out);
  TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(Floats::Float_3), result);
  TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(Floats::Float_3), out);
}

void test_floats_getValuePtrOrDefault() {
  float out;
  const void* result;

  // Key exists in NVS (index 0 = Float_1): returns non-null pointer to the NVS value
  result = settings[ID_Floats]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], *static_cast<const float*>(result));
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], out);

  // Key not in NVS (index 2 = Float_3): returns non-null pointer to the default value
  result = settings[ID_Floats]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(Floats::Float_3),
                          *static_cast<const float*>(result));
  TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(Floats::Float_3), out);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(
    settings[ID_Floats]->getValuePtrOrDefault(settings[ID_Floats]->getSize(), &out, sizeof(out)));

  // Buffer too small: returns nullptr
  TEST_ASSERT_NULL(settings[ID_Floats]->getValuePtrOrDefault(0, &out, 0));
}

void test_floats_format() {
  TEST_ASSERT_EQUAL(0, floats.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    float val;
    floats.getValue(static_cast<Floats>(i), val);
    TEST_ASSERT_EQUAL_FLOAT(new_float[i], val);
  }
}

void test_floats_forceFormat() {
  TEST_ASSERT_EQUAL(0, floats.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    float val;
    floats.getValue(static_cast<Floats>(i), val);
    TEST_ASSERT_EQUAL_FLOAT(floats.getDefaultValue(static_cast<Floats>(i)), val);
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
  double val;
  TEST_ASSERT(doubles.getValue(Doubles::Double_1, val));
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], val);
  TEST_ASSERT(doubles.getValue(Doubles::Double_2, val));
  TEST_ASSERT_EQUAL_DOUBLE(new_double[1], val);
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
    double dval;
    doubles.getValue(static_cast<Doubles>(i), dval);
    TEST_ASSERT_EQUAL_DOUBLE(dval, value);

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

  // NVS key not saved yet - getValuePtr returns false and leaves value unmodified
  uint8_t idx_nullptr = idx_out - 1;
  TEST_ASSERT_FALSE(settings[ID_Doubles]->getValuePtr(idx_nullptr, &value, sizeof(value)));
}

void test_doubles_getValueOrDefault() {
  double out;

  // Key exists in NVS (Double_1 after setValue): should return the NVS value
  double result = doubles.getValueOrDefault(Doubles::Double_1, out);
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], result);
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], out);

  // Key not in NVS (Double_3 was never written): should return the default value
  result = doubles.getValueOrDefault(Doubles::Double_3, out);
  TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(Doubles::Double_3), result);
  TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(Doubles::Double_3), out);
}

void test_doubles_getValuePtrOrDefault() {
  double out;
  const void* result;

  // Key exists in NVS (index 0 = Double_1): returns non-null pointer to the NVS value
  result = settings[ID_Doubles]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], *static_cast<const double*>(result));
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], out);

  // Key not in NVS (index 2 = Double_3): returns non-null pointer to the default value
  result = settings[ID_Doubles]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(Doubles::Double_3),
                           *static_cast<const double*>(result));
  TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(Doubles::Double_3), out);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(
    settings[ID_Doubles]->getValuePtrOrDefault(settings[ID_Doubles]->getSize(), &out, sizeof(out)));

  // Buffer too small: returns nullptr
  TEST_ASSERT_NULL(settings[ID_Doubles]->getValuePtrOrDefault(0, &out, 0));
}

void test_doubles_format() {
  TEST_ASSERT_EQUAL(0, doubles.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    double val;
    doubles.getValue(static_cast<Doubles>(i), val);
    TEST_ASSERT_EQUAL_DOUBLE(new_double[i], val);
  }
}

void test_doubles_forceFormat() {
  TEST_ASSERT_EQUAL(0, doubles.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    double val;
    doubles.getValue(static_cast<Doubles>(i), val);
    TEST_ASSERT_EQUAL_DOUBLE(doubles.getDefaultValue(static_cast<Doubles>(i)), val);
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
  TEST_ASSERT_EQUAL_STRING("str 1", strings.getDefaultValue(Strings::String_1).data);
  TEST_ASSERT_EQUAL_STRING("str 2", strings.getDefaultValue(Strings::String_2).data);
}

void test_strings_setValue() {
  TEST_ASSERT(strings.setValue(Strings::String_1, NVS::StrView{new_string[0]}));
  TEST_ASSERT(strings.setValue(Strings::String_2, NVS::StrView{new_string[1]}));
}

void test_strings_getValue() {
  char buf[32];
  NVS::Str out{buf, sizeof(buf)};
  TEST_ASSERT(strings.getValue(Strings::String_1, out));
  TEST_ASSERT_EQUAL_STRING(new_string[0], out.data);
  out = {buf, sizeof(buf)};
  TEST_ASSERT(strings.getValue(Strings::String_2, out));
  TEST_ASSERT_EQUAL_STRING(new_string[1], out.data);
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

    // Value via pointer interface
    NVS::Str ptr_out{buffer, sizeof(buffer)};
    TEST_ASSERT_TRUE(settings[ID_Strings]->getValuePtr(i, &ptr_out, sizeof(ptr_out)));
    char sval[64];
    NVS::Str sval_ns{sval, sizeof(sval)};
    strings.getValue(static_cast<Strings>(i), sval_ns);
    TEST_ASSERT_EQUAL_STRING(sval, buffer);

    // Default value
    TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(static_cast<Strings>(i)).data,
                             settings[ID_Strings]->getDefaultValueAs<NVS::StrView>(i).data);
  }

  // Index out of bounds
  size_t idx_out = settings[ID_Strings]->getSize();

  TEST_ASSERT_NULL(settings[ID_Strings]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_Strings]->getHint(idx_out));
  TEST_ASSERT_NULL(settings[ID_Strings]->getDefaultValueAs<NVS::StrView>(idx_out).data);

  NVS::Str oob{buffer, sizeof(buffer)};
  TEST_ASSERT_FALSE(settings[ID_Strings]->getValuePtr(idx_out, &oob, sizeof(oob)));

  // Buffer too small
  NVS::Str tiny{buffer, 0};
  TEST_ASSERT_FALSE(settings[ID_Strings]->getValuePtr(0, &tiny, sizeof(tiny)));

  // NVS key not saved yet - getValuePtr returns false and leaves the buffer unmodified
  uint8_t idx_nullptr = idx_out - 1;
  NVS::Str def_out{buffer, sizeof(buffer)};
  TEST_ASSERT_FALSE(settings[ID_Strings]->getValuePtr(idx_nullptr, &def_out, sizeof(def_out)));
}

void test_strings_getValueOrDefault() {
  char buf[32];
  NVS::Str out{buf, sizeof(buf)};

  // Key exists in NVS (String_1 after setValue): should return the NVS value
  NVS::Str result = strings.getValueOrDefault(Strings::String_1, out);
  TEST_ASSERT_EQUAL_STRING(new_string[0], result.data);
  TEST_ASSERT_EQUAL_STRING(new_string[0], out.data);

  // Key not in NVS (String_3 was never written): should return the default value
  out    = {buf, sizeof(buf)};
  result = strings.getValueOrDefault(Strings::String_3, out);
  TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(Strings::String_3).data, result.data);
  TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(Strings::String_3).data, out.data);
}

void test_strings_getValuePtrOrDefault() {
  char buf[32];
  NVS::Str out{buf, sizeof(buf)};
  const void* result;

  // Key exists in NVS (index 0 = String_1): returns non-null pointer to the NVS value
  result = settings[ID_Strings]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_STRING(new_string[0], static_cast<const NVS::Str*>(result)->data);
  TEST_ASSERT_EQUAL_STRING(new_string[0], out.data);

  // Key not in NVS (index 2 = String_3): returns non-null pointer to the default value
  out    = {buf, sizeof(buf)};
  result = settings[ID_Strings]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(Strings::String_3).data,
                           static_cast<const NVS::Str*>(result)->data);
  TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(Strings::String_3).data, out.data);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(
    settings[ID_Strings]->getValuePtrOrDefault(settings[ID_Strings]->getSize(), &out, sizeof(out)));

  // Buffer too small (size field = 0): returns nullptr
  NVS::Str tiny{buf, 0};
  TEST_ASSERT_NULL(settings[ID_Strings]->getValuePtrOrDefault(0, &tiny, 0));
}

void test_strings_format() {
  TEST_ASSERT_EQUAL(0, strings.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    char buf[32];
    NVS::Str out{buf, sizeof(buf)};
    TEST_ASSERT(strings.getValue(static_cast<Strings>(i), out));
    TEST_ASSERT_EQUAL_STRING(new_string[i], out.data);
  }
}

void test_strings_forceFormat() {
  TEST_ASSERT_EQUAL(0, strings.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    char buf[32];
    NVS::Str out{buf, sizeof(buf)};
    TEST_ASSERT(strings.getValue(static_cast<Strings>(i), out));
    TEST_ASSERT_EQUAL_STRING(strings.getDefaultValue(static_cast<Strings>(i)).data, out.data);
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
  NVS::ByteStreamView default_value;

  default_value = bytestreams.getDefaultValue(ByteStreams::Stream_1);
  TEST_ASSERT_EQUAL_UINT32(bytestream1_default.size, default_value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(bytestream1_default.data, default_value.data, default_value.size);
  TEST_ASSERT_EQUAL(bytestream1_default.format, default_value.format);

  default_value = bytestreams.getDefaultValue(ByteStreams::Stream_2);
  TEST_ASSERT_EQUAL_UINT32(bytestream2_default.size, default_value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(bytestream2_default.data, default_value.data, default_value.size);
  TEST_ASSERT_EQUAL(bytestream2_default.format, default_value.format);
}

void test_bytestreams_setValue() {
  TEST_ASSERT(bytestreams.setValue(ByteStreams::Stream_1, new_bytestream[0]));
  TEST_ASSERT(bytestreams.setValue(ByteStreams::Stream_2, new_bytestream[1]));
}

void test_bytestreams_getValue() {
  uint8_t buf[32];
  NVS::ByteStream value{buf, sizeof(buf)};

  TEST_ASSERT(bytestreams.getValue(ByteStreams::Stream_1, value));
  TEST_ASSERT_EQUAL_UINT32(new_bytestream[0].size, value.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[0].data, value.data, value.size);

  value = {buf, sizeof(buf)};
  TEST_ASSERT(bytestreams.getValue(ByteStreams::Stream_2, value));
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

  char key_buf[64];
  uint8_t blob_buf[64];

  for (size_t i = 0; i < NVS_VALUES; i++) {
    // Key
    sprintf(key_buf, "Stream_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(key_buf, settings[ID_ByteStreams]->getKey(i));

    // Hint
    sprintf(key_buf, "My ByteStream %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(key_buf, settings[ID_ByteStreams]->getHint(i));

    // Value via pointer interface
    NVS::ByteStream ptr_value{blob_buf, sizeof(blob_buf)};
    TEST_ASSERT_TRUE(settings[ID_ByteStreams]->getValuePtr(i, &ptr_value, sizeof(ptr_value)));

    uint8_t ref_buf[64];
    NVS::ByteStream ref_value{ref_buf, sizeof(ref_buf)};
    bytestreams.getValue(static_cast<ByteStreams>(i), ref_value);

    TEST_ASSERT_EQUAL_UINT32(ref_value.size, ptr_value.size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(ref_value.data, ptr_value.data, ptr_value.size);

    // Default value
    TEST_ASSERT_EQUAL_UINT32(
      bytestreams.getDefaultValue(static_cast<ByteStreams>(i)).size,
      settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStreamView>(i).size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(
      bytestreams.getDefaultValue(static_cast<ByteStreams>(i)).data,
      settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStreamView>(i).data,
      settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStreamView>(i).size);
  }

  // Index out of bounds
  size_t idx_out = settings[ID_ByteStreams]->getSize();

  TEST_ASSERT_NULL(settings[ID_ByteStreams]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_ByteStreams]->getHint(idx_out));

  NVS::ByteStreamView def_value;
  def_value = settings[ID_ByteStreams]->getDefaultValueAs<NVS::ByteStreamView>(idx_out);
  TEST_ASSERT_NULL(def_value.data);
  TEST_ASSERT_EQUAL_UINT32(0, def_value.size);

  NVS::ByteStream value{blob_buf, sizeof(blob_buf)};
  TEST_ASSERT_FALSE(settings[ID_ByteStreams]->getValuePtr(idx_out, &value, sizeof(value)));

  // Buffer too small for the blob (size=0 means 0-capacity buffer in our struct)
  NVS::ByteStream tiny{blob_buf, 0};
  TEST_ASSERT_FALSE(settings[ID_ByteStreams]->getValuePtr(0, &tiny, sizeof(tiny)));

  // NVS key not saved yet - getValuePtr returns false and leaves the buffer unmodified
  uint8_t idx_nullptr = idx_out - 1;
  NVS::ByteStream def_out{blob_buf, sizeof(blob_buf)};
  TEST_ASSERT_FALSE(settings[ID_ByteStreams]->getValuePtr(idx_nullptr, &def_out, sizeof(def_out)));
}

void test_bytestreams_getValueOrDefault() {
  uint8_t buf[32];
  NVS::ByteStream out{buf, sizeof(buf)};

  // Key exists in NVS (Stream_1 after setValue): should return the NVS value
  NVS::ByteStream result = bytestreams.getValueOrDefault(ByteStreams::Stream_1, out);
  TEST_ASSERT_EQUAL_UINT32(new_bytestream[0].size, result.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[0].data, result.data, result.size);
  TEST_ASSERT_EQUAL_UINT32(new_bytestream[0].size, out.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[0].data, out.data, out.size);

  // Key not in NVS (Stream_3 was never written): should return the default value
  out                      = {buf, sizeof(buf)};
  result                   = bytestreams.getValueOrDefault(ByteStreams::Stream_3, out);
  NVS::ByteStreamView def3 = bytestreams.getDefaultValue(ByteStreams::Stream_3);
  TEST_ASSERT_EQUAL_UINT32(def3.size, result.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(def3.data, result.data, result.size);
  TEST_ASSERT_EQUAL_UINT32(def3.size, out.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(def3.data, out.data, out.size);
}

void test_bytestreams_getValuePtrOrDefault() {
  uint8_t buf[32];
  NVS::ByteStream out{buf, sizeof(buf)};
  const void* result;

  // Key exists in NVS (index 0 = Stream_1): returns non-null pointer to the NVS value
  result = settings[ID_ByteStreams]->getValuePtrOrDefault(0, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_UINT32(new_bytestream[0].size,
                           static_cast<const NVS::ByteStream*>(result)->size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[0].data,
                               static_cast<const NVS::ByteStream*>(result)->data,
                               new_bytestream[0].size);

  // Key not in NVS (index 2 = Stream_3): returns non-null pointer to the default value
  out    = {buf, sizeof(buf)};
  result = settings[ID_ByteStreams]->getValuePtrOrDefault(2, &out, sizeof(out));
  TEST_ASSERT_NOT_NULL(result);
  NVS::ByteStreamView def3 = bytestreams.getDefaultValue(ByteStreams::Stream_3);
  TEST_ASSERT_EQUAL_UINT32(def3.size, static_cast<const NVS::ByteStream*>(result)->size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(
    def3.data, static_cast<const NVS::ByteStream*>(result)->data, def3.size);
  TEST_ASSERT_EQUAL_UINT32(def3.size, out.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(def3.data, out.data, out.size);

  // Index out of bounds: returns nullptr
  TEST_ASSERT_NULL(settings[ID_ByteStreams]->getValuePtrOrDefault(
    settings[ID_ByteStreams]->getSize(), &out, sizeof(out)));

  // Buffer too small (max_size = 0): returns nullptr
  NVS::ByteStream tiny{buf, 0};
  TEST_ASSERT_NULL(settings[ID_ByteStreams]->getValuePtrOrDefault(0, &tiny, 0));
}

void test_bytestreams_format() {
  TEST_ASSERT_EQUAL(0, bytestreams.formatAll());

  for (size_t i = 0; i < NVS_VALUES; i++) {
    uint8_t buf[32];
    NVS::ByteStream value{buf, sizeof(buf)};
    TEST_ASSERT(bytestreams.getValue(static_cast<ByteStreams>(i), value));

    TEST_ASSERT_EQUAL_UINT32(new_bytestream[i].size, value.size);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(new_bytestream[i].data, value.data, value.size);
  }
}

void test_bytestreams_forceFormat() {
  TEST_ASSERT_EQUAL(0, bytestreams.formatAll(true));

  for (size_t i = 0; i < NVS_VALUES; i++) {
    uint8_t buf[32];
    NVS::ByteStream value{buf, sizeof(buf)};
    TEST_ASSERT(bytestreams.getValue(static_cast<ByteStreams>(i), value));

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
