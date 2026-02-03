// unit-test-replay-context.cpp
// Unit tests for the ReplayContext serializer - validates round-trip serialization

// Include cpp files directly to access internal symbols not exported from slang DLL
#include "../../source/slang-record-replay/replay-context.cpp"
#include "../../source/slang-record-replay/proxy/proxy-base.cpp"

#include "../../source/slang-record-replay/proxy/proxy-global-session.h"

#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;
using namespace SlangRecord;

// =============================================================================
// Helper: Round-trip test template
// Writes a value, creates a reader, reads it back, and compares
// =============================================================================

template<typename T>
static bool roundTripValue(T writeValue, T& readValue)
{
    ReplayContext writer;
    writer.setMode(Mode::Record);
    writer.record(RecordFlag::None, writeValue);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());
    reader.record(RecordFlag::None, readValue);

    return reader.getStream().atEnd();
}

template<typename T>
static bool roundTripCheck(T value)
{
    T readValue{};
    roundTripValue(value, readValue);
    return readValue == value;
}

// =============================================================================
// Basic Integer Types
// =============================================================================

SLANG_UNIT_TEST(replayContextInt8)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int8_t>(0));
    SLANG_CHECK(roundTripCheck<int8_t>(127));
    SLANG_CHECK(roundTripCheck<int8_t>(-128));
    SLANG_CHECK(roundTripCheck<int8_t>(42));
}

SLANG_UNIT_TEST(replayContextInt16)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int16_t>(0));
    SLANG_CHECK(roundTripCheck<int16_t>(32767));
    SLANG_CHECK(roundTripCheck<int16_t>(-32768));
    SLANG_CHECK(roundTripCheck<int16_t>(12345));
}

SLANG_UNIT_TEST(replayContextInt32)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int32_t>(0));
    SLANG_CHECK(roundTripCheck<int32_t>(2147483647));
    SLANG_CHECK(roundTripCheck<int32_t>(-2147483648));
    SLANG_CHECK(roundTripCheck<int32_t>(123456789));
}

SLANG_UNIT_TEST(replayContextInt64)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<int64_t>(0));
    SLANG_CHECK(roundTripCheck<int64_t>(9223372036854775807LL));
    SLANG_CHECK(roundTripCheck<int64_t>(-9223372036854775807LL - 1));
    SLANG_CHECK(roundTripCheck<int64_t>(1234567890123456789LL));
}

SLANG_UNIT_TEST(replayContextUInt8)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint8_t>(0));
    SLANG_CHECK(roundTripCheck<uint8_t>(255));
    SLANG_CHECK(roundTripCheck<uint8_t>(128));
}

SLANG_UNIT_TEST(replayContextUInt16)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint16_t>(0));
    SLANG_CHECK(roundTripCheck<uint16_t>(65535));
    SLANG_CHECK(roundTripCheck<uint16_t>(32768));
}

SLANG_UNIT_TEST(replayContextUInt32)
{
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<uint32_t>(0));
    SLANG_CHECK(roundTripCheck<uint32_t>(4294967295U));
    SLANG_CHECK(roundTripCheck<uint32_t>(2147483648U));
}

SLANG_UNIT_TEST(replayContextUInt64)
{
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
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<float>(0.0f));
    SLANG_CHECK(roundTripCheck<float>(3.14159f));
    SLANG_CHECK(roundTripCheck<float>(-2.71828f));
    SLANG_CHECK(roundTripCheck<float>(1.0e38f));
    SLANG_CHECK(roundTripCheck<float>(1.0e-38f));
}

SLANG_UNIT_TEST(replayContextDouble)
{
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
    SLANG_UNUSED(unitTestContext);
    SLANG_CHECK(roundTripCheck<bool>(true));
    SLANG_CHECK(roundTripCheck<bool>(false));
}

// =============================================================================
// String Type
// =============================================================================

