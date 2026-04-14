// unit-test-replay-integration.cpp
// Integration tests: API calls, directory management

#include "unit-test-replay-common.h"

#include <chrono>
#include <thread>

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

    // Stop playback (or replay mechanism will still be running during cleanup)
    ctx().disable();
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
    // - file system handle
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
    SLANG_CHECK(readTypeIdFromStream(ctx().getStream()) == TypeId::UInt64);
    uint64_t fileSystemHandle = 0;
    ctx().getStream().read(&fileSystemHandle, sizeof(fileSystemHandle));
    SLANG_CHECK(fileSystemHandle == kDefaultFileSystemHandle); // NULL file system
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

    // Stop playback (or replay mechanism will still be running during cleanup)
    ctx().disable();
}

// =============================================================================
// Replay Directory Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextReplayDirectory)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test default directory
    SLANG_CHECK(strcmp(ctx().getReplayDirectory(), ".slang-replays") == 0);

    // Test setting custom directory
    ctx().setReplayDirectory("test-replays");
    SLANG_CHECK(strcmp(ctx().getReplayDirectory(), "test-replays") == 0);

    // Test that current replay path is nullptr when not recording
    SLANG_CHECK(ctx().getCurrentReplayPath() == nullptr);

    // Restore default
    ctx().setReplayDirectory(".slang-replays");
}

SLANG_UNIT_TEST(replayContextMirrorFileCreation)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Use a unique test directory
    ctx().setReplayDirectory(".slang-replays-test");

    // Enable recording - this should create the mirror file
    ctx().setMode(Mode::Record);

    // Check that we have a current replay path
    const char* replayPath = ctx().getCurrentReplayPath();
    SLANG_CHECK(replayPath != nullptr);

    // The path should contain our test directory
    SLANG_CHECK(strstr(replayPath, ".slang-replays-test") != nullptr);

    // Record some data
    int32_t value = 42;
    ctx().record(RecordFlag::None, value);

    // Disable recording - this should close the mirror file
    ctx().disable();

    // Current replay path should be nullptr now
    SLANG_CHECK(ctx().getCurrentReplayPath() == nullptr);

    // Restore default directory
    ctx().setReplayDirectory(".slang-replays");
}

SLANG_UNIT_TEST(replayContextLoadLatestReplay)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Use a unique test directory
    ctx().setReplayDirectory(".slang-replays-test");

    // Create a recording
    ctx().setMode(Mode::Record);

    // Record some data
    int32_t value1 = 123;
    float value2 = 3.14f;
    ctx().record(RecordFlag::None, value1);
    ctx().record(RecordFlag::None, value2);

    // Remember the replay path
    const char* firstReplayPath = ctx().getCurrentReplayPath();
    SLANG_CHECK(firstReplayPath != nullptr);
    String savedPath(firstReplayPath);

    // Disable recording
    ctx().disable();

    // Now load the latest replay
    SlangResult result = ctx().loadLatestReplay();
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(ctx().isPlayback());

    // Read back the values
    int32_t readValue1 = 0;
    float readValue2 = 0.0f;
    ctx().record(RecordFlag::None, readValue1);
    ctx().record(RecordFlag::None, readValue2);

    SLANG_CHECK(readValue1 == 123);
    SLANG_CHECK(readValue2 == 3.14f);

    // Clean up - reset and restore default directory
    ctx().reset();
    ctx().setReplayDirectory(".slang-replays");
}

SLANG_UNIT_TEST(replayContextFindLatestFolder)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test that the timestamp sorting works correctly
    // We'll create two recordings with a small delay between them

    ctx().setReplayDirectory(".slang-replays-test");

    // First recording
    ctx().setMode(Mode::Record);
    int32_t val1 = 111;
    ctx().record(RecordFlag::None, val1);
    String firstPath(ctx().getCurrentReplayPath());
    ctx().disable();

    // Small delay to ensure different timestamp
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // Second recording
    ctx().setMode(Mode::Record);
    int32_t val2 = 222;
    ctx().record(RecordFlag::None, val2);
    String secondPath(ctx().getCurrentReplayPath());
    ctx().disable();

    // The paths should be different
    SLANG_CHECK(firstPath != secondPath);

    // Find latest should return the second one's folder name
    String latest = ReplayContext::findLatestReplayFolder(".slang-replays-test");
    SLANG_CHECK(latest.getLength() > 0);

    // The full second path should end with the latest folder name
    SLANG_CHECK(secondPath.endsWith(latest));

    // Clean up
    ctx().reset();
    ctx().setReplayDirectory(".slang-replays");
}

