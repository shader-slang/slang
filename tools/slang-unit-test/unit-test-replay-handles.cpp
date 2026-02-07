// unit-test-replay-handles.cpp
// Unit tests for object handle tracking and TypeReflection serialization

#include "unit-test-replay-common.h"

// =============================================================================
// Object Handles (via interface recording)
// =============================================================================

SLANG_UNIT_TEST(replayContextHandle)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // This test verifies that blob content survives a record/playback round-trip
    // using hash-based serialization. Blobs are serialized by content hash to disk,
    // not tracked as COM interface proxies.
    
    const char* testData = "original data";
    size_t testDataSize = 13;

    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Simulate: API creates a blob and returns it as output
    Slang::ComPtr<ISlangBlob> recordedBlob = Slang::RawBlob::create(testData, testDataSize);
    ISlangBlob* outputBlob = recordedBlob.get();
    ctx().record(RecordFlag::Output, outputBlob);  // Hashes content, stores to disk
    
    // Simulate: The output is passed as input to another call
    ISlangBlob* inputBlob = recordedBlob.get();
    ctx().record(RecordFlag::Input, inputBlob);  // Hashes content, stores hash to stream
    
    // Verify recording produced data
    SLANG_CHECK(ctx().getStream().getSize() > 0);
    
    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();
    
    // Playback: First call outputs the blob - reads hash, loads from disk
    ISlangBlob* playbackOutput = nullptr;
    ctx().record(RecordFlag::Output, playbackOutput);
    SLANG_CHECK(playbackOutput != nullptr);
    SLANG_CHECK(playbackOutput->getBufferSize() == testDataSize);
    SLANG_CHECK(memcmp(playbackOutput->getBufferPointer(), testData, testDataSize) == 0);
    
    // Playback: Second call inputs the blob - reads hash, loads from disk
    ISlangBlob* playbackInput = nullptr;
    ctx().record(RecordFlag::Input, playbackInput);
    SLANG_CHECK(playbackInput != nullptr);
    SLANG_CHECK(playbackInput->getBufferSize() == testDataSize);
    SLANG_CHECK(memcmp(playbackInput->getBufferPointer(), testData, testDataSize) == 0);

    // Clean up
    playbackOutput->release();
    playbackInput->release();
}

SLANG_UNIT_TEST(replayContextHandleMultipleBlobs)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test with multiple blobs with different content to ensure hash-based
    // serialization correctly distinguishes them by content
    
    const char* data1 = "blob one";
    const char* data2 = "blob two";
    size_t size1 = 8;
    size_t size2 = 8;

    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Create and output two different blobs
    Slang::ComPtr<ISlangBlob> blob1 = Slang::RawBlob::create(data1, size1);
    Slang::ComPtr<ISlangBlob> blob2 = Slang::RawBlob::create(data2, size2);
    
    ISlangBlob* out1 = blob1.get();
    ISlangBlob* out2 = blob2.get();
    ctx().record(RecordFlag::Output, out1);
    ctx().record(RecordFlag::Output, out2);
    
    // Now input them in reverse order
    ISlangBlob* in2 = blob2.get();
    ISlangBlob* in1 = blob1.get();
    ctx().record(RecordFlag::Input, in2);
    ctx().record(RecordFlag::Input, in1);
    
    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();
    
    // Playback outputs - each creates a blob from disk
    ISlangBlob* playOut1 = nullptr;
    ISlangBlob* playOut2 = nullptr;
    ctx().record(RecordFlag::Output, playOut1);
    ctx().record(RecordFlag::Output, playOut2);
    
    SLANG_CHECK(playOut1 != nullptr);
    SLANG_CHECK(playOut2 != nullptr);
    SLANG_CHECK(memcmp(playOut1->getBufferPointer(), data1, size1) == 0);
    SLANG_CHECK(memcmp(playOut2->getBufferPointer(), data2, size2) == 0);
    
    // Playback inputs (reverse order, matching recording)
    ISlangBlob* playIn2 = nullptr;
    ISlangBlob* playIn1 = nullptr;
    ctx().record(RecordFlag::Input, playIn2);
    ctx().record(RecordFlag::Input, playIn1);
    
    // Verify correct blob content resolution
    SLANG_CHECK(playIn1 != nullptr);
    SLANG_CHECK(playIn2 != nullptr);
    SLANG_CHECK(memcmp(playIn1->getBufferPointer(), data1, size1) == 0);
    SLANG_CHECK(memcmp(playIn2->getBufferPointer(), data2, size2) == 0);

    // Clean up
    playOut1->release();
    playOut2->release();
    playIn1->release();
    playIn2->release();
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

    // Test that the same blob content serialized multiple times (as input then output
    // then input again) is correctly de-duplicated on disk and round-trips properly.

    const char* testData = "Inline then tracked";
    size_t testDataSize = strlen(testData) + 1;

    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);

    // User provides blob as input
    Slang::ComPtr<ISlangBlob> userBlob = Slang::RawBlob::create(testData, testDataSize);
    ISlangBlob* inputBlob = userBlob.get();
    ctx().record(RecordFlag::Input, inputBlob);

    // API stores it and later returns it as output
    ISlangBlob* outputBlob = userBlob.get();
    ctx().record(RecordFlag::Output, outputBlob);

    // Later, it's passed as input again
    ISlangBlob* inputAgain = userBlob.get();
    ctx().record(RecordFlag::Input, inputAgain);

    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();

    // First: read input blob
    ISlangBlob* readInput = nullptr;
    ctx().record(RecordFlag::Input, readInput);
    SLANG_CHECK(readInput != nullptr);
    SLANG_CHECK(readInput->getBufferSize() == testDataSize);
    SLANG_CHECK(memcmp(readInput->getBufferPointer(), testData, testDataSize) == 0);

    // Second: read output blob (same content, loaded from same hash on disk)
    ISlangBlob* readOutput = nullptr;
    ctx().record(RecordFlag::Output, readOutput);
    SLANG_CHECK(readOutput != nullptr);
    SLANG_CHECK(readOutput->getBufferSize() == testDataSize);
    SLANG_CHECK(memcmp(readOutput->getBufferPointer(), testData, testDataSize) == 0);

    // Third: read input again
    ISlangBlob* readAgain = nullptr;
    ctx().record(RecordFlag::Input, readAgain);
    SLANG_CHECK(readAgain != nullptr);
    SLANG_CHECK(readAgain->getBufferSize() == testDataSize);
    SLANG_CHECK(memcmp(readAgain->getBufferPointer(), testData, testDataSize) == 0);

    // Clean up
    readInput->release();
    readOutput->release();
    readAgain->release();
}

