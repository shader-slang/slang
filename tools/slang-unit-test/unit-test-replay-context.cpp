// unit-test-replay-context.cpp
// Unit tests for the ReplayContext serializer - validates round-trip serialization

// Include cpp files directly to access internal symbols not exported from slang DLL
#include "../../source/slang-record-replay/replay-context.h"
#include "../../source/slang-record-replay/proxy/proxy-base.h"
#include "../../source/slang-record-replay/proxy/proxy-global-session.h"

#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;
using namespace SlangRecord;

inline ReplayContext& ctx()
{
    return ReplayContext::get();
}

class ScopedReplayContext
{
public:
    ScopedReplayContext()
    {
        ctx().reset();
    }

    ~ScopedReplayContext()
    {
        ctx().reset();
    }
};

#define REPLAY_TEST \
    if(ReplayContext::get().isActive()) { \
        SLANG_IGNORE_TEST; \
    } \
    ScopedReplayContext _scopedReplayContext;

// =============================================================================
// Helper: Round-trip test template
// Writes a value, creates a reader, reads it back, and compares
// =============================================================================
// quick access to global context

template<typename T>
static bool roundTripValue(T writeValue, T& readValue)
{
    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();
    ctx().record(RecordFlag::None, readValue);

    return ctx().getStream().atEnd();
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
// Blob Data
// =============================================================================

SLANG_UNIT_TEST(replayContextBlob)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    auto testBlob = [](const void* data, size_t size)
    {
        ctx().reset();
        ctx().setMode(Mode::Record);
        ctx().recordBlob(RecordFlag::None, data, size);

        ctx().switchToPlayback();

        const void* readData = nullptr;
        size_t readSize = 0;
        ctx().recordBlob(RecordFlag::None, readData, readSize);

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
// Object Handles (via interface recording)
// =============================================================================

SLANG_UNIT_TEST(replayContextHandle)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // This test emulates a real-world scenario:
    // 1. RECORD: An API call creates a blob and outputs it, then later inputs it
    // 2. PLAYBACK: A new blob is created, and when the 2nd input occurs,
    //    the system correctly identifies it by handle and provides the new blob
    
    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Simulate: API creates a blob and returns it as output (e.g., getCompileResult)
    Slang::ComPtr<ISlangBlob> recordedBlob = Slang::RawBlob::create("original data", 13);
    ISlangBlob* outputBlob = recordedBlob.get();
    ctx().record(RecordFlag::Output, outputBlob);  // Registers blob with handle 1, records handle
    
    // Simulate: Same blob is passed as input to another call (e.g., writeToFile)
    ISlangBlob* inputBlob = recordedBlob.get();
    ctx().record(RecordFlag::Input, inputBlob);  // Looks up handle for blob, records handle
    
    // Verify recording produced data
    SLANG_CHECK(ctx().getStream().getSize() > 0);
    
    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();
    
    // During playback, a NEW blob is created (simulating the real API being called)
    Slang::ComPtr<ISlangBlob> playbackBlob = Slang::RawBlob::create("playback data", 13);
    
    // Playback: First call outputs the blob - we register the new blob with the handle
    ISlangBlob* playbackOutput = playbackBlob.get();
    ctx().record(RecordFlag::Output, playbackOutput);  // Reads handle, verifies/registers
    
    // Playback: Second call inputs the blob - should resolve to our new blob
    ISlangBlob* playbackInput = nullptr;
    ctx().record(RecordFlag::Input, playbackInput);  // Reads handle, looks up object
    
    // The input should resolve to our playback blob (same pointer)
    SLANG_CHECK(playbackInput == playbackBlob.get());
}

SLANG_UNIT_TEST(replayContextHandleMultipleBlobs)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test with multiple blobs to ensure handle tracking works correctly
    
    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Create and output two different blobs
    Slang::ComPtr<ISlangBlob> blob1 = Slang::RawBlob::create("blob one", 8);
    Slang::ComPtr<ISlangBlob> blob2 = Slang::RawBlob::create("blob two", 8);
    
    ISlangBlob* out1 = blob1.get();
    ISlangBlob* out2 = blob2.get();
    ctx().record(RecordFlag::Output, out1);  // Handle 1
    ctx().record(RecordFlag::Output, out2);  // Handle 2
    
    // Now input them in reverse order
    ISlangBlob* in2 = blob2.get();
    ISlangBlob* in1 = blob1.get();
    ctx().record(RecordFlag::Input, in2);
    ctx().record(RecordFlag::Input, in1);
    
    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();
    
    // Create new blobs for playback
    Slang::ComPtr<ISlangBlob> newBlob1 = Slang::RawBlob::create("new one!", 8);
    Slang::ComPtr<ISlangBlob> newBlob2 = Slang::RawBlob::create("new two!", 8);
    
    // Playback outputs
    ISlangBlob* playOut1 = newBlob1.get();
    ISlangBlob* playOut2 = newBlob2.get();
    ctx().record(RecordFlag::Output, playOut1);
    ctx().record(RecordFlag::Output, playOut2);
    
    // Playback inputs (reverse order, matching recording)
    ISlangBlob* playIn2 = nullptr;
    ISlangBlob* playIn1 = nullptr;
    ctx().record(RecordFlag::Input, playIn2);
    ctx().record(RecordFlag::Input, playIn1);
    
    // Verify correct blob resolution
    SLANG_CHECK(playIn1 == newBlob1.get());
    SLANG_CHECK(playIn2 == newBlob2.get());
}