// =============================================================================
// getSessionDescDigest: stream verification
// Verify the recorded stream contains the expected signature and SessionDesc.
// =============================================================================

SLANG_UNIT_TEST(replayContextRecordGetSessionDescDigestCall)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    slang::SessionDesc sessionDesc = {};
    Slang::ComPtr<ISlangBlob> digestBlob;
    SlangResult digestResult = globalSession->getSessionDescDigest(&sessionDesc, digestBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(digestResult));
    SLANG_CHECK(digestBlob != nullptr);

    ctx().switchToPlayback();

    // Skip past the slang_createGlobalSession2 call:
    // signature + this handle + global desc + output handle + result
    const char* signature = nullptr;
    ctx().record(RecordFlag::Input, signature);
    SLANG_CHECK(strcmp(signature, "slang_createGlobalSession2") == 0);
    uint64_t thisHandle = 0;
    ctx().recordHandle(RecordFlag::Input, thisHandle);
    SlangGlobalSessionDesc readGlobalDesc = {};
    ctx().record(RecordFlag::Input, readGlobalDesc);
    SLANG_CHECK(readTypeIdFromStream(ctx().getStream()) == TypeId::ObjectHandle);
    uint64_t globalContextHandle = 0;
    ctx().getStream().read(&globalContextHandle, sizeof(globalContextHandle));
    SlangResult createResult;
    ctx().record(RecordFlag::None, createResult);

    // Now read the getSessionDescDigest call:
    // - signature
    // - this handle
    // - input SessionDesc
    // - output blob (hash)
    // - return value
    ctx().record(RecordFlag::Input, signature);
    SLANG_CHECK(strstr(signature, "getSessionDescDigest") != nullptr);
    ctx().recordHandle(RecordFlag::Input, thisHandle);
    SLANG_CHECK(thisHandle == globalContextHandle);
    slang::SessionDesc readDesc = {};
    ctx().record(RecordFlag::Input, readDesc);
    SLANG_CHECK(readDesc.targetCount == sessionDesc.targetCount);
    SLANG_CHECK(readDesc.searchPathCount == sessionDesc.searchPathCount);

    // Read output blob (recorded as content hash)
    ISlangBlob* readBlob = nullptr;
    ctx().record(RecordFlag::Output, readBlob);

    // Read return value
    SlangResult readResult;
    ctx().record(RecordFlag::None, readResult);
    SLANG_CHECK(readResult == SLANG_OK);

    SLANG_CHECK(ctx().getStream().atEnd());
    ctx().disable();
}

// =============================================================================
// getSessionDescDigest: end-to-end replay via executeAll()
// This is the test that would crash before the PREPARE_POINTER_INPUT fix,
// because callWithDefaults passes nullptr for the SessionDesc* parameter.
// =============================================================================

SLANG_UNIT_TEST(replayContextGetSessionDescDigestPlayback)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().enable();
    ctx().reset();
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    slang::SessionDesc sessionDesc = {};
    Slang::ComPtr<ISlangBlob> digestBlob;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->getSessionDescDigest(&sessionDesc, digestBlob.writeRef())));
    SLANG_CHECK(digestBlob != nullptr);

    size_t recordedDigestSize = digestBlob->getBufferSize();
    SLANG_CHECK(recordedDigestSize > 0);

    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());

    // This calls through the proxy with default (null) arguments;
    // PREPARE_POINTER_INPUT ensures the null SessionDesc* is patched
    // to valid storage before RECORD_INPUT dereferences it.
    ctx().executeAll();

    ctx().disable();
}
