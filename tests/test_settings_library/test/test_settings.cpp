#include <Arduino.h>

#define UNITY_INCLUDE_DOUBLE
#include <unity.h>

#include "SettingsManagerESP32.h"

/* ---------------------------------------------------------------------------------------------- */
// Utilizar 3 settings por cada tipo
#define FLAGS(X)             \
  X(FLAG_1, "flag 1", false) \
  X(FLAG_2, "flag 2", true)  \
  X(FLAG_3, "flag 3", false)

#define UINT32S(X)           \
  X(UINT32_1, "uint32 1", 1) \
  X(UINT32_2, "uint32 2", 2) \
  X(UINT32_3, "uint32 3", 3)

#define INT32S(X)           \
  X(INT32_1, "int32 1", -1) \
  X(INT32_2, "int32 2", -2) \
  X(INT32_3, "int32 3", -3)

#define FLOATS(X)            \
  X(FLOAT_1, "float 1", 1.1) \
  X(FLOAT_2, "float 2", 1.2) \
  X(FLOAT_3, "float 3", 1.3)

#define DOUBLES(X)                  \
  X(DOUBLE_1, "double 1", 1.123456) \
  X(DOUBLE_2, "double 2", 2.123456) \
  X(DOUBLE_3, "double 3", 3.123456)

#define STRINGS(X)                 \
  X(STRING_1, "string 1", "str 1") \
  X(STRING_2, "string 2", "str 2") \
  X(STRING_3, "string 3", "str 3")

bool new_bool[3]          = {true, false, true};
uint32_t new_uint32[3]    = {11, 12, 13};
int32_t new_int32[3]      = {-11, -12, -13};
float new_float[3]        = {10.1, 10.2, 10.3};
double new_double[3]      = {11.123456789, 22.123456789, 33.123456789};
const char* new_string[3] = {"hello 1", "hello 2", "hello 3"};

SETTINGS_CREATE_FLAGS(Flags, FLAGS)
SETTINGS_CREATE_UINT32S(UInt32s, UINT32S)
SETTINGS_CREATE_INT32S(Int32s, INT32S)
SETTINGS_CREATE_FLOATS(Floats, FLOATS)
SETTINGS_CREATE_DOUBLES(Doubles, DOUBLES)
SETTINGS_CREATE_STRINGS(Strings, STRINGS)

Settings* settings[] = {
  &Flags_list, &UInt32s_list, &Int32s_list, &Floats_list, &Doubles_list, &Strings_list};
constexpr size_t settings_size = sizeof(settings) / sizeof(settings_size);

enum SettingIndex { ID_FLAGS = 0, ID_UINT32S, ID_INT32S, ID_FLOATS, ID_DOUBLES, ID_STRINGS };
/* ---------------------------------------------------------------------------------------------- */
void test_initializeNVS();

void test_getType();
void test_getSize();

void test_flags_getKey();
void test_flags_getText();
void test_flags_getDefaultValue();
void test_flags_setValue();
void test_flags_getValue();
void test_flags_pointer();
void test_flags_format();

void test_uint32s_getKey();
void test_uint32s_getText();
void test_uint32s_getDefaultValue();
void test_uint32s_setValue();
void test_uint32s_getValue();
void test_uint32s_pointer();
void test_uint32s_format();

void test_int32s_getKey();
void test_int32s_getText();
void test_int32s_getDefaultValue();
void test_int32s_setValue();
void test_int32s_getValue();
void test_int32s_pointer();
void test_int32s_format();

void test_floats_getKey();
void test_floats_getText();
void test_floats_getDefaultValue();
void test_floats_setValue();
void test_floats_getValue();
void test_floats_pointer();
void test_floats_format();

void test_doubles_getKey();
void test_doubles_getText();
void test_doubles_getDefaultValue();
void test_doubles_setValue();
void test_doubles_getValue();
void test_doubles_pointer();
void test_doubles_format();

void test_strings_getKey();
void test_strings_getText();
void test_strings_getDefaultValue();
void test_strings_setValue();
void test_strings_getValue();
void test_strings_pointer();
void test_strings_format();