SLANG_UNIT_TEST(replayContextHandleNull)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test that null pointers are handled correctly
    
    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    ISlangBlob* nullBlob = nullptr;
    ctx().record(RecordFlag::Input, nullBlob);
    
    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();
    
    ISlangBlob* readBlob = reinterpret_cast<ISlangBlob*>(0xDEADBEEF);  // Non-null sentinel
    ctx().record(RecordFlag::Input, readBlob);
    
    // Should be null after playback
    SLANG_CHECK(readBlob == nullptr);
}

SLANG_UNIT_TEST(replayContextInlineBlob)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test inline blob serialization - when a user-provided blob (not tracked)
    // is passed as input, its data should be serialized inline and reconstructed
    // during playback.

    const char* testData = "Hello, inline blob world!";
    size_t testDataSize = strlen(testData) + 1;  // Include null terminator

    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Create a user-provided blob that is NOT registered/tracked
    Slang::ComPtr<ISlangBlob> userBlob = Slang::RawBlob::create(testData, testDataSize);
    ISlangBlob* inputBlob = userBlob.get();
    
    // Record it as input - since it's not tracked, it should serialize inline
    ctx().record(RecordFlag::Input, inputBlob);

    // Verify recording produced data
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();

    // During playback, the blob should be reconstructed from the serialized data
    ISlangBlob* readBlob = nullptr;
    ctx().record(RecordFlag::Input, readBlob);

    // Verify the blob was created
    SLANG_CHECK(readBlob != nullptr);
    
    // Verify the data matches
    SLANG_CHECK(readBlob->getBufferSize() == testDataSize);
    SLANG_CHECK(memcmp(readBlob->getBufferPointer(), testData, testDataSize) == 0);

    // Clean up - the blob was detached so we own it
    readBlob->release();
}

SLANG_UNIT_TEST(replayContextInlineBlobThenTracked)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test that an inline blob can later be output (registered) and input again
    // This simulates: user passes blob as input, API stores it internally,
    // then later returns the same blob as output

    const char* testData = "Inline then tracked";
    size_t testDataSize = strlen(testData) + 1;

    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);

    // User provides blob as input (untracked -> inline)
    Slang::ComPtr<ISlangBlob> userBlob = Slang::RawBlob::create(testData, testDataSize);
    ISlangBlob* inputBlob = userBlob.get();
    ctx().record(RecordFlag::Input, inputBlob);

    // API stores it and later returns it as output (now it gets tracked)
    ISlangBlob* outputBlob = userBlob.get();
    ctx().record(RecordFlag::Output, outputBlob);

    // Later, it's passed as input again (should use handle, not inline)
    ISlangBlob* inputAgain = userBlob.get();
    ctx().record(RecordFlag::Input, inputAgain);

    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();

    // First: read inline blob
    ISlangBlob* readInline = nullptr;
    ctx().record(RecordFlag::Input, readInline);
    SLANG_CHECK(readInline != nullptr);
    SLANG_CHECK(readInline->getBufferSize() == testDataSize);

    // Second: output registers a blob (simulating API creating/returning it)
    // In real usage, playback would provide its own blob here
    ISlangBlob* playbackOutput = readInline;  // Use the reconstructed blob
    ctx().record(RecordFlag::Output, playbackOutput);

    // Third: input should resolve to the registered blob
    ISlangBlob* readAgain = nullptr;
    ctx().record(RecordFlag::Input, readAgain);
    SLANG_CHECK(readAgain == readInline);

    // Clean up
    readInline->release();
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

// =============================================================================
// Stream State
// =============================================================================

SLANG_UNIT_TEST(replayContextStreamState)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test isReading/isWriting
    ctx().reset();
    ctx().setMode(Mode::Record);
    SLANG_CHECK(ctx().isWriting());
    SLANG_CHECK(!ctx().isReading());

    int32_t value = 42;
    ctx().record(RecordFlag::None, value);

    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isReading());
    SLANG_CHECK(!ctx().isWriting());
}

// =============================================================================
// Proxy Wrapping Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextSessionWrappedWhenActive)
{
    REPLAY_TEST;
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
    REPLAY_TEST;
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
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // In Idle mode, record operations should be no-ops
    ctx().reset();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(ctx().isIdle());
    SLANG_CHECK(!ctx().isActive());

    // Recording should not write anything
    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    SLANG_CHECK(ctx().getStream().getSize() == 0);

    // Multiple records should still produce no data
    float f = 3.14f;
    const char* str = "hello";
    ctx().record(RecordFlag::None, f);
    ctx().record(RecordFlag::None, str);
    SLANG_CHECK(ctx().getStream().getSize() == 0);
}

