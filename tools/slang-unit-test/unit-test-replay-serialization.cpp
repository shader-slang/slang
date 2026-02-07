// unit-test-replay-serialization.cpp
// Unit tests for basic type serialization round-trips

#include "unit-test-replay-common.h"

// =============================================================================
// Basic Integer Types
// =============================================================================

SLANG_UNIT_TEST(replayContextInt8)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int8_t>(0));
    SLANG_CHECK(roundTripCheck<int8_t>(127));
    SLANG_CHECK(roundTripCheck<int8_t>(-128));
    SLANG_CHECK(roundTripCheck<int8_t>(42));
}

SLANG_UNIT_TEST(replayContextInt16)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int16_t>(0));
    SLANG_CHECK(roundTripCheck<int16_t>(32767));
    SLANG_CHECK(roundTripCheck<int16_t>(-32768));
    SLANG_CHECK(roundTripCheck<int16_t>(12345));
}

SLANG_UNIT_TEST(replayContextInt32)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int32_t>(0));
    SLANG_CHECK(roundTripCheck<int32_t>(2147483647));
    SLANG_CHECK(roundTripCheck<int32_t>(-2147483648));
    SLANG_CHECK(roundTripCheck<int32_t>(123456789));
}

SLANG_UNIT_TEST(replayContextInt64)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int64_t>(0));
    SLANG_CHECK(roundTripCheck<int64_t>(9223372036854775807LL));
    SLANG_CHECK(roundTripCheck<int64_t>(-9223372036854775807LL - 1));
    SLANG_CHECK(roundTripCheck<int64_t>(1234567890123456789LL));
}

SLANG_UNIT_TEST(replayContextUInt8)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint8_t>(0));
    SLANG_CHECK(roundTripCheck<uint8_t>(255));
    SLANG_CHECK(roundTripCheck<uint8_t>(128));
}

SLANG_UNIT_TEST(replayContextUInt16)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint16_t>(0));
    SLANG_CHECK(roundTripCheck<uint16_t>(65535));
    SLANG_CHECK(roundTripCheck<uint16_t>(32768));
}

SLANG_UNIT_TEST(replayContextUInt32)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint32_t>(0));
    SLANG_CHECK(roundTripCheck<uint32_t>(4294967295U));
    SLANG_CHECK(roundTripCheck<uint32_t>(2147483648U));
}

SLANG_UNIT_TEST(replayContextUInt64)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint64_t>(0));
    SLANG_CHECK(roundTripCheck<uint64_t>(18446744073709551615ULL));
    SLANG_CHECK(roundTripCheck<uint64_t>(9223372036854775808ULL));
}

// =============================================================================
// Floating-Point Types
// =============================================================================

SLANG_UNIT_TEST(replayContextFloat)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<float>(0.0f));
    SLANG_CHECK(roundTripCheck<float>(3.14159f));
    SLANG_CHECK(roundTripCheck<float>(-2.71828f));
    SLANG_CHECK(roundTripCheck<float>(1.0e38f));
    SLANG_CHECK(roundTripCheck<float>(1.0e-38f));
}

SLANG_UNIT_TEST(replayContextDouble)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<double>(0.0));
    SLANG_CHECK(roundTripCheck<double>(3.141592653589793));
    SLANG_CHECK(roundTripCheck<double>(-2.718281828459045));
    SLANG_CHECK(roundTripCheck<double>(1.0e308));
    SLANG_CHECK(roundTripCheck<double>(1.0e-308));
}

// =============================================================================
// Boolean Type
// =============================================================================

SLANG_UNIT_TEST(replayContextBool)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<bool>(true));
    SLANG_CHECK(roundTripCheck<bool>(false));
}

// =============================================================================
// String Type
// =============================================================================

