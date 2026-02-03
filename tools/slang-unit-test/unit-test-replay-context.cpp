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

// =============================================================================
// Mode Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextIdleMode)
{
    SLANG_UNUSED(unitTestContext);

    // In Idle mode, record operations should be no-ops
    ReplayContext ctx;
    SLANG_CHECK(ctx.getMode() == Mode::Idle);
    SLANG_CHECK(ctx.isIdle());
    SLANG_CHECK(!ctx.isActive());

    // Recording should not write anything
    int32_t value = 42;
    ctx.record(RecordFlag::None, value);
    SLANG_CHECK(ctx.getStream().getSize() == 0);

    // Multiple records should still produce no data
    float f = 3.14f;
    const char* str = "hello";
    ctx.record(RecordFlag::None, f);
    ctx.record(RecordFlag::None, str);
    SLANG_CHECK(ctx.getStream().getSize() == 0);
}

SLANG_UNIT_TEST(replayContextRecordMode)
{
    SLANG_UNUSED(unitTestContext);

    // In Record mode, data should be written to stream
    ReplayContext ctx;
    ctx.setMode(Mode::Record);
    SLANG_CHECK(ctx.getMode() == Mode::Record);
    SLANG_CHECK(ctx.isRecording());
    SLANG_CHECK(ctx.isActive());
    SLANG_CHECK(ctx.isWriting());

    int32_t value = 42;
    ctx.record(RecordFlag::None, value);
    SLANG_CHECK(ctx.getStream().getSize() > 0);

    // Verify data was written correctly by reading it back
    ReplayContext reader(ctx.getStream().getData(), ctx.getStream().getSize());
    int32_t readValue = 0;
    reader.record(RecordFlag::None, readValue);
    SLANG_CHECK(readValue == 42);
}

SLANG_UNIT_TEST(replayContextPlaybackMode)
{
    SLANG_UNUSED(unitTestContext);

    // First record some data
    ReplayContext writer;
    writer.setMode(Mode::Record);

    int32_t writeInt = 123;
    float writeFloat = 2.5f;
    const char* writeStr = "test";

    writer.record(RecordFlag::None, writeInt);
    writer.record(RecordFlag::None, writeFloat);
    writer.record(RecordFlag::None, writeStr);

    // Create playback context and read data
    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());
    SLANG_CHECK(reader.getMode() == Mode::Playback);
    SLANG_CHECK(reader.isPlayback());
    SLANG_CHECK(reader.isActive());
    SLANG_CHECK(reader.isReading());

    int32_t readInt = 0;
    float readFloat = 0.0f;
    const char* readStr = nullptr;

    reader.record(RecordFlag::None, readInt);
    reader.record(RecordFlag::None, readFloat);
    reader.record(RecordFlag::None, readStr);

    SLANG_CHECK(readInt == 123);
    SLANG_CHECK(readFloat == 2.5f);
    SLANG_CHECK(strcmp(readStr, "test") == 0);
}