SLANG_UNIT_TEST(replayContextRecordMode)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // In Record mode, data should be written to stream
    ctx().reset();
    ctx().setMode(Mode::Record);
    SLANG_CHECK(ctx().getMode() == Mode::Record);
    SLANG_CHECK(ctx().isRecording());
    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().isWriting());

    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // Verify data was written correctly by reading it back
    ctx().switchToPlayback();
    int32_t readValue = 0;
    ctx().record(RecordFlag::None, readValue);
    SLANG_CHECK(readValue == 42);
}

SLANG_UNIT_TEST(replayContextPlaybackMode)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // First record some data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t writeInt = 123;
    float writeFloat = 2.5f;
    const char* writeStr = "test";

    ctx().record(RecordFlag::None, writeInt);
    ctx().record(RecordFlag::None, writeFloat);
    ctx().record(RecordFlag::None, writeStr);

    // Switch to playback and read data
    ctx().switchToPlayback();
    SLANG_CHECK(ctx().getMode() == Mode::Playback);
    SLANG_CHECK(ctx().isPlayback());
    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().isReading());

    int32_t readInt = 0;
    float readFloat = 0.0f;
    const char* readStr = nullptr;

    ctx().record(RecordFlag::None, readInt);
    ctx().record(RecordFlag::None, readFloat);
    ctx().record(RecordFlag::None, readStr);

    SLANG_CHECK(readInt == 123);
    SLANG_CHECK(readFloat == 2.5f);
    SLANG_CHECK(strcmp(readStr, "test") == 0);
}

SLANG_UNIT_TEST(replayContextSyncModeMatching)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // First, record reference data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t val1 = 100;
    int32_t val2 = 200;
    ctx().record(RecordFlag::None, val1);
    ctx().record(RecordFlag::None, val2);

    // Switch to sync mode - reset position and set mode
    ctx().switchToSync();
    SLANG_CHECK(ctx().getMode() == Mode::Sync);
    SLANG_CHECK(ctx().isSyncing());
    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().isWriting());

    // Record the same values - should succeed
    int32_t syncVal1 = 100;
    int32_t syncVal2 = 200;
    bool noException = true;
    try
    {
        ctx().record(RecordFlag::None, syncVal1);
        ctx().record(RecordFlag::None, syncVal2);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
}

SLANG_UNIT_TEST(replayContextSyncModeMismatch)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // First, record reference data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t val = 100;
    ctx().record(RecordFlag::None, val);

    // Switch to sync mode
    ctx().switchToSync();

    // Record a different value - should throw
    int32_t differentVal = 999;
    bool caughtException = false;
    try
    {
        ctx().record(RecordFlag::None, differentVal);
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
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Record data with Output flag
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t inputVal = 42;
    int32_t outputVal = 100;
    ctx().record(RecordFlag::Input, inputVal);
    ctx().record(RecordFlag::Output, outputVal);

    // Switch to playback
    ctx().switchToPlayback();

    // For inputs, we read the value from the stream (user provides 0, gets 42)
    int32_t readInput = 0;
    ctx().record(RecordFlag::Input, readInput);
    SLANG_CHECK(readInput == 42);

    // For outputs, user provides the expected value. Playback verifies it matches
    // the recorded value. If they match, no exception is thrown.
    int32_t expectedOutput = 100; // User says "I expect output to be 100"
    bool noException = true;
    try
    {
        ctx().record(RecordFlag::Output, expectedOutput);
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
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Record an output value
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t outputVal = 100;
    ctx().record(RecordFlag::Output, outputVal);

    // Switch to playback
    ctx().switchToPlayback();

    // User provides wrong expected value - should throw
    int32_t wrongExpected = 999; // User says "I expect 999" but recorded was 100
    bool caughtException = false;
    try
    {
        ctx().record(RecordFlag::Output, wrongExpected);
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
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().reset();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);

    // Test setMode transitions
    ctx().setMode(Mode::Record);
    SLANG_CHECK(ctx().getMode() == Mode::Record);
    SLANG_CHECK(ctx().isRecording());

    ctx().setMode(Mode::Idle);
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(ctx().isIdle());

    // Test enable() convenience method
    ctx().enable();
    SLANG_CHECK(ctx().getMode() == Mode::Record);

    // Test disable() convenience method
    ctx().disable();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);

    // enable() should only work from Idle
    ctx().setMode(Mode::Playback);
    ctx().enable(); // Should not change from Playback
    SLANG_CHECK(ctx().getMode() == Mode::Playback);
}

SLANG_UNIT_TEST(replayContextReset)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // Reset should clear everything
    ctx().reset();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(ctx().getStream().getSize() == 0);
}

SLANG_UNIT_TEST(replayContextSyncModeWritesToStream)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Record reference data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t val = 42;
    ctx().record(RecordFlag::None, val);

    // Switch to sync mode
    ctx().switchToSync();

    // Sync mode should write to its own stream too
    int32_t syncVal = 42;
    ctx().record(RecordFlag::None, syncVal);

    // Verify sync context wrote to its stream
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // The written data should be readable
    ctx().switchToPlayback();
    int32_t readVal = 0;
    ctx().record(RecordFlag::None, readVal);
    SLANG_CHECK(readVal == 42);
}

// =============================================================================
// Integration Test: Record actual API calls and verify exact bytes
// =============================================================================