SLANG_UNIT_TEST(replayContextString)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    auto testString = [](const char* str)
    {
        ctx().reset();
        ctx().setMode(Mode::Record);
        ctx().record(RecordFlag::None, str);

        ctx().switchToPlayback();

        const char* readStr = nullptr;
        ctx().record(RecordFlag::None, readStr);

        if (str == nullptr)
            return readStr == nullptr;
        if (readStr == nullptr)
            return false;
        return strcmp(str, readStr) == 0;
    };

    SLANG_CHECK(testString("Hello, World!"));
    SLANG_CHECK(testString(""));
    SLANG_CHECK(testString(nullptr));
    SLANG_CHECK(testString("A longer string with special chars: \t\n\r"));
    SLANG_CHECK(testString("Unicode test: こんにちは"));
}

// =============================================================================
// Slang Enums
// =============================================================================

SLANG_UNIT_TEST(replayContextSlangEnums)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test a selection of Slang enum types
    {
        SlangCompileTarget value = SLANG_SPIRV;
        SlangCompileTarget readValue = SLANG_TARGET_UNKNOWN;
        roundTripValue(value, readValue);
        SLANG_CHECK(value == readValue);
    }

    {
        SlangStage value = SLANG_STAGE_FRAGMENT;
        SlangStage readValue = SLANG_STAGE_NONE;
        roundTripValue(value, readValue);
        SLANG_CHECK(value == readValue);
    }

    {
        SlangOptimizationLevel value = SLANG_OPTIMIZATION_LEVEL_HIGH;
        SlangOptimizationLevel readValue = SLANG_OPTIMIZATION_LEVEL_NONE;
        roundTripValue(value, readValue);
        SLANG_CHECK(value == readValue);
    }

    {
        SlangDebugInfoLevel value = SLANG_DEBUG_INFO_LEVEL_MAXIMAL;
        SlangDebugInfoLevel readValue = SLANG_DEBUG_INFO_LEVEL_NONE;
        roundTripValue(value, readValue);
        SLANG_CHECK(value == readValue);
    }

    {
        SlangSourceLanguage value = SLANG_SOURCE_LANGUAGE_HLSL;
        SlangSourceLanguage readValue = SLANG_SOURCE_LANGUAGE_UNKNOWN;
        roundTripValue(value, readValue);
        SLANG_CHECK(value == readValue);
    }

    {
        SlangMatrixLayoutMode value = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
        SlangMatrixLayoutMode readValue = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;
        roundTripValue(value, readValue);
        SLANG_CHECK(value == readValue);
    }
}

// =============================================================================
// SlangUUID
// =============================================================================

SLANG_UNIT_TEST(replayContextSlangUUID)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    SlangUUID writeValue = {0x12345678, 0x1234, 0x5678, {0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78}};
    SlangUUID readValue = {};

    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();
    ctx().record(RecordFlag::None, readValue);

    SLANG_CHECK(writeValue.data1 == readValue.data1);
    SLANG_CHECK(writeValue.data2 == readValue.data2);
    SLANG_CHECK(writeValue.data3 == readValue.data3);
    for (int i = 0; i < 8; ++i)
        SLANG_CHECK(writeValue.data4[i] == readValue.data4[i]);
}

// =============================================================================
// CompilerOptionValue
// =============================================================================

SLANG_UNIT_TEST(replayContextCompilerOptionValue)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    slang::CompilerOptionValue writeValue = {};
    writeValue.kind = slang::CompilerOptionValueKind::Int;
    writeValue.intValue0 = 42;
    writeValue.intValue1 = 123;
    writeValue.stringValue0 = "test0";
    writeValue.stringValue1 = "test1";

    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();

    slang::CompilerOptionValue readValue = {};
    ctx().record(RecordFlag::None, readValue);

    SLANG_CHECK(writeValue.kind == readValue.kind);
    SLANG_CHECK(writeValue.intValue0 == readValue.intValue0);
    SLANG_CHECK(writeValue.intValue1 == readValue.intValue1);
    SLANG_CHECK(strcmp(writeValue.stringValue0, readValue.stringValue0) == 0);
    SLANG_CHECK(strcmp(writeValue.stringValue1, readValue.stringValue1) == 0);
}

