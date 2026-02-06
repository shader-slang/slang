// unit-test-replay-filesystem.cpp
// Comprehensive tests for file system replay functionality

#include "unit-test-replay-common.h"

// Only this file uses MutableFileSystemProxy
#include "../../source/slang-record-replay/proxy/proxy-mutable-file-system.h"

// =============================================================================
// File System Proxy Tests - ISlangFileSystem interface
// =============================================================================

SLANG_UNIT_TEST(replayFileSystemProxyLoadFile)
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

// =============================================================================
// File System Proxy Tests - ISlangFileSystemExt interface
// =============================================================================

SLANG_UNIT_TEST(replayFileSystemProxyGetFileUniqueIdentity)
{
    REPLAY_TEST;

    // Create a temp file for testing
    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    const char* testFileName = ".slang-test-uniqueid-file.txt";
    SlangResult writeResult = osFileSystem->saveFile(testFileName, "test content", 12);
    SLANG_CHECK(SLANG_SUCCEEDED(writeResult));

    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getFileUniqueIdentity through the proxy
    ComPtr<ISlangBlob> uniqueIdBlob;
    SlangResult result = fsProxy->getFileUniqueIdentity(testFileName, uniqueIdBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(uniqueIdBlob != nullptr);
    SLANG_CHECK(uniqueIdBlob->getBufferSize() > 0);

    // Store the unique ID for comparison
    UnownedStringSlice recordedId(
        (const char*)uniqueIdBlob->getBufferPointer(), uniqueIdBlob->getBufferSize());

    // Delete the file
    osFileSystem->remove(testFileName);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getFileUniqueIdentity again - should read from recorded data
    ComPtr<ISlangBlob> replayedIdBlob;
    SlangResult replayResult = fsProxy->getFileUniqueIdentity(testFileName, replayedIdBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedIdBlob != nullptr);

    // The replayed ID should match the recorded ID
    UnownedStringSlice replayedId(
        (const char*)replayedIdBlob->getBufferPointer(), replayedIdBlob->getBufferSize());
    SLANG_CHECK(recordedId == replayedId);

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyCalcCombinedPath)
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

SLANG_UNIT_TEST(replayFileSystemProxyGetPathType)
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

SLANG_UNIT_TEST(replayFileSystemProxyGetPath)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getPath through the proxy to simplify a path
    const char* complexPath = "/base/dir/../other/./file.txt";
    ComPtr<ISlangBlob> simplifiedBlob;
    SlangResult result = fsProxy->getPath(PathKind::Simplified, complexPath, simplifiedBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(simplifiedBlob != nullptr);

    // Store the result for comparison
    UnownedStringSlice recordedPath(
        (const char*)simplifiedBlob->getBufferPointer(), simplifiedBlob->getBufferSize());

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getPath again - should read from recorded data
    ComPtr<ISlangBlob> replayedBlob;
    SlangResult replayResult = fsProxy->getPath(PathKind::Simplified, complexPath, replayedBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedBlob != nullptr);

    // The replayed path should match the recorded path
    UnownedStringSlice replayedPath(
        (const char*)replayedBlob->getBufferPointer(), replayedBlob->getBufferSize());
    SLANG_CHECK(recordedPath == replayedPath);

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyClearCache)
{
    REPLAY_TEST;
    SLANG_CHECK(true); // this test only fails through exceptions, so record a check

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call clearCache through the proxy - this is a void function
    fsProxy->clearCache();

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call clearCache again - should replay successfully without calling actual FS
    fsProxy->clearCache();

    // Clean up
    ctx().reset();
}

struct EnumerationEntry
{
    SlangPathType pathType;
    String name;
};

struct EnumerationResults
{
    List<EnumerationEntry> entries;
};

static void testEnumerateCallback(SlangPathType pathType, const char* name, void* userData)
{
    auto results = static_cast<EnumerationResults*>(userData);
    EnumerationEntry entry;
    entry.pathType = pathType;
    entry.name = name;
    results->entries.add(entry);
}

SLANG_UNIT_TEST(replayFileSystemProxyEnumeratePathContents)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Create a temp directory with some files for enumeration
    const char* testDir = ".slang-test-enum-dir";
    osFileSystem->createDirectory(testDir);
    osFileSystem->saveFile(".slang-test-enum-dir/file1.txt", "test1", 5);
    osFileSystem->saveFile(".slang-test-enum-dir/file2.txt", "test2", 5);

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call enumeratePathContents through the proxy, capturing results
    EnumerationResults recordedResults;
    SlangResult result = fsProxy->enumeratePathContents(testDir, testEnumerateCallback, &recordedResults);
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(recordedResults.entries.getCount() >= 2); // At least our 2 files

    // Verify we got file1.txt and file2.txt
    bool foundFile1 = false, foundFile2 = false;
    for (const auto& entry : recordedResults.entries)
    {
        if (entry.name == "file1.txt")
        {
            foundFile1 = true;
            SLANG_CHECK(entry.pathType == SLANG_PATH_TYPE_FILE);
        }
        if (entry.name == "file2.txt")
        {
            foundFile2 = true;
            SLANG_CHECK(entry.pathType == SLANG_PATH_TYPE_FILE);
        }
    }
    SLANG_CHECK(foundFile1 && foundFile2);

    // Clean up the directory
    osFileSystem->remove(".slang-test-enum-dir/file1.txt");
    osFileSystem->remove(".slang-test-enum-dir/file2.txt");
    osFileSystem->remove(testDir);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call enumeratePathContents again - should read from recorded data
    EnumerationResults replayedResults;
    SlangResult replayResult = fsProxy->enumeratePathContents(testDir, testEnumerateCallback, &replayedResults);
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    
    // Verify that replayed results exactly match recorded results
    SLANG_CHECK(replayedResults.entries.getCount() == recordedResults.entries.getCount());
    for (Index i = 0; i < recordedResults.entries.getCount(); i++)
    {
        SLANG_CHECK(replayedResults.entries[i].pathType == recordedResults.entries[i].pathType);
        SLANG_CHECK(replayedResults.entries[i].name == recordedResults.entries[i].name);
    }

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyGetOSPathKind)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getOSPathKind through the proxy
    OSPathKind pathKind = fsProxy->getOSPathKind();
    SLANG_CHECK(pathKind != OSPathKind::None); // OS file system should have a valid path kind

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call getOSPathKind again - should read from recorded data
    OSPathKind replayedPathKind = fsProxy->getOSPathKind();
    SLANG_CHECK(replayedPathKind == pathKind);

    // Clean up
    ctx().reset();
}