// Helper to read a TypeId byte from the stream
static TypeId readTypeIdFromStream(ReplayStream& stream)
{
    uint8_t byte = 0;
    stream.read(&byte, 1);
    return static_cast<TypeId>(byte);
}

SLANG_UNIT_TEST(replayContextRecordFindProfileCall)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Start recording
    ctx().setMode(Mode::Record);

    // Record the creation of a global session and calling findProfile
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SlangProfileID profileId = globalSession->findProfile("sm_5_0");
    SLANG_CHECK(profileId != SLANG_PROFILE_UNKNOWN);

    // Get the recorded data from DLL's context + switch to playback.
    const void* data = ctx().getStream().getData();
    size_t size = ctx().getStream().getSize();
    SLANG_CHECK(data != nullptr);
    SLANG_CHECK(size > 0);
    ctx().switchToPlayback();

    // Read / verify the creation of the global session. This is:
    // - signature (slang_createGlobalSession2)
    // - this handle (null)
    // - input descriptor
    // - output handle to new global context
    // - output success result code
    const char* signature = nullptr;
    ctx().record(RecordFlag::Input, signature);
    SLANG_CHECK(signature != nullptr);
    SLANG_CHECK(strcmp(signature, "slang_createGlobalSession2") == 0);
    uint64_t thisHandle = 0;
    ctx().recordHandle(RecordFlag::Input, thisHandle);
    SLANG_CHECK(thisHandle == kNullHandle);
    SlangGlobalSessionDesc globalDesc = {};
    ctx().record(RecordFlag::Input, globalDesc);
    SLANG_CHECK(globalDesc.apiVersion == 0);
    SLANG_CHECK(readTypeIdFromStream(ctx().getStream()) == TypeId::ObjectHandle);
    uint64_t globalContextHandle = 0;
    ctx().getStream().read(&globalContextHandle, sizeof(globalContextHandle));
    SLANG_CHECK(globalContextHandle == kFirstValidHandle);
    SlangResult globalContextResult;
    ctx().record(RecordFlag::None, globalContextResult);

    // Read / verify the findProfile call. This is:
    // - signature: GlobalSessionProxy::findProfile
    // - this handle (the handle of the global session)
    // - input profile name ("sm_5_0")
    // - output profileId
    ctx().record(RecordFlag::Input, signature);
    SLANG_CHECK(strcmp(signature, "GlobalSessionProxy::findProfile") == 0);
    ctx().recordHandle(RecordFlag::Input, thisHandle);
     SLANG_CHECK(thisHandle >= kFirstValidHandle); // Should be a valid handle
    const char* profileName = nullptr;
    ctx().record(RecordFlag::Input, profileName);
    SLANG_CHECK(profileName != nullptr);
    SLANG_CHECK(strcmp(profileName, "sm_5_0") == 0);
    int32_t returnedProfileId = 0;
    ctx().record(RecordFlag::None, returnedProfileId);
    SLANG_CHECK(returnedProfileId == static_cast<int32_t>(profileId));

    // Should have consumed all data
    SLANG_CHECK(ctx().getStream().atEnd());

    // Clean up
    globalSession = nullptr;
}

SLANG_UNIT_TEST(replayContextRecordCreateSessionCall)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Start recording
    ctx().setMode(Mode::Record);

    // Create a global session
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    // Create a session
    slang::SessionDesc sessionDesc = {};
    Slang::ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    // Get the recorded data from DLL's context + switch to playback.
    const void* data = ctx().getStream().getData();
    size_t size = ctx().getStream().getSize();
    SLANG_CHECK(data != nullptr);
    SLANG_CHECK(size > 0);
    ctx().switchToPlayback();

    // Read / verify the creation of the global session. This is:
    // - signature (slang_createGlobalSession2)
    // - this handle (NULL)
    // - input descriptor
    // - output handle to new global context
    // - output success result code
    const char* signature = nullptr;
    ctx().record(RecordFlag::Input, signature);
    SLANG_CHECK(signature != nullptr);
    SLANG_CHECK(strcmp(signature, "slang_createGlobalSession2") == 0);
    uint64_t thisHandle = 0;
    ctx().recordHandle(RecordFlag::Input, thisHandle);
    SLANG_CHECK(thisHandle == kNullHandle);
    SlangGlobalSessionDesc readGlobalDesc = {};
    ctx().record(RecordFlag::Input, readGlobalDesc);
    SLANG_CHECK(readGlobalDesc.apiVersion == 0);
    SLANG_CHECK(readTypeIdFromStream(ctx().getStream()) == TypeId::ObjectHandle);
    uint64_t globalContextHandle = 0;
    ctx().getStream().read(&globalContextHandle, sizeof(globalContextHandle));
    SLANG_CHECK(globalContextHandle == kFirstValidHandle);
    SlangResult globalContextResult;
    ctx().record(RecordFlag::None, globalContextResult);

    // Read / verify the createSession call. This is:
    // - signature: GlobalSessionProxy::createSession
    // - this handle (the handle of the global session)
    // - SessionDesc (complex struct)
    // - output session handle
    // - return value (SlangResult)
    ctx().record(RecordFlag::Input, signature);
    SLANG_CHECK(signature != nullptr);
    SLANG_CHECK(strstr(signature, "createSession") != nullptr);
    ctx().recordHandle(RecordFlag::Input, thisHandle);
    SLANG_CHECK(thisHandle == kFirstValidHandle);
    SLANG_CHECK(thisHandle == globalContextHandle); // Should be the global session
    slang::SessionDesc readDesc = {};
    ctx().record(RecordFlag::Input, readDesc);
    SLANG_CHECK(readDesc.targetCount == 0);
    SLANG_CHECK(readDesc.searchPathCount == 0);
    SLANG_CHECK(readTypeIdFromStream(ctx().getStream()) == TypeId::ObjectHandle);
    uint64_t sessionHandle = 0;
    ctx().getStream().read(&sessionHandle, sizeof(sessionHandle));
    SLANG_CHECK(sessionHandle >= kFirstValidHandle);
    SLANG_CHECK(sessionHandle != globalContextHandle); // Different object from global session
    SlangResult result = 0;
    ctx().record(RecordFlag::None, result);
    SLANG_CHECK(result == SLANG_OK);

    // Should have consumed all data
    SLANG_CHECK(ctx().getStream().atEnd());

    // Clean up
    session = nullptr;
    globalSession = nullptr;
}