// =============================================================================
// TypeReflection Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextTypeReflectionNull)
{
    REPLAY_TEST;

    // Test null TypeReflection round-trip
    ctx().enable();

    slang::TypeReflection* writeType = nullptr;
    ctx().record(RecordFlag::Input, writeType);

    ctx().switchToPlayback();

    slang::TypeReflection* readType = reinterpret_cast<slang::TypeReflection*>(0xDEADBEEF);
    ctx().record(RecordFlag::Input, readType);

    SLANG_CHECK(readType == nullptr);
}

SLANG_UNIT_TEST(replayContextTypeReflectionBasic)
{
    REPLAY_TEST;

    // Now test recording/playback
    ctx().enable();
    ctx().setTtyLogging(true);

    // Create a global session and session to load a module
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    // Load a simple module with a type we can look up
    const char* testCode = R"(
        struct MyTestStruct
        {
            float x;
            int y;
        };
    )";
    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModuleFromSourceString(
        "test-module",
        "test-module.slang",
        testCode,
        diagnostics.writeRef());
    auto* layout = module->getLayout();
    slang::TypeReflection* originalType = layout->findTypeByName("MyTestStruct");

    // Record the type explicitly
    slang::TypeReflection* writeType = originalType;
    ctx().record(RecordFlag::Input, writeType);

    // Switch to playback and execute the calls that get us to the point
    // at which the type was recorded.
    ctx().switchToPlayback();
    ctx().executeNextCall(); //slang_createGlobalSession
    ctx().executeNextCall(); //globalSession->findProfile
    ctx().executeNextCall(); //globalSession->createSession
    ctx().executeNextCall(); //session->loadModuleFromSourceString
    ctx().executeNextCall(); //module->getLayout

    //<layout->findTypeByName not captured>
 
    // Now at the point we recorded a type
    slang::TypeReflection* readType = nullptr;
    ctx().record(RecordFlag::Input, readType);

    // Verify the type was recovered
    SLANG_CHECK(readType != nullptr);
    ComPtr<ISlangBlob> originalNameBlob;
    ComPtr<ISlangBlob> readNameBlob;
    originalType->getFullName(originalNameBlob.writeRef());
    readType->getFullName(readNameBlob.writeRef());
    SLANG_CHECK(originalNameBlob != nullptr);
    SLANG_CHECK(readNameBlob != nullptr);
    const char* originalName = (const char*)originalNameBlob->getBufferPointer();
    const char* readName = (const char*)readNameBlob->getBufferPointer();
    SLANG_CHECK(strcmp(originalName, readName) == 0);

    // We messed with the stream, so clear context before it tries to clean itself up
    ctx().reset();
}

SLANG_UNIT_TEST(replayContextTypeReflectionBuiltinType)
{
    REPLAY_TEST;

    // Enable recording first
    ctx().enable();
    ctx().setTtyLogging(true);

    // Create a global session and session
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));

    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    // Load a module that uses built-in types
    const char* testCode = R"(
        struct TestStruct
        {
            float3 position;
            float4x4 transform;
        };
    )";

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModuleFromSourceString(
        "builtin-test",
        "builtin-test.slang",
        testCode,
        diagnostics.writeRef());
    SLANG_CHECK(module != nullptr);

    auto* layout = module->getLayout();
    SLANG_CHECK(layout != nullptr);

    // Get a builtin type like float3
    slang::TypeReflection* float3Type = layout->findTypeByName("float3");
    SLANG_CHECK(float3Type != nullptr);

    // Record the builtin type
    slang::TypeReflection* writeType = float3Type;
    ctx().record(RecordFlag::Input, writeType);

    // Switch to playback and execute the calls that get us to the point
    // at which the type was recorded.
    ctx().switchToPlayback();
    ctx().executeNextCall(); //slang_createGlobalSession
    ctx().executeNextCall(); //globalSession->findProfile
    ctx().executeNextCall(); //globalSession->createSession
    ctx().executeNextCall(); //session->loadModuleFromSourceString
    ctx().executeNextCall(); //module->getLayout

    //<layout->findTypeByName not captured>

    // Now at the point we recorded a type
    slang::TypeReflection* readType = nullptr;
    ctx().record(RecordFlag::Input, readType);

    SLANG_CHECK(readType != nullptr);

    // Verify name matches
    ComPtr<ISlangBlob> originalNameBlob;
    ComPtr<ISlangBlob> readNameBlob;
    float3Type->getFullName(originalNameBlob.writeRef());
    readType->getFullName(readNameBlob.writeRef());

    const char* originalName = (const char*)originalNameBlob->getBufferPointer();
    const char* readName = (const char*)readNameBlob->getBufferPointer();

    SLANG_CHECK(strcmp(originalName, readName) == 0);

    // We messed with the stream, so clear context before it tries to clean itself up
    ctx().reset();
}