void setup() {
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_initializeNVS);

  RUN_TEST(test_getType);
  RUN_TEST(test_getSize);

  RUN_TEST(test_flags_getKey);
  RUN_TEST(test_flags_getText);
  RUN_TEST(test_flags_getDefaultValue);
  RUN_TEST(test_flags_setValue);
  RUN_TEST(test_flags_getValue);
  RUN_TEST(test_flags_pointer);
  RUN_TEST(test_flags_format);

  RUN_TEST(test_uint32s_getKey);
  RUN_TEST(test_uint32s_getText);
  RUN_TEST(test_uint32s_getDefaultValue);
  RUN_TEST(test_uint32s_setValue);
  RUN_TEST(test_uint32s_getValue);
  RUN_TEST(test_uint32s_pointer);
  RUN_TEST(test_uint32s_format);

  RUN_TEST(test_int32s_getKey);
  RUN_TEST(test_int32s_getText);
  RUN_TEST(test_int32s_getDefaultValue);
  RUN_TEST(test_int32s_setValue);
  RUN_TEST(test_int32s_getValue);
  RUN_TEST(test_int32s_pointer);
  RUN_TEST(test_int32s_format);

  RUN_TEST(test_floats_getKey);
  RUN_TEST(test_floats_getText);
  RUN_TEST(test_floats_getDefaultValue);
  RUN_TEST(test_floats_setValue);
  RUN_TEST(test_floats_getValue);
  RUN_TEST(test_floats_pointer);
  RUN_TEST(test_floats_format);

  RUN_TEST(test_doubles_getKey);
  RUN_TEST(test_doubles_getText);
  RUN_TEST(test_doubles_getDefaultValue);
  RUN_TEST(test_doubles_setValue);
  RUN_TEST(test_doubles_getValue);
  RUN_TEST(test_doubles_pointer);
  RUN_TEST(test_doubles_format);

  RUN_TEST(test_strings_getKey);
  RUN_TEST(test_strings_getText);
  RUN_TEST(test_strings_getDefaultValue);
  RUN_TEST(test_strings_setValue);
  RUN_TEST(test_strings_getValue);
  RUN_TEST(test_strings_pointer);
  RUN_TEST(test_strings_format);

  UNITY_END();
}

void loop() {}

void test_initializeNVS() { TEST_ASSERT(nvs.begin("esp32")); }

void test_getType() {
  TEST_ASSERT(Flags_list.getType() == SettingsType::FLAG);
  TEST_ASSERT(UInt32s_list.getType() == SettingsType::UINT32);
  TEST_ASSERT(Int32s_list.getType() == SettingsType::INT32);
  TEST_ASSERT(Floats_list.getType() == SettingsType::FLOAT);
  TEST_ASSERT(Doubles_list.getType() == SettingsType::DOUBLE);
  TEST_ASSERT(Strings_list.getType() == SettingsType::STRING);
}

void test_getSize() {
  TEST_ASSERT(Flags_list.getSize() == 3);
  TEST_ASSERT(UInt32s_list.getSize() == 3);
  TEST_ASSERT(Int32s_list.getSize() == 3);
  TEST_ASSERT(Floats_list.getSize() == 3);
  TEST_ASSERT(Doubles_list.getSize() == 3);
  TEST_ASSERT(Strings_list.getSize() == 3);
}

void test_flags_getKey() {
  TEST_ASSERT_EQUAL_STRING("FLAG_1", Flags_list.getKey(Flags::FLAG_1));
  TEST_ASSERT_EQUAL_STRING("FLAG_2", Flags_list.getKey(Flags::FLAG_2));
  TEST_ASSERT_EQUAL_STRING("FLAG_3", Flags_list.getKey(Flags::FLAG_3));
}

void test_flags_getText() {
  TEST_ASSERT_EQUAL_STRING("flag 1", Flags_list.getText(Flags::FLAG_1));
  TEST_ASSERT_EQUAL_STRING("flag 2", Flags_list.getText(Flags::FLAG_2));
  TEST_ASSERT_EQUAL_STRING("flag 3", Flags_list.getText(Flags::FLAG_3));
}