// =============================================================================
// Playback Dispatcher Tests
// =============================================================================

// Track calls made during playback
static int s_playbackCallCount = 0;
static const char* s_lastProfileName = nullptr;

// Handler for findProfile playback
static void playbackFindProfile(ReplayContext& ctx)
{
    s_playbackCallCount++;

    // executeNextCall seeks back to the start of the command, so we must
    // first consume the signature and 'this' handle (just like RECORD_CALL does)
    const char* signature = nullptr;
    ctx.record(RecordFlag::Input, signature);
    
    ISlangBlob* thisPtr = nullptr;
    ctx.record(RecordFlag::Input, thisPtr);

    // Now read the profile name input
    const char* profileName = nullptr;
    ctx.record(RecordFlag::Input, profileName);
    s_lastProfileName = profileName;

    // In a real playback, we'd call the actual function here:
    // auto* globalSession = ctx.getCurrentThis<slang::IGlobalSession>();
    // SlangProfileID result = globalSession->findProfile(profileName);

    // For testing, just read and discard the return value
    int32_t returnValue = 0;
    ctx.record(RecordFlag::None, returnValue);
}

SLANG_UNIT_TEST(replayContextPlaybackDispatcher)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Reset test state
    s_playbackCallCount = 0;
    s_lastProfileName = nullptr;

    // First, record a call - we'll manually construct the stream format
    // Format: signature (string), thisHandle (ObjectHandle), input args..., return value
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Record signature
    const char* signature = "findProfile_test_signature";
    ctx().record(RecordFlag::Input, signature);

    // Record 'this' pointer as a handle - use a blob as a tracked object since we can record those
    // Actually, let's just write the handle bytes directly since recorder is in Record mode
    // We need to register an object first, then record its handle
    
    // Create a fake blob to use as "this"
    Slang::ComPtr<ISlangBlob> fakeBlob = Slang::RawBlob::create("fake", 4);
    uint64_t thisHandle = ctx().registerInterface(fakeBlob.get());
    SLANG_UNUSED(thisHandle);
    
    // Write the handle directly (ObjectHandle TypeId + handle value)
    ISlangBlob* blobPtr = fakeBlob.get();
    ctx().record(RecordFlag::Input, blobPtr);

    // Record the profile name input
    const char* profileName = "sm_6_0";
    ctx().record(RecordFlag::Input, profileName);

    // Record return value
    int32_t profileId = 42;
    ctx().record(RecordFlag::ReturnValue, profileId);

    // Now set up playback
    ctx().switchToPlayback();
    
    // Register the handler
    ctx().registerHandler("findProfile_test_signature", playbackFindProfile);

    // Also need to register the fake object so getCurrentThis works
    // Use the same handle value for consistency
    ctx().registerInterface(fakeBlob.get());

    // Execute the call
    bool executed = ctx().executeNextCall();
    SLANG_CHECK(executed);
    SLANG_CHECK(s_playbackCallCount == 1);
    SLANG_CHECK(s_lastProfileName != nullptr);
    SLANG_CHECK(strcmp(s_lastProfileName, "sm_6_0") == 0);

    // No more calls
    SLANG_CHECK(!ctx().hasMoreCalls());
    SLANG_CHECK(!ctx().executeNextCall());
}

SLANG_UNIT_TEST(replayContextPlaybackMultipleCalls)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    s_playbackCallCount = 0;

    // Record multiple calls
    ctx().reset();
    ctx().setMode(Mode::Record);

    Slang::ComPtr<ISlangBlob> fakeBlob = Slang::RawBlob::create("fake", 4);
    ctx().registerInterface(fakeBlob.get());

    for (int i = 0; i < 3; i++)
    {
        const char* sig = "findProfile_test_signature";
        ctx().record(RecordFlag::Input, sig);
        
        ISlangBlob* blobPtr = fakeBlob.get();
        ctx().record(RecordFlag::Input, blobPtr);
        
        const char* name = "sm_5_0";
        ctx().record(RecordFlag::Input, name);
        
        int32_t result = 10 + i;
        ctx().record(RecordFlag::ReturnValue, result);
    }

    // Playback all
    ctx().switchToPlayback();
    ctx().registerHandler("findProfile_test_signature", playbackFindProfile);
    ctx().registerInterface(fakeBlob.get());

    ctx().executeAll();

    SLANG_CHECK(s_playbackCallCount == 3);
    SLANG_CHECK(!ctx().hasMoreCalls());
}