SLANG_UNIT_TEST(replayContextString)
{
    SLANG_UNUSED(unitTestContext);

    auto testString = [](const char* str)
    {
        ReplayContext writer;
        writer.setMode(Mode::Record);
        writer.record(RecordFlag::None, str);

        ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

        const char* readStr = nullptr;
        reader.record(RecordFlag::None, readStr);

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
// Blob Data
// =============================================================================

SLANG_UNIT_TEST(replayContextBlob)
{
    SLANG_UNUSED(unitTestContext);

    auto testBlob = [](const void* data, size_t size)
    {
        ReplayContext writer;
        writer.setMode(Mode::Record);
        writer.recordBlob(RecordFlag::None, data, size);

        ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

        const void* readData = nullptr;
        size_t readSize = 0;
        reader.recordBlob(RecordFlag::None, readData, readSize);

        if (size != readSize)
            return false;
        if (size == 0)
            return true;
        return memcmp(data, readData, size) == 0;
    };

    uint8_t blobData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0xFE, 0xFF};
    SLANG_CHECK(testBlob(blobData, sizeof(blobData)));
    SLANG_CHECK(testBlob(nullptr, 0));

    // Larger blob
    uint8_t largeBlob[1024];
    for (int i = 0; i < 1024; ++i)
        largeBlob[i] = static_cast<uint8_t>(i & 0xFF);
    SLANG_CHECK(testBlob(largeBlob, sizeof(largeBlob)));
}

// =============================================================================
// Object Handles
// =============================================================================

SLANG_UNIT_TEST(replayContextHandle)
{
    SLANG_UNUSED(unitTestContext);

    auto testHandle = [](uint64_t handle)
    {
        ReplayContext writer;
        writer.setMode(Mode::Record);
        writer.recordHandle(RecordFlag::None, handle);

        ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

        uint64_t readHandle = 0;
        reader.recordHandle(RecordFlag::None, readHandle);

        return handle == readHandle;
    };

    SLANG_CHECK(testHandle(0));
    SLANG_CHECK(testHandle(1));
    SLANG_CHECK(testHandle(0xDEADBEEFCAFEBABEULL));
    SLANG_CHECK(testHandle(UINT64_MAX));
}

// =============================================================================
// Slang Enums
// =============================================================================

SLANG_UNIT_TEST(replayContextSlangEnums)
{
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
    SLANG_UNUSED(unitTestContext);

    SlangUUID writeValue = {0x12345678, 0x1234, 0x5678, {0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78}};
    SlangUUID readValue = {};

    ReplayContext writer;
    writer.setMode(Mode::Record);
    writer.record(RecordFlag::None, writeValue);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());
    reader.record(RecordFlag::None, readValue);

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
    SLANG_UNUSED(unitTestContext);

    slang::CompilerOptionValue writeValue = {};
    writeValue.kind = slang::CompilerOptionValueKind::Int;
    writeValue.intValue0 = 42;
    writeValue.intValue1 = 123;
    writeValue.stringValue0 = "test0";
    writeValue.stringValue1 = "test1";

    ReplayContext writer;
    writer.setMode(Mode::Record);
    writer.record(RecordFlag::None, writeValue);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    slang::CompilerOptionValue readValue = {};
    reader.record(RecordFlag::None, readValue);

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
    SLANG_UNUSED(unitTestContext);

    slang::PreprocessorMacroDesc writeValue = {};
    writeValue.name = "MY_MACRO";
    writeValue.value = "123";

    ReplayContext writer;
    writer.setMode(Mode::Record);
    writer.record(RecordFlag::None, writeValue);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    slang::PreprocessorMacroDesc readValue = {};
    reader.record(RecordFlag::None, readValue);

    SLANG_CHECK(strcmp(writeValue.name, readValue.name) == 0);
    SLANG_CHECK(strcmp(writeValue.value, readValue.value) == 0);
}

// =============================================================================
// TargetDesc
// =============================================================================