void test_flags_getDefaultValue() {
  TEST_ASSERT_EQUAL(false, Flags_list.getDefaultValue(Flags::FLAG_1));
  TEST_ASSERT_EQUAL(true, Flags_list.getDefaultValue(Flags::FLAG_2));
  TEST_ASSERT_EQUAL(false, Flags_list.getDefaultValue(Flags::FLAG_3));
}

void test_flags_setValue() {
  TEST_ASSERT(Flags_list.setValue(Flags::FLAG_1, new_bool[0]));
  TEST_ASSERT(Flags_list.setValue(Flags::FLAG_2, new_bool[1]));
  TEST_ASSERT(Flags_list.setValue(Flags::FLAG_3, new_bool[2]));
}

void test_flags_getValue() {
  TEST_ASSERT_EQUAL(new_bool[0], Flags_list.getValue(Flags::FLAG_1));
  TEST_ASSERT_EQUAL(new_bool[1], Flags_list.getValue(Flags::FLAG_2));
  TEST_ASSERT_EQUAL(new_bool[2], Flags_list.getValue(Flags::FLAG_3));
}

void test_flags_pointer() {
  TEST_ASSERT_EQUAL(SettingsType::FLAG, settings[ID_FLAGS]->getType());

  for (size_t i = 0; i < settings[ID_FLAGS]->getSize(); i++) {
    char buffer[64];

    // Key
    sprintf(buffer, "FLAG_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_FLAGS]->getKey(i));

    // Text
    sprintf(buffer, "flag %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_FLAGS]->getText(i));

    // Value
    bool value;
    settings[ID_FLAGS]->getValuePtr(i, &value);
    TEST_ASSERT_EQUAL(Flags_list.getValue(static_cast<Flags>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL(Flags_list.getDefaultValue(static_cast<Flags>(i)),
                      settings[ID_FLAGS]->getDefaultValueAs<bool>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_FLAGS]->getSize();

  TEST_ASSERT_NULL(settings[ID_FLAGS]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_FLAGS]->getText(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_FLAGS]->getDefaultValueAs<bool>(idx_out));

  bool value;
  settings[ID_FLAGS]->getValuePtr(idx_out, &value);
  TEST_ASSERT_EQUAL(0, value);
}

void test_flags_format() {
  TEST_ASSERT_EQUAL(0, Flags_list.format());

  for (size_t i = 0; i < 3; i++) {
    TEST_ASSERT_NOT_EQUAL(new_bool[i], Flags_list.getValue(static_cast<Flags>(i)));
  }
}

void test_uint32s_getKey() {
  TEST_ASSERT_EQUAL_STRING("UINT32_1", UInt32s_list.getKey(UInt32s::UINT32_1));
  TEST_ASSERT_EQUAL_STRING("UINT32_2", UInt32s_list.getKey(UInt32s::UINT32_2));
  TEST_ASSERT_EQUAL_STRING("UINT32_3", UInt32s_list.getKey(UInt32s::UINT32_3));
}

void test_uint32s_getText() {
  TEST_ASSERT_EQUAL_STRING("uint32 1", UInt32s_list.getText(UInt32s::UINT32_1));
  TEST_ASSERT_EQUAL_STRING("uint32 2", UInt32s_list.getText(UInt32s::UINT32_2));
  TEST_ASSERT_EQUAL_STRING("uint32 3", UInt32s_list.getText(UInt32s::UINT32_3));
}

void test_uint32s_getDefaultValue() {
  TEST_ASSERT_EQUAL_UINT32(1, UInt32s_list.getDefaultValue(UInt32s::UINT32_1));
  TEST_ASSERT_EQUAL_UINT32(2, UInt32s_list.getDefaultValue(UInt32s::UINT32_2));
  TEST_ASSERT_EQUAL_UINT32(3, UInt32s_list.getDefaultValue(UInt32s::UINT32_3));
}

void test_uint32s_setValue() {
  TEST_ASSERT(UInt32s_list.setValue(UInt32s::UINT32_1, new_uint32[0]));
  TEST_ASSERT(UInt32s_list.setValue(UInt32s::UINT32_2, new_uint32[1]));
  TEST_ASSERT(UInt32s_list.setValue(UInt32s::UINT32_3, new_uint32[2]));
}

void test_uint32s_getValue() {
  TEST_ASSERT_EQUAL_UINT32(new_uint32[0], UInt32s_list.getValue(UInt32s::UINT32_1));
  TEST_ASSERT_EQUAL_UINT32(new_uint32[1], UInt32s_list.getValue(UInt32s::UINT32_2));
  TEST_ASSERT_EQUAL_UINT32(new_uint32[2], UInt32s_list.getValue(UInt32s::UINT32_3));
}

void test_uint32s_pointer() {
  TEST_ASSERT_EQUAL(SettingsType::UINT32, settings[ID_UINT32S]->getType());

  for (size_t i = 0; i < settings[ID_UINT32S]->getSize(); i++) {
    char buffer[64];

    // Key
    sprintf(buffer, "UINT32_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_UINT32S]->getKey(i));

    // Text
    sprintf(buffer, "uint32 %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_UINT32S]->getText(i));

    // Value
    uint32_t value;
    settings[ID_UINT32S]->getValuePtr(i, &value);
    TEST_ASSERT_EQUAL_UINT32(UInt32s_list.getValue(static_cast<UInt32s>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_UINT32(UInt32s_list.getDefaultValue(static_cast<UInt32s>(i)),
                             settings[ID_UINT32S]->getDefaultValueAs<uint32_t>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_UINT32S]->getSize();

  TEST_ASSERT_NULL(settings[ID_UINT32S]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_UINT32S]->getText(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_UINT32S]->getDefaultValueAs<uint32_t>(idx_out));

  uint32_t value;
  settings[ID_UINT32S]->getValuePtr(idx_out, &value);
  TEST_ASSERT_EQUAL(0, value);
}

void test_uint32s_format() {
  TEST_ASSERT_EQUAL(0, UInt32s_list.format());

  for (size_t i = 0; i < 3; i++) {
    TEST_ASSERT_NOT_EQUAL(new_uint32[i], UInt32s_list.getValue(static_cast<UInt32s>(i)));
  }
}

void test_int32s_getKey() {
  TEST_ASSERT_EQUAL_STRING("INT32_1", Int32s_list.getKey(Int32s::INT32_1));
  TEST_ASSERT_EQUAL_STRING("INT32_2", Int32s_list.getKey(Int32s::INT32_2));
  TEST_ASSERT_EQUAL_STRING("INT32_3", Int32s_list.getKey(Int32s::INT32_3));
}

void test_int32s_getText() {
  TEST_ASSERT_EQUAL_STRING("int32 1", Int32s_list.getText(Int32s::INT32_1));
  TEST_ASSERT_EQUAL_STRING("int32 2", Int32s_list.getText(Int32s::INT32_2));
  TEST_ASSERT_EQUAL_STRING("int32 3", Int32s_list.getText(Int32s::INT32_3));
}

void test_int32s_getDefaultValue() {
  TEST_ASSERT_EQUAL_INT32(-1, Int32s_list.getDefaultValue(Int32s::INT32_1));
  TEST_ASSERT_EQUAL_INT32(-2, Int32s_list.getDefaultValue(Int32s::INT32_2));
  TEST_ASSERT_EQUAL_INT32(-3, Int32s_list.getDefaultValue(Int32s::INT32_3));
}

void test_int32s_setValue() {
  TEST_ASSERT(Int32s_list.setValue(Int32s::INT32_1, new_int32[0]));
  TEST_ASSERT(Int32s_list.setValue(Int32s::INT32_2, new_int32[1]));
  TEST_ASSERT(Int32s_list.setValue(Int32s::INT32_3, new_int32[2]));
}

void test_int32s_getValue() {
  TEST_ASSERT_EQUAL_INT32(new_int32[0], Int32s_list.getValue(Int32s::INT32_1));
  TEST_ASSERT_EQUAL_INT32(new_int32[1], Int32s_list.getValue(Int32s::INT32_2));
  TEST_ASSERT_EQUAL_INT32(new_int32[2], Int32s_list.getValue(Int32s::INT32_3));
}

void test_int32s_pointer() {
  TEST_ASSERT_EQUAL(SettingsType::INT32, settings[ID_INT32S]->getType());

  for (size_t i = 0; i < settings[ID_INT32S]->getSize(); i++) {
    char buffer[64];

    // Key
    sprintf(buffer, "INT32_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_INT32S]->getKey(i));

    // Text
    sprintf(buffer, "int32 %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_INT32S]->getText(i));

    // Value
    int32_t value;
    settings[ID_INT32S]->getValuePtr(i, &value);
    TEST_ASSERT_EQUAL_INT32(Int32s_list.getValue(static_cast<Int32s>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_INT32(Int32s_list.getDefaultValue(static_cast<Int32s>(i)),
                            settings[ID_INT32S]->getDefaultValueAs<int32_t>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_INT32S]->getSize();

  TEST_ASSERT_NULL(settings[ID_INT32S]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_INT32S]->getText(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_INT32S]->getDefaultValueAs<int32_t>(idx_out));

  int32_t value;
  settings[ID_INT32S]->getValuePtr(idx_out, &value);
  TEST_ASSERT_EQUAL(0, value);
}

void test_int32s_format() {
  TEST_ASSERT_EQUAL(0, Int32s_list.format());

  for (size_t i = 0; i < 3; i++) {
    TEST_ASSERT_NOT_EQUAL(new_int32[i], Int32s_list.getValue(static_cast<Int32s>(i)));
  }
}

void test_floats_getKey() {
  TEST_ASSERT_EQUAL_STRING("FLOAT_1", Floats_list.getKey(Floats::FLOAT_1));
  TEST_ASSERT_EQUAL_STRING("FLOAT_2", Floats_list.getKey(Floats::FLOAT_2));
  TEST_ASSERT_EQUAL_STRING("FLOAT_3", Floats_list.getKey(Floats::FLOAT_3));
}

void test_floats_getText() {
  TEST_ASSERT_EQUAL_STRING("float 1", Floats_list.getText(Floats::FLOAT_1));
  TEST_ASSERT_EQUAL_STRING("float 2", Floats_list.getText(Floats::FLOAT_2));
  TEST_ASSERT_EQUAL_STRING("float 3", Floats_list.getText(Floats::FLOAT_3));
}

void test_floats_getDefaultValue() {
  TEST_ASSERT_EQUAL_FLOAT(1.1, Floats_list.getDefaultValue(Floats::FLOAT_1));
  TEST_ASSERT_EQUAL_FLOAT(1.2, Floats_list.getDefaultValue(Floats::FLOAT_2));
  TEST_ASSERT_EQUAL_FLOAT(1.3, Floats_list.getDefaultValue(Floats::FLOAT_3));
}

void test_floats_setValue() {
  TEST_ASSERT(Floats_list.setValue(Floats::FLOAT_1, new_float[0]));
  TEST_ASSERT(Floats_list.setValue(Floats::FLOAT_2, new_float[1]));
  TEST_ASSERT(Floats_list.setValue(Floats::FLOAT_3, new_float[2]));
}

void test_floats_getValue() {
  TEST_ASSERT_EQUAL_FLOAT(new_float[0], Floats_list.getValue(Floats::FLOAT_1));
  TEST_ASSERT_EQUAL_FLOAT(new_float[1], Floats_list.getValue(Floats::FLOAT_2));
  TEST_ASSERT_EQUAL_FLOAT(new_float[2], Floats_list.getValue(Floats::FLOAT_3));
}

void test_floats_pointer() {
  TEST_ASSERT_EQUAL(SettingsType::FLOAT, settings[ID_FLOATS]->getType());

  for (size_t i = 0; i < settings[ID_FLOATS]->getSize(); i++) {
    char buffer[64];

    // Key
    sprintf(buffer, "FLOAT_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_FLOATS]->getKey(i));

    // Text
    sprintf(buffer, "float %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_FLOATS]->getText(i));

    // Value
    float value;
    settings[ID_FLOATS]->getValuePtr(i, &value);
    TEST_ASSERT_EQUAL_FLOAT(Floats_list.getValue(static_cast<Floats>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_FLOAT(Floats_list.getDefaultValue(static_cast<Floats>(i)),
                            settings[ID_FLOATS]->getDefaultValueAs<float>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_FLOATS]->getSize();

  TEST_ASSERT_NULL(settings[ID_FLOATS]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_FLOATS]->getText(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_FLOATS]->getDefaultValueAs<float>(idx_out));

  float value;
  settings[ID_FLOATS]->getValuePtr(idx_out, &value);
  TEST_ASSERT_EQUAL(0, value);
}

void test_floats_format() {
  TEST_ASSERT_EQUAL(0, Floats_list.format());

  for (size_t i = 0; i < 3; i++) {
    TEST_ASSERT_NOT_EQUAL(new_float[i], Floats_list.getValue(static_cast<Floats>(i)));
  }
}

void test_doubles_getKey() {
  TEST_ASSERT_EQUAL_STRING("DOUBLE_1", Doubles_list.getKey(Doubles::DOUBLE_1));
  TEST_ASSERT_EQUAL_STRING("DOUBLE_2", Doubles_list.getKey(Doubles::DOUBLE_2));
  TEST_ASSERT_EQUAL_STRING("DOUBLE_3", Doubles_list.getKey(Doubles::DOUBLE_3));
}

void test_doubles_getText() {
  TEST_ASSERT_EQUAL_STRING("double 1", Doubles_list.getText(Doubles::DOUBLE_1));
  TEST_ASSERT_EQUAL_STRING("double 2", Doubles_list.getText(Doubles::DOUBLE_2));
  TEST_ASSERT_EQUAL_STRING("double 3", Doubles_list.getText(Doubles::DOUBLE_3));
}

void test_doubles_getDefaultValue() {
  TEST_ASSERT_EQUAL_DOUBLE(1.123456, Doubles_list.getDefaultValue(Doubles::DOUBLE_1));
  TEST_ASSERT_EQUAL_DOUBLE(2.123456, Doubles_list.getDefaultValue(Doubles::DOUBLE_2));
  TEST_ASSERT_EQUAL_DOUBLE(3.123456, Doubles_list.getDefaultValue(Doubles::DOUBLE_3));
}

void test_doubles_setValue() {
  TEST_ASSERT(Doubles_list.setValue(Doubles::DOUBLE_1, new_double[0]));
  TEST_ASSERT(Doubles_list.setValue(Doubles::DOUBLE_2, new_double[1]));
  TEST_ASSERT(Doubles_list.setValue(Doubles::DOUBLE_3, new_double[2]));
}

void test_doubles_getValue() {
  TEST_ASSERT_EQUAL_DOUBLE(new_double[0], Doubles_list.getValue(Doubles::DOUBLE_1));
  TEST_ASSERT_EQUAL_DOUBLE(new_double[1], Doubles_list.getValue(Doubles::DOUBLE_2));
  TEST_ASSERT_EQUAL_DOUBLE(new_double[2], Doubles_list.getValue(Doubles::DOUBLE_3));
}

void test_doubles_pointer() {
  TEST_ASSERT_EQUAL(SettingsType::DOUBLE, settings[ID_DOUBLES]->getType());

  for (size_t i = 0; i < settings[ID_DOUBLES]->getSize(); i++) {
    char buffer[64];

    // Key
    sprintf(buffer, "DOUBLE_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_DOUBLES]->getKey(i));

    // Text
    sprintf(buffer, "double %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_DOUBLES]->getText(i));

    // Value
    double value;
    settings[ID_DOUBLES]->getValuePtr(i, &value);
    TEST_ASSERT_EQUAL_DOUBLE(Doubles_list.getValue(static_cast<Doubles>(i)), value);

    // Default value
    TEST_ASSERT_EQUAL_DOUBLE(Doubles_list.getDefaultValue(static_cast<Doubles>(i)),
                             settings[ID_DOUBLES]->getDefaultValueAs<double>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_DOUBLES]->getSize();

  TEST_ASSERT_NULL(settings[ID_DOUBLES]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_DOUBLES]->getText(idx_out));
  TEST_ASSERT_EQUAL(0, settings[ID_DOUBLES]->getDefaultValueAs<double>(idx_out));

  double value;
  settings[ID_DOUBLES]->getValuePtr(idx_out, &value);
  TEST_ASSERT_EQUAL(0, value);
}

void test_doubles_format() {
  TEST_ASSERT_EQUAL(0, Doubles_list.format());

  for (size_t i = 0; i < 3; i++) {
    TEST_ASSERT_NOT_EQUAL(new_double[i], Doubles_list.getValue(static_cast<Doubles>(i)));
  }
}

void test_strings_getKey() {
  TEST_ASSERT_EQUAL_STRING("STRING_1", Strings_list.getKey(Strings::STRING_1));
  TEST_ASSERT_EQUAL_STRING("STRING_2", Strings_list.getKey(Strings::STRING_2));
  TEST_ASSERT_EQUAL_STRING("STRING_3", Strings_list.getKey(Strings::STRING_3));
}

void test_strings_getText() {
  TEST_ASSERT_EQUAL_STRING("string 1", Strings_list.getText(Strings::STRING_1));
  TEST_ASSERT_EQUAL_STRING("string 2", Strings_list.getText(Strings::STRING_2));
  TEST_ASSERT_EQUAL_STRING("string 3", Strings_list.getText(Strings::STRING_3));
}

void test_strings_getDefaultValue() {
  TEST_ASSERT_EQUAL_STRING("str 1", Strings_list.getDefaultValue(Strings::STRING_1));
  TEST_ASSERT_EQUAL_STRING("str 2", Strings_list.getDefaultValue(Strings::STRING_2));
  TEST_ASSERT_EQUAL_STRING("str 3", Strings_list.getDefaultValue(Strings::STRING_3));
}

void test_strings_setValue() {
  TEST_ASSERT(Strings_list.setValue(Strings::STRING_1, new_string[0]));
  TEST_ASSERT(Strings_list.setValue(Strings::STRING_2, new_string[1]));
  TEST_ASSERT(Strings_list.setValue(Strings::STRING_3, new_string[2]));
}

void test_strings_getValue() {
  TEST_ASSERT_EQUAL_STRING(new_string[0], Strings_list.getValue(Strings::STRING_1).c_str());
  TEST_ASSERT_EQUAL_STRING(new_string[1], Strings_list.getValue(Strings::STRING_2).c_str());
  TEST_ASSERT_EQUAL_STRING(new_string[2], Strings_list.getValue(Strings::STRING_3).c_str());
}

void test_strings_pointer() {
  TEST_ASSERT_EQUAL(SettingsType::STRING, settings[ID_STRINGS]->getType());

  char buffer[64];

  for (size_t i = 0; i < settings[ID_STRINGS]->getSize(); i++) {
    // Key
    sprintf(buffer, "STRING_%u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_STRINGS]->getKey(i));

    // Text
    sprintf(buffer, "string %u", i + 1);
    TEST_ASSERT_EQUAL_STRING(buffer, settings[ID_STRINGS]->getText(i));

    // Value
    settings[ID_STRINGS]->getValuePtr(i, buffer);
    TEST_ASSERT_EQUAL_STRING(Strings_list.getValue(static_cast<Strings>(i)).c_str(), buffer);

    // Default value
    TEST_ASSERT_EQUAL_STRING(Strings_list.getDefaultValue(static_cast<Strings>(i)),
                             settings[ID_STRINGS]->getDefaultValueAs<const char*>(i));
  }

  // Index out of bounds
  size_t idx_out = settings[ID_STRINGS]->getSize();

  TEST_ASSERT_NULL(settings[ID_STRINGS]->getKey(idx_out));
  TEST_ASSERT_NULL(settings[ID_STRINGS]->getText(idx_out));
  TEST_ASSERT_NULL(settings[ID_STRINGS]->getDefaultValueAs<const char*>(idx_out));

  settings[ID_STRINGS]->getValuePtr(idx_out, buffer);
  TEST_ASSERT_EQUAL_STRING("", buffer);
}

void test_strings_format() {
  TEST_ASSERT_EQUAL(0, Strings_list.format());

  for (size_t i = 0; i < 3; i++) {
    TEST_ASSERT_NOT_EQUAL(new_string[i], Strings_list.getValue(static_cast<Strings>(i)).c_str());
  }
}