// =============================================================================
// Test REPLAY_REGISTER macro - using a simple test proxy
// =============================================================================

// Simple test interface for replay macro testing
struct ITestCalculator : public ISlangUnknown
{
    SLANG_COM_INTERFACE(0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0});
    
    virtual int32_t SLANG_MCALL add(int32_t a, int32_t b) = 0;
    virtual void SLANG_MCALL setOffset(int32_t offset) = 0;
};

// Track what gets called during playback
static int32_t s_testCalcLastA = 0;
static int32_t s_testCalcLastB = 0;
static int32_t s_testCalcOffset = 0;
static int s_testCalcAddCalls = 0;
static int s_testCalcSetOffsetCalls = 0;

// Simple proxy for ITestCalculator that uses our recording macros
class TestCalculatorProxy : public ITestCalculator
{
public:
    TestCalculatorProxy(ITestCalculator* actual) : m_actual(actual), m_refCount(1) {}

    // ISlangUnknown
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == ITestCalculator::getTypeGuid() || uuid == ISlangUnknown::getTypeGuid())
        {
            *outObject = this;
            addRef();
            return SLANG_OK;
        }
        *outObject = nullptr;
        return SLANG_E_NO_INTERFACE;
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ++m_refCount; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override 
    { 
        uint32_t count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }

    // ITestCalculator - with recording
    int32_t SLANG_MCALL add(int32_t a, int32_t b) override
    {
        RECORD_CALL();
        RECORD_INPUT(a);
        RECORD_INPUT(b);

        // Track for test verification
        s_testCalcLastA = a;
        s_testCalcLastB = b;
        s_testCalcAddCalls++;

        int32_t result = m_actual ? m_actual->add(a, b) : (a + b);
        RECORD_RETURN(result);
    }

    void SLANG_MCALL setOffset(int32_t offset) override
    {
        RECORD_CALL();
        RECORD_INPUT(offset);

        s_testCalcOffset = offset;
        s_testCalcSetOffsetCalls++;

        if (m_actual) m_actual->setOffset(offset);
    }

    ITestCalculator* getActual() { return m_actual; }

private:
    ITestCalculator* m_actual;
    std::atomic<uint32_t> m_refCount;
};

// Simple implementation that just does the math
class TestCalculatorImpl : public ITestCalculator
{
public:
    TestCalculatorImpl() : m_offset(0), m_refCount(1) {}

    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == ITestCalculator::getTypeGuid() || uuid == ISlangUnknown::getTypeGuid())
        {
            *outObject = this;
            addRef();
            return SLANG_OK;
        }
        *outObject = nullptr;
        return SLANG_E_NO_INTERFACE;
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ++m_refCount; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override 
    { 
        uint32_t count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }

    int32_t SLANG_MCALL add(int32_t a, int32_t b) override { return a + b + m_offset; }
    void SLANG_MCALL setOffset(int32_t offset) override { m_offset = offset; }

private:
    int32_t m_offset;
    std::atomic<uint32_t> m_refCount;
};

// Test the REPLAY_REGISTER infrastructure by using the replayHandler template directly
// with a known signature
SLANG_UNIT_TEST(replayContextReplayRegisterMacro)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Reset test state
    s_testCalcLastA = 0;
    s_testCalcLastB = 0;
    s_testCalcAddCalls = 0;
    s_testCalcSetOffsetCalls = 0;
    s_testCalcOffset = 0;

    // Create implementation and proxy
    Slang::ComPtr<ITestCalculator> impl(new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    Slang::ComPtr<ITestCalculator> proxyPtr(proxy);

    // Build a recorded stream manually with known signatures
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Register the proxy and get its handle
    uint64_t proxyHandle = ctx().registerInterface(proxyPtr.get());
    SLANG_CHECK(proxyHandle >= kFirstValidHandle);

    // Record a call manually with a simple signature we control
    const char* addSignature = "TestCalculator::add";
    ctx().record(RecordFlag::Input, addSignature);  // signature
    
    // Record 'this' handle with proper TypeId (what beginCall does via recordHandle)
    ctx().recordHandle(RecordFlag::Input, proxyHandle);
    
    int32_t arg_a = 10;
    int32_t arg_b = 20;
    ctx().record(RecordFlag::Input, arg_a);
    ctx().record(RecordFlag::Input, arg_b);
    
    int32_t returnVal = 30;
    ctx().record(RecordFlag::ReturnValue, returnVal);

    // Verify we recorded something
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(proxyPtr.get());  // Same handle value
    
    // Register a handler using the replayHandler template (what REPLAY_REGISTER does internally)
    auto addHandler = [](ReplayContext& ctxRef) {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::add
        );
    };
    ctx().registerHandler(addSignature, addHandler);

    // Execute playback - this should:
    // 1. Read signature "TestCalculator::add"
    // 2. Read thisHandle and set m_currentThisHandle
    // 3. Call addHandler which calls replayHandler
    // 4. replayHandler gets 'this' via getCurrentThis and calls proxy->add(default, default)
    // 5. Proxy's add method uses RECORD_* macros which read from stream in Playback mode
    
    // But wait - the proxy's RECORD_CALL uses ctx() singleton, not 'player'
    // We need to test differently - verify the template infrastructure compiles and works
    
    // For this test, just verify the handler dispatch works
    bool executed = ctx().executeNextCall();
    SLANG_CHECK(executed);
    
    // In this test, the proxy's add() was called with default args (0, 0)
    // because we're testing the dispatch, not full bidirectional record/replay
    SLANG_CHECK(s_testCalcAddCalls == 1);
    
    // No more calls
    SLANG_CHECK(!ctx().hasMoreCalls());
}