// =============================================================================
// File System Proxy Tests - ISlangMutableFileSystem interface
// =============================================================================

SLANG_UNIT_TEST(replayFileSystemProxySaveFile)
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

SLANG_UNIT_TEST(replayFileSystemProxySaveFileBlob)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* testFileName = ".slang-test-savefileblob-replay.txt";
    const char* testContent = "Test blob content for replay";
    
    // Create a blob with test content
    ComPtr<ISlangBlob> testBlob = Slang::RawBlob::create(testContent, strlen(testContent));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Save file blob through proxy
    SlangResult result = fsProxy->saveFileBlob(testFileName, testBlob.get());
    SLANG_CHECK(SLANG_SUCCEEDED(result));

    // Verify file was actually written
    SLANG_CHECK(File::exists(testFileName));

    // Load it back to verify content
    ComPtr<ISlangBlob> loadedBlob;
    osFileSystem->loadFile(testFileName, loadedBlob.writeRef());
    SLANG_CHECK(loadedBlob->getBufferSize() == strlen(testContent));
    SLANG_CHECK(memcmp(loadedBlob->getBufferPointer(), testContent, strlen(testContent)) == 0);

    // Delete the file
    osFileSystem->remove(testFileName);
    SLANG_CHECK(!File::exists(testFileName));

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call saveFileBlob again through playback - should NOT actually write to disk
    SlangResult replayResult = fsProxy->saveFileBlob(testFileName, testBlob.get());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));

    // File should NOT exist because playback doesn't call actual FS
    SLANG_CHECK(!File::exists(testFileName));

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyRemove)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* testFileName = ".slang-test-remove-replay.txt";

    // Create a test file
    osFileSystem->saveFile(testFileName, "test", 4);
    SLANG_CHECK(File::exists(testFileName));

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Remove file through proxy
    SlangResult result = fsProxy->remove(testFileName);
    SLANG_CHECK(SLANG_SUCCEEDED(result));

    // Verify file was actually removed
    SLANG_CHECK(!File::exists(testFileName));

    // Recreate the file for playback test
    osFileSystem->saveFile(testFileName, "test", 4);
    SLANG_CHECK(File::exists(testFileName));

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call remove again through playback - should NOT actually remove from disk
    SlangResult replayResult = fsProxy->remove(testFileName);
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));

    // File should still exist because playback doesn't call actual FS
    SLANG_CHECK(File::exists(testFileName));

    // Clean up - remove the file we created
    osFileSystem->remove(testFileName);

    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyCreateDirectory)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* testDirName = ".slang-test-createdir-replay";

    // Make sure directory doesn't exist
    osFileSystem->remove(testDirName);

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Create directory through proxy
    SlangResult result = fsProxy->createDirectory(testDirName);
    SLANG_CHECK(SLANG_SUCCEEDED(result));

    // Verify directory was actually created
    SlangPathType pathType;
    osFileSystem->getPathType(testDirName, &pathType);
    SLANG_CHECK(pathType == SLANG_PATH_TYPE_DIRECTORY);

    // Remove the directory
    osFileSystem->remove(testDirName);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Call createDirectory again through playback - should NOT actually create on disk
    SlangResult replayResult = fsProxy->createDirectory(testDirName);
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));

    // Directory should NOT exist because playback doesn't call actual FS
    SlangResult typeResult = osFileSystem->getPathType(testDirName, &pathType);
    SLANG_CHECK(SLANG_FAILED(typeResult)); // Should fail because directory doesn't exist

    // Clean up
    ctx().reset();
}

