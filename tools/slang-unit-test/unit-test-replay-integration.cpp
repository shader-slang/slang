// unit-test-replay-integration.cpp
// Integration tests: API calls, directory management, file system

#include "unit-test-replay-common.h"

// Only this file uses MutableFileSystemProxy
#include "../../source/slang-record-replay/proxy/proxy-mutable-file-system.h"

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
    // (millisecond precision should be enough)
    
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
// File System Proxy Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextFileSystemProxyLoadFile)
{
    REPLAY_TEST;

    // Use a unique test directory for this test's replays
    ctx().setReplayDirectory(".slang-replays-fs-test");

    // Create a file system proxy wrapping the OS file system
    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Define test file content and write it out (using non-recorded FS)
    const char* testFileName = ".slang-test-temp-file.txt";
    const char* testContent = "Hello from file system proxy test!\nLine 2\nLine 3";
    size_t testContentSize = strlen(testContent);
    SlangResult writeResult = osFileSystem->saveFile(testFileName, testContent, testContentSize);
    SLANG_CHECK(SLANG_SUCCEEDED(writeResult));

    // Enable recording - this creates the replay directory
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Load the file through the proxy - this should capture its content
    ComPtr<ISlangBlob> blob;
    SlangResult result = fsProxy->loadFile(testFileName, blob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(blob != nullptr);
    SLANG_CHECK(blob->getBufferSize() == testContentSize);
    SLANG_CHECK(memcmp(blob->getBufferPointer(), testContent, testContentSize) == 0);

    // Delete the file (using non-recorded FS)
    SlangResult removeResult = osFileSystem->remove(testFileName);
    SLANG_CHECK(SLANG_SUCCEEDED(removeResult));

    // Switch to playback - the file system proxy should serve from captured files
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Should be able to replay, even though the file is gone.
    ComPtr<ISlangBlob> replayedBlob;
    SlangResult replayResult = fsProxy->loadFile(testFileName, replayedBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedBlob != nullptr);
    SLANG_CHECK(replayedBlob->getBufferSize() == testContentSize);
    SLANG_CHECK(memcmp(replayedBlob->getBufferPointer(), testContent, testContentSize) == 0);

    // Clean up
    ctx().reset();
    ctx().setReplayDirectory(".slang-replays");
}

SLANG_UNIT_TEST(replayContextFileSystemProxyCalcCombinedPath)
{
    REPLAY_TEST;

    // Create a file system proxy wrapping the OS file system
    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call calcCombinedPath through the proxy
    ComPtr<ISlangBlob> pathBlob;
    SlangResult result = fsProxy->calcCombinedPath(
        SLANG_PATH_TYPE_DIRECTORY, "/base/dir", "subpath/file.txt", pathBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(pathBlob != nullptr);

    // Store the result for comparison
    UnownedStringSlice recordedPath(
        (const char*)pathBlob->getBufferPointer(), pathBlob->getBufferSize());

    // Switch to playback - the file system proxy should serve from recorded data
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call calcCombinedPath again - should read from recorded data, not call actual FS
    ComPtr<ISlangBlob> replayedPathBlob;
    SlangResult replayResult = fsProxy->calcCombinedPath(
        SLANG_PATH_TYPE_DIRECTORY, "/base/dir", "subpath/file.txt", replayedPathBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedPathBlob != nullptr);

    // The replayed path should match the recorded path
    UnownedStringSlice replayedPath(
        (const char*)replayedPathBlob->getBufferPointer(), replayedPathBlob->getBufferSize());
    SLANG_CHECK(recordedPath == replayedPath);

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayContextFileSystemProxyGetPathType)
{
    REPLAY_TEST;

    // Create a temp file for testing
    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    const char* testFileName = ".slang-test-pathtype-file.txt";
    SlangResult writeResult = osFileSystem->saveFile(testFileName, "test", 4);
    SLANG_CHECK(SLANG_SUCCEEDED(writeResult));

    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getPathType through the proxy
    SlangPathType pathType = SLANG_PATH_TYPE_DIRECTORY;
    SlangResult result = fsProxy->getPathType(testFileName, &pathType);
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(pathType == SLANG_PATH_TYPE_FILE);

    // Delete the file
    osFileSystem->remove(testFileName);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getPathType again - should read from recorded data even though file is gone
    SlangPathType replayedPathType = SLANG_PATH_TYPE_DIRECTORY;
    SlangResult replayResult = fsProxy->getPathType(testFileName, &replayedPathType);
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedPathType == SLANG_PATH_TYPE_FILE);

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayContextFileSystemProxySaveFile)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* testFileName = ".slang-test-savefile-replay.txt";
    const char* testContent = "Test content for replay";

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Save file through proxy
    SlangResult result = fsProxy->saveFile(testFileName, testContent, strlen(testContent));
    SLANG_CHECK(SLANG_SUCCEEDED(result));

    // Verify file was actually written
    SLANG_CHECK(File::exists(testFileName));

    // Delete the file
    osFileSystem->remove(testFileName);
    SLANG_CHECK(!File::exists(testFileName));

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call saveFile again through playback - should NOT actually write to disk
    SlangResult replayResult = fsProxy->saveFile(testFileName, testContent, strlen(testContent));
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));

    // File should NOT exist because playback doesn't call actual FS
    SLANG_CHECK(!File::exists(testFileName));

    // Clean up
    ctx().reset();
}