// Test the MemberFunctionTraits template
SLANG_UNIT_TEST(replayContextMemberFunctionTraits)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    
    // Test arity detection
    using AddTraits = MemberFunctionTraits<decltype(&TestCalculatorProxy::add)>;
    static_assert(AddTraits::Arity == 2, "add should have 2 args");
    static_assert(std::is_same_v<AddTraits::ReturnType, int32_t>, "add returns int32_t");
    
    using SetOffsetTraits = MemberFunctionTraits<decltype(&TestCalculatorProxy::setOffset)>;
    static_assert(SetOffsetTraits::Arity == 1, "setOffset should have 1 arg");
    static_assert(std::is_void_v<SetOffsetTraits::ReturnType>, "setOffset returns void");
    
    // Test DefaultValue
    int32_t defInt = DefaultValue<int32_t>::get();
    SLANG_CHECK(defInt == 0);
    
    int32_t* defPtr = DefaultValue<int32_t*>::get();
    SLANG_CHECK(defPtr == nullptr);
    
    // All checks passed
    SLANG_CHECK(true);
}

// =============================================================================
// Test full round-trip: record through proxy, playback through proxy
// =============================================================================

// Test that recording via RECORD_CALL and playback via REPLAY_REGISTER work together
// This validates that parseSignature produces matching signatures in both directions
SLANG_UNIT_TEST(replayContextFullRoundTrip)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Reset test state
    s_testCalcLastA = 0;
    s_testCalcLastB = 0;
    s_testCalcAddCalls = 0;
    s_testCalcSetOffsetCalls = 0;
    s_testCalcOffset = 0;

    // Create implementation and proxy
    Slang::ComPtr<ITestCalculator> impl(new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    Slang::ComPtr<ITestCalculator> proxyPtr(proxy);

    // ========== RECORDING PHASE ==========
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Register the proxy (simulates what happens during createSession)
    uint64_t proxyHandle = ctx().registerInterface(proxyPtr.get());
    SLANG_CHECK(proxyHandle >= kFirstValidHandle);

    // Call methods through proxy - this uses RECORD_CALL() which normalizes the signature
    int32_t result1 = proxy->add(10, 20);
    SLANG_CHECK(result1 == 30);  // Implementation adds the values
    SLANG_CHECK(s_testCalcAddCalls == 1);
    SLANG_CHECK(s_testCalcLastA == 10);
    SLANG_CHECK(s_testCalcLastB == 20);

    proxy->setOffset(5);
    SLANG_CHECK(s_testCalcSetOffsetCalls == 1);
    SLANG_CHECK(s_testCalcOffset == 5);

    int32_t result2 = proxy->add(100, 200);
    SLANG_CHECK(result2 == 305);  // 100 + 200 + 5 (offset)
    SLANG_CHECK(s_testCalcAddCalls == 2);

    // Verify we recorded something
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // ========== PLAYBACK PHASE ==========
    // Reset call tracking
    s_testCalcLastA = 0;
    s_testCalcLastB = 0;
    s_testCalcAddCalls = 0;
    s_testCalcSetOffsetCalls = 0;
    s_testCalcOffset = 0;

    // Create new implementation and proxy for playback
    Slang::ComPtr<ITestCalculator> impl2(new TestCalculatorImpl());
    TestCalculatorProxy* proxy2 = new TestCalculatorProxy(impl2.get());
    Slang::ComPtr<ITestCalculator> proxyPtr2(proxy2);

    // Switch to playback mode
    ctx().switchToPlayback();
    
    // Re-register with same handle - during real playback, this happens
    // when the creation methods are replayed
    ctx().registerInterface(proxyPtr2.get());

    // Register handlers - this is what REPLAY_REGISTER does
    // We need to use the signature that parseSignature produces from __FUNCSIG__
    // For TestCalculatorProxy::add, parseSignature extracts "TestCalculatorProxy::add"
    auto addHandler = [](ReplayContext& ctxRef) {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::add
        );
    };
    auto setOffsetHandler = [](ReplayContext& ctxRef) {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::setOffset
        );
    };
    
    // Use the exact signature that parseSignature will produce
    ctx().registerHandler("TestCalculatorProxy::add", addHandler);
    ctx().registerHandler("TestCalculatorProxy::setOffset", setOffsetHandler);

    // Execute all recorded calls
    ctx().executeAll();

    // Verify the calls were replayed
    // Note: the values should match what was recorded
    SLANG_CHECK(s_testCalcAddCalls == 2);
    SLANG_CHECK(s_testCalcSetOffsetCalls == 1);

    // The last recorded call was add(100, 200)
    SLANG_CHECK(s_testCalcLastA == 100);
    SLANG_CHECK(s_testCalcLastB == 200);
    SLANG_CHECK(s_testCalcOffset == 5);

    // No more calls
    SLANG_CHECK(!ctx().hasMoreCalls());
}