// =============================================================================
// Edge Cases and Error Handling
// =============================================================================

SLANG_UNIT_TEST(replayFileSystemProxyLoadFileNotFound)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* nonExistentFile = ".slang-test-nonexistent-file-12345.txt";

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Try to load a file that doesn't exist
    ComPtr<ISlangBlob> blob;
    SlangResult result = fsProxy->loadFile(nonExistentFile, blob.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(blob == nullptr);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Try to load the same file - should get the same error result
    ComPtr<ISlangBlob> replayedBlob;
    SlangResult replayResult = fsProxy->loadFile(nonExistentFile, replayedBlob.writeRef());
    SLANG_CHECK(SLANG_FAILED(replayResult));
    SLANG_CHECK(replayedBlob == nullptr);

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyEmptyFile)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* emptyFileName = ".slang-test-empty-file.txt";

    // Create an empty file
    osFileSystem->saveFile(emptyFileName, "", 0);

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Load the empty file
    ComPtr<ISlangBlob> blob;
    SlangResult result = fsProxy->loadFile(emptyFileName, blob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(blob != nullptr);
    SLANG_CHECK(blob->getBufferSize() == 0);

    // Delete the file
    osFileSystem->remove(emptyFileName);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Load the empty file again through replay
    ComPtr<ISlangBlob> replayedBlob;
    SlangResult replayResult = fsProxy->loadFile(emptyFileName, replayedBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedBlob != nullptr);
    SLANG_CHECK(replayedBlob->getBufferSize() == 0);

    // Clean up
    ctx().reset();
}

SLANG_UNIT_TEST(replayFileSystemProxyLargeFile)
{
    REPLAY_TEST;

    auto osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));

    const char* largeFileName = ".slang-test-large-file.txt";

    // Create a large file (1MB of data)
    const size_t largeSize = 1024 * 1024;
    List<uint8_t> largeData;
    largeData.setCount(largeSize);
    for (size_t i = 0; i < largeSize; i++)
    {
        largeData[i] = static_cast<uint8_t>(i % 256);
    }
    osFileSystem->saveFile(largeFileName, largeData.getBuffer(), largeSize);

    // Enable recording
    ctx().enable();
    ctx().setMode(Mode::Record);
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Load the large file
    ComPtr<ISlangBlob> blob;
    SlangResult result = fsProxy->loadFile(largeFileName, blob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(blob != nullptr);
    SLANG_CHECK(blob->getBufferSize() == largeSize);

    // Verify content
    SLANG_CHECK(memcmp(blob->getBufferPointer(), largeData.getBuffer(), largeSize) == 0);

    // Delete the file
    osFileSystem->remove(largeFileName);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(fsProxy.get());
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    // Load the large file again through replay
    ComPtr<ISlangBlob> replayedBlob;
    SlangResult replayResult = fsProxy->loadFile(largeFileName, replayedBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(replayResult));
    SLANG_CHECK(replayedBlob != nullptr);
    SLANG_CHECK(replayedBlob->getBufferSize() == largeSize);

    // Verify replayed content matches
    SLANG_CHECK(memcmp(replayedBlob->getBufferPointer(), largeData.getBuffer(), largeSize) == 0);

    // Clean up
    ctx().reset();
}