SLANG_UNIT_TEST(replayContextTargetDesc)
{
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

    ReplayContext writer;
    writer.setMode(Mode::Record);
    writer.record(RecordFlag::None, writeValue);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    slang::TargetDesc readValue = {};
    reader.record(RecordFlag::None, readValue);

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
    SLANG_UNUSED(unitTestContext);

    // Write multiple values of different types
    ReplayContext writer;
    writer.setMode(Mode::Record);

    int32_t writeInt = 42;
    float writeFloat = 3.14f;
    const char* writeStr = "Hello";
    bool writeBool = true;
    uint64_t writeHandle = 0xDEADBEEF;

    writer.record(RecordFlag::None, writeInt);
    writer.record(RecordFlag::None, writeFloat);
    writer.record(RecordFlag::None, writeStr);
    writer.record(RecordFlag::None, writeBool);
    writer.recordHandle(RecordFlag::None, writeHandle);

    // Read them back
    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    int32_t readInt = 0;
    float readFloat = 0.0f;
    const char* readStr = nullptr;
    bool readBool = false;
    uint64_t readHandle = 0;

    reader.record(RecordFlag::None, readInt);
    reader.record(RecordFlag::None, readFloat);
    reader.record(RecordFlag::None, readStr);
    reader.record(RecordFlag::None, readBool);
    reader.recordHandle(RecordFlag::None, readHandle);

    SLANG_CHECK(writeInt == readInt);
    SLANG_CHECK(writeFloat == readFloat);
    SLANG_CHECK(strcmp(writeStr, readStr) == 0);
    SLANG_CHECK(writeBool == readBool);
    SLANG_CHECK(writeHandle == readHandle);
}

// =============================================================================
// TypeMismatchException
// =============================================================================

SLANG_UNIT_TEST(replayContextTypeMismatch)
{
    SLANG_UNUSED(unitTestContext);

    // Write an int32, try to read a string - should throw
    ReplayContext writer;
    writer.setMode(Mode::Record);
    int32_t writeInt = 42;
    writer.record(RecordFlag::None, writeInt);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    bool caughtException = false;
    try
    {
        const char* readStr = nullptr;
        reader.record(RecordFlag::None, readStr);
    }
    catch (const TypeMismatchException& e)
    {
        caughtException = true;
        SLANG_CHECK(e.getExpected() == TypeId::String || e.getExpected() == TypeId::Null);
        SLANG_CHECK(e.getActual() == TypeId::Int32);
    }
    SLANG_CHECK(caughtException);
}

// =============================================================================
// Stream State
// =============================================================================

SLANG_UNIT_TEST(replayContextStreamState)
{
    SLANG_UNUSED(unitTestContext);

    // Test isReading/isWriting
    ReplayContext writer;
    writer.setMode(Mode::Record);
    SLANG_CHECK(writer.isWriting());
    SLANG_CHECK(!writer.isReading());

    int32_t value = 42;
    writer.record(RecordFlag::None, value);

    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());
    SLANG_CHECK(reader.isReading());
    SLANG_CHECK(!reader.isWriting());
}

// =============================================================================
// Proxy Wrapping Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextSessionWrappedWhenActive)
{
    SLANG_UNUSED(unitTestContext);

    // Save original state using the public C API
    bool wasActive = slang_isRecordLayerEnabled();

    // Enable replay context via the public C API
    slang_enableRecordLayer(true);
    SLANG_CHECK(slang_isRecordLayerEnabled() == true);

    // Create a global session via the public API
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SLANG_CHECK(globalSession != nullptr);

    // The session should be wrapped - verify by checking it's a GlobalSessionProxy
    // and has the correct ref count.
    SLANG_CHECK(dynamic_cast<GlobalSessionProxy*>(globalSession.get()) != nullptr);
    SLANG_CHECK(dynamic_cast<GlobalSessionProxy*>(globalSession.get())->debugGetReferenceCount() == 1);

    // Restore original state
    slang_enableRecordLayer(wasActive);
}

SLANG_UNIT_TEST(replayContextSessionNotWrappedWhenInactive)
{
    SLANG_UNUSED(unitTestContext);

    // Save original state using the public C API
    bool wasActive = slang_isRecordLayerEnabled();

    // Disable replay context via the public C API
    slang_enableRecordLayer(false);
    SLANG_CHECK(slang_isRecordLayerEnabled() == false);

    // Create a global session via the public API
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SLANG_CHECK(globalSession != nullptr);

    // The session should NOT be wrapped
    SLANG_CHECK(dynamic_cast<GlobalSessionProxy*>(globalSession.get()) == nullptr);

    // Restore original state
    slang_enableRecordLayer(wasActive);
}