// Test parseSignature with various signature formats
SLANG_UNIT_TEST(replayContextParseSignature)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    char buffer[256];

    // Test MSVC-style __FUNCSIG__
    {
        const char* msvcSig = "SlangResult __cdecl SlangRecord::GlobalSessionProxy::createSession(struct slang::SessionDesc const &,struct slang::ISession **)";
        const char* result = ReplayContext::parseSignature(msvcSig, buffer, sizeof(buffer));
        SLANG_CHECK(strcmp(result, "GlobalSessionProxy::createSession") == 0);
    }

    // Test with void return type
    {
        const char* voidSig = "void __cdecl SlangRecord::SessionProxy::addSearchPath(char const *)";
        const char* result = ReplayContext::parseSignature(voidSig, buffer, sizeof(buffer));
        SLANG_CHECK(strcmp(result, "SessionProxy::addSearchPath") == 0);
    }

    // Test with virtual and other modifiers
    {
        const char* virtualSig = "virtual SlangProfileID __cdecl SlangRecord::GlobalSessionProxy::findProfile(char const *)";
        const char* result = ReplayContext::parseSignature(virtualSig, buffer, sizeof(buffer));
        SLANG_CHECK(strcmp(result, "GlobalSessionProxy::findProfile") == 0);
    }

    // Test simple signature (no namespace)
    {
        const char* simpleSig = "int MyClass::myMethod(int, int)";
        const char* result = ReplayContext::parseSignature(simpleSig, buffer, sizeof(buffer));
        // Should handle this case gracefully
        SLANG_CHECK(result != nullptr);
    }
}

// =============================================================================
// End-to-End Playback Test: Global Session and Session Creation
// =============================================================================

// This test validates the full playback process for creating a global session
// and a session within it. The test has 3 stages:
// 
// Stage 1: Without replay system, create objects to establish baseline behavior
// Stage 2: With recording enabled, create same objects and verify proxy wrapping
// Stage 3: Playback the recording and verify objects are recreated correctly

SLANG_UNIT_TEST(replayContextEndToEndSessionPlayback)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Start recording
    ctx().setMode(Mode::Record);

    // Create a global session without replay
    SlangProfileID baselineProfile;
    Slang::ComPtr<slang::IGlobalSession> baselineGlobalSession;
    Slang::ComPtr<slang::ISession> baselineSession;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, baselineGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = baselineGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        SLANG_CHECK(SLANG_SUCCEEDED(baselineGlobalSession->createSession(sessionDesc, baselineSession.writeRef())));
        baselineProfile = targetDesc.profile;
        SLANG_CHECK(baselineProfile != SLANG_PROFILE_UNKNOWN);
    }

    // =========================================================================
    // Stage 2: Create objects WITH recording enabled and verify proxy wrapping
    // =========================================================================

    // Enable recording
    ctx().enable();
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Pretty much identical process but recording this time. From outside perspective,
    // should have exactly the same data (albeit wrapped in proxys)
    Slang::ComPtr<slang::IGlobalSession> recordedGlobalSession;
    Slang::ComPtr<slang::ISession> recordedSession;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, recordedGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = recordedGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        SLANG_CHECK(SLANG_SUCCEEDED(recordedGlobalSession->createSession(sessionDesc, recordedSession.writeRef())));
        SLANG_CHECK(baselineProfile == targetDesc.profile);
    }

    // Get the handle for the session so we can look it up after playback
    uint64_t recordedSessionHandle = ctx().getHandleForInterface(recordedSession.get());
    SLANG_CHECK(recordedSessionHandle >= kFirstValidHandle);

    // =========================================================================
    // Stage 3: Playback the recording and verify objects are recreated
    // =========================================================================

    // Switch to playback mode - this resets handles but keeps stream data
    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());

    // Execute all recorded calls
    // This should recreate the global session and session
    ctx().executeAll();

    // Look up the session by its handle - it should exist after playback
    ISlangUnknown* playedBackSessionUnk = ctx().getInterfaceForHandle(recordedSessionHandle);
    SLANG_CHECK(playedBackSessionUnk != nullptr);

    // Verify we can query the ISession interface
    Slang::ComPtr<slang::ISession> playedBackSession;
    SLANG_CHECK(SLANG_SUCCEEDED(playedBackSessionUnk->queryInterface(
        slang::ISession::getTypeGuid(), (void**)playedBackSession.writeRef())));

}




