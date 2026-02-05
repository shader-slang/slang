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

    // This test emulates a real-world scenario:
    // 1. RECORD: An API call creates a blob and outputs it, then later inputs it
    // 2. PLAYBACK: A new blob is created, and when the 2nd input occurs,
    //    the system correctly identifies it by handle and provides the new blob
    
    // === RECORDING PHASE ===
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Simulate: API creates a blob and returns it as output (e.g., getCompileResult)
    // The 'record' call should wrap the recordedBlob in a proxy
    Slang::ComPtr<ISlangBlob> recordedBlob = Slang::RawBlob::create("original data", 13);
    ISlangBlob* outputBlob = recordedBlob.get();
    ctx().record(RecordFlag::Output, outputBlob);  // Registers blob with handle 1, records handle
    
    // Simulate: The output is passed as input to another call (e.g., writeToFile)
    // The 'record' call should unwrap the proxy and spit out the original
    ISlangBlob* inputBlob = outputBlob;
    ctx().record(RecordFlag::Input, inputBlob);  // Looks up handle for blob, records handle
    SLANG_CHECK(inputBlob == recordedBlob.get());
    
    // Verify recording produced data
    SLANG_CHECK(ctx().getStream().getSize() > 0);
    
    // === PLAYBACK PHASE ===
    ctx().switchToPlayback();
    
    // During playback, a NEW blob is created (simulating the real API being called)
    Slang::ComPtr<ISlangBlob> playbackBlob = Slang::RawBlob::create("playback data", 13);
    
    // Playback: First call outputs the blob - we register the new blob with the handle
    ISlangBlob* playbackOutput = playbackBlob.get();
    ctx().record(RecordFlag::Output, playbackOutput);  // Reads handle, verifies/registers
    
    // Playback: Second call should replay the same mechanism - playbackInput should end
    // up as an unwrapped reference to the original blob we fed in.
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
    ISlangBlob* in2 = out2;
    ISlangBlob* in1 = out1;
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
    ISlangBlob* inputAgain = outputBlob;
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