// =============================================================================
// PreprocessorMacroDesc
// =============================================================================

SLANG_UNIT_TEST(replayContextPreprocessorMacroDesc)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    slang::PreprocessorMacroDesc writeValue = {};
    writeValue.name = "MY_MACRO";
    writeValue.value = "123";

    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();

    slang::PreprocessorMacroDesc readValue = {};
    ctx().record(RecordFlag::None, readValue);

    SLANG_CHECK(strcmp(writeValue.name, readValue.name) == 0);
    SLANG_CHECK(strcmp(writeValue.value, readValue.value) == 0);
}

// =============================================================================
// TargetDesc
// =============================================================================

SLANG_UNIT_TEST(replayContextTargetDesc)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    slang::TargetDesc writeValue = {};
    writeValue.structureSize = sizeof(slang::TargetDesc);
    writeValue.format = SLANG_SPIRV;
    writeValue.profile = SLANG_PROFILE_UNKNOWN;
    writeValue.flags = 0;
    writeValue.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
    writeValue.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
    writeValue.forceGLSLScalarBufferLayout = false;
    writeValue.compilerOptionEntries = nullptr;
    writeValue.compilerOptionEntryCount = 0;

    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();

    slang::TargetDesc readValue = {};
    ctx().record(RecordFlag::None, readValue);

    SLANG_CHECK(writeValue.structureSize == readValue.structureSize);
    SLANG_CHECK(writeValue.format == readValue.format);
    SLANG_CHECK(writeValue.floatingPointMode == readValue.floatingPointMode);
    SLANG_CHECK(writeValue.lineDirectiveMode == readValue.lineDirectiveMode);
    SLANG_CHECK(writeValue.forceGLSLScalarBufferLayout == readValue.forceGLSLScalarBufferLayout);
}

// =============================================================================
// Multiple Values in Sequence
// =============================================================================

SLANG_UNIT_TEST(replayContextMultipleValues)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Write multiple values of different types
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t writeInt = 42;
    float writeFloat = 3.14f;
    const char* writeStr = "Hello";
    bool writeBool = true;
    double writeDouble = 2.71828;

    ctx().record(RecordFlag::None, writeInt);
    ctx().record(RecordFlag::None, writeFloat);
    ctx().record(RecordFlag::None, writeStr);
    ctx().record(RecordFlag::None, writeBool);
    ctx().record(RecordFlag::None, writeDouble);

    // Read them back
    ctx().switchToPlayback();

    int32_t readInt = 0;
    float readFloat = 0.0f;
    const char* readStr = nullptr;
    bool readBool = false;
    double readDouble = 0.0;

    ctx().record(RecordFlag::None, readInt);
    ctx().record(RecordFlag::None, readFloat);
    ctx().record(RecordFlag::None, readStr);
    ctx().record(RecordFlag::None, readBool);
    ctx().record(RecordFlag::None, readDouble);

    SLANG_CHECK(writeInt == readInt);
    SLANG_CHECK(writeFloat == readFloat);
    SLANG_CHECK(strcmp(writeStr, readStr) == 0);
    SLANG_CHECK(writeBool == readBool);
    SLANG_CHECK(writeDouble == readDouble);
}

// =============================================================================
// TypeMismatchException
// =============================================================================

SLANG_UNIT_TEST(replayContextTypeMismatch)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Write an int32, try to read a string - should throw
    ctx().reset();
    ctx().setMode(Mode::Record);
    int32_t writeInt = 42;
    ctx().record(RecordFlag::None, writeInt);

    ctx().switchToPlayback();

    bool caughtException = false;
    try
    {
        const char* readStr = nullptr;
        ctx().record(RecordFlag::None, readStr);
    }
    catch (const TypeMismatchException& e)
    {
        caughtException = true;
        SLANG_CHECK(e.getExpected() == TypeId::String || e.getExpected() == TypeId::Null);
        SLANG_CHECK(e.getActual() == TypeId::Int32);
    }
    SLANG_CHECK(caughtException);
}