SLANG_UNIT_TEST(replayContextSyncModeMatching)
{
    SLANG_UNUSED(unitTestContext);

    // First, record reference data
    ReplayContext reference;
    reference.setMode(Mode::Record);

    int32_t val1 = 100;
    int32_t val2 = 200;
    reference.record(RecordFlag::None, val1);
    reference.record(RecordFlag::None, val2);

    // Create sync context with reference data
    ReplayContext sync(reference.getStream().getData(), reference.getStream().getSize(), true);
    SLANG_CHECK(sync.getMode() == Mode::Sync);
    SLANG_CHECK(sync.isSyncing());
    SLANG_CHECK(sync.isActive());
    SLANG_CHECK(sync.isWriting());

    // Record the same values - should succeed
    int32_t syncVal1 = 100;
    int32_t syncVal2 = 200;
    bool noException = true;
    try
    {
        sync.record(RecordFlag::None, syncVal1);
        sync.record(RecordFlag::None, syncVal2);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
}

SLANG_UNIT_TEST(replayContextSyncModeMismatch)
{
    SLANG_UNUSED(unitTestContext);

    // First, record reference data
    ReplayContext reference;
    reference.setMode(Mode::Record);

    int32_t val = 100;
    reference.record(RecordFlag::None, val);

    // Create sync context with reference data
    ReplayContext sync(reference.getStream().getData(), reference.getStream().getSize(), true);

    // Record a different value - should throw
    int32_t differentVal = 999;
    bool caughtException = false;
    try
    {
        sync.record(RecordFlag::None, differentVal);
    }
    catch (const DataMismatchException& e)
    {
        caughtException = true;
        SLANG_CHECK(e.getSize() == sizeof(int32_t));
    }
    SLANG_CHECK(caughtException);
}

SLANG_UNIT_TEST(replayContextPlaybackOutputVerification)
{
    SLANG_UNUSED(unitTestContext);

    // Record data with Output flag
    ReplayContext writer;
    writer.setMode(Mode::Record);

    int32_t inputVal = 42;
    int32_t outputVal = 100;
    writer.record(RecordFlag::Input, inputVal);
    writer.record(RecordFlag::Output, outputVal);

    // Create playback context
    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    // For inputs, we read the value from the stream (user provides 0, gets 42)
    int32_t readInput = 0;
    reader.record(RecordFlag::Input, readInput);
    SLANG_CHECK(readInput == 42);

    // For outputs, user provides the expected value. Playback verifies it matches
    // the recorded value. If they match, no exception is thrown.
    int32_t expectedOutput = 100; // User says "I expect output to be 100"
    bool noException = true;
    try
    {
        reader.record(RecordFlag::Output, expectedOutput);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
    SLANG_CHECK(expectedOutput == 100); // Value unchanged since it matched
}

SLANG_UNIT_TEST(replayContextPlaybackOutputMismatch)
{
    SLANG_UNUSED(unitTestContext);

    // Record an output value
    ReplayContext writer;
    writer.setMode(Mode::Record);

    int32_t outputVal = 100;
    writer.record(RecordFlag::Output, outputVal);

    // Create playback context
    ReplayContext reader(writer.getStream().getData(), writer.getStream().getSize());

    // User provides wrong expected value - should throw
    int32_t wrongExpected = 999; // User says "I expect 999" but recorded was 100
    bool caughtException = false;
    try
    {
        reader.record(RecordFlag::Output, wrongExpected);
    }
    catch (const DataMismatchException& e)
    {
        caughtException = true;
        SLANG_CHECK(e.getSize() == sizeof(int32_t));
    }
    SLANG_CHECK(caughtException);
}

SLANG_UNIT_TEST(replayContextModeTransitions)
{
    SLANG_UNUSED(unitTestContext);

    ReplayContext ctx;
    SLANG_CHECK(ctx.getMode() == Mode::Idle);

    // Test setMode transitions
    ctx.setMode(Mode::Record);
    SLANG_CHECK(ctx.getMode() == Mode::Record);
    SLANG_CHECK(ctx.isRecording());

    ctx.setMode(Mode::Idle);
    SLANG_CHECK(ctx.getMode() == Mode::Idle);
    SLANG_CHECK(ctx.isIdle());

    // Test enable() convenience method
    ctx.enable();
    SLANG_CHECK(ctx.getMode() == Mode::Record);

    // Test disable() convenience method
    ctx.disable();
    SLANG_CHECK(ctx.getMode() == Mode::Idle);

    // enable() should only work from Idle
    ctx.setMode(Mode::Playback);
    ctx.enable(); // Should not change from Playback
    SLANG_CHECK(ctx.getMode() == Mode::Playback);
}

SLANG_UNIT_TEST(replayContextReset)
{
    SLANG_UNUSED(unitTestContext);

    ReplayContext ctx;
    ctx.setMode(Mode::Record);

    int32_t value = 42;
    ctx.record(RecordFlag::None, value);
    SLANG_CHECK(ctx.getStream().getSize() > 0);

    // Reset should clear everything
    ctx.reset();
    SLANG_CHECK(ctx.getMode() == Mode::Idle);
    SLANG_CHECK(ctx.getStream().getSize() == 0);
}

SLANG_UNIT_TEST(replayContextSyncModeWritesToStream)
{
    SLANG_UNUSED(unitTestContext);

    // Record reference data
    ReplayContext reference;
    reference.setMode(Mode::Record);

    int32_t val = 42;
    reference.record(RecordFlag::None, val);

    // Create sync context
    ReplayContext sync(reference.getStream().getData(), reference.getStream().getSize(), true);

    // Sync mode should write to its own stream too
    int32_t syncVal = 42;
    sync.record(RecordFlag::None, syncVal);

    // Verify sync context wrote to its stream
    SLANG_CHECK(sync.getStream().getSize() > 0);

    // The written data should be readable
    ReplayContext reader(sync.getStream().getData(), sync.getStream().getSize());
    int32_t readVal = 0;
    reader.record(RecordFlag::None, readVal);
    SLANG_CHECK(readVal == 42);
}
