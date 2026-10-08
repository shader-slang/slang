// unit-test-builtin-module-cache.cpp

#include "core/slang-builtin-module-cache.h"
#include "core/slang-io.h"
#include "core/slang-shared-library.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(builtinModuleCache)
{
    String cachePath;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-core-module-cache"), cachePath)));

    const uint8_t moduleData[] = {0x12, 0x34, 0x56, 0x78};
    const uint64_t libraryTimestamp = 123456789;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        BuiltinModuleCache::write(cachePath, libraryTimestamp, moduleData, sizeof(moduleData))));

    ScopedAllocation cacheStorage;
    const void* loadedModuleData = nullptr;
    size_t loadedModuleSize = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(BuiltinModuleCache::read(
        cachePath,
        libraryTimestamp,
        cacheStorage,
        loadedModuleData,
        loadedModuleSize)));
    SLANG_CHECK(loadedModuleSize == sizeof(moduleData));
    SLANG_CHECK(memcmp(loadedModuleData, moduleData, sizeof(moduleData)) == 0);

    SLANG_CHECK(SLANG_FAILED(BuiltinModuleCache::read(
        cachePath,
        libraryTimestamp + 1,
        cacheStorage,
        loadedModuleData,
        loadedModuleSize)));
    SLANG_CHECK(loadedModuleData == nullptr);
    SLANG_CHECK(loadedModuleSize == 0);

    // Zero is the unavailable-timestamp sentinel, so it must never produce a cache.
    SLANG_CHECK(
        SLANG_FAILED(BuiltinModuleCache::write(cachePath, 0, moduleData, sizeof(moduleData))));

    // A header without a payload is valid and must not underflow the reported payload size.
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(BuiltinModuleCache::write(cachePath, libraryTimestamp, moduleData, 0)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(BuiltinModuleCache::read(
        cachePath,
        libraryTimestamp,
        cacheStorage,
        loadedModuleData,
        loadedModuleSize)));
    SLANG_CHECK(loadedModuleSize == 0);

    // A truncated header must fail before the reader tries to copy its timestamp.
    const uint8_t truncatedCacheData[] = {0x01, 0x02, 0x03};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllBytes(cachePath, truncatedCacheData, sizeof(truncatedCacheData))));
    loadedModuleData = moduleData;
    loadedModuleSize = sizeof(moduleData);
    SLANG_CHECK(SLANG_FAILED(BuiltinModuleCache::read(
        cachePath,
        libraryTimestamp,
        cacheStorage,
        loadedModuleData,
        loadedModuleSize)));
    SLANG_CHECK(loadedModuleData == nullptr);
    SLANG_CHECK(loadedModuleSize == 0);

    // A real file has a usable timestamp, while empty and missing paths return the sentinel.
    SLANG_CHECK(SharedLibraryUtils::getFileTimestamp(cachePath) != 0);
    SLANG_CHECK(SharedLibraryUtils::getFileTimestamp(String()) == 0);

    String missingCachePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::generateTemporary(toSlice("slang-core-module-cache-missing"), missingCachePath)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::remove(missingCachePath)));
    SLANG_CHECK(SharedLibraryUtils::getFileTimestamp(missingCachePath) == 0);

    // A missing cache must clear its outputs before the caller falls back to compilation.
    loadedModuleData = moduleData;
    loadedModuleSize = sizeof(moduleData);
    SLANG_CHECK(SLANG_FAILED(BuiltinModuleCache::read(
        missingCachePath,
        libraryTimestamp,
        cacheStorage,
        loadedModuleData,
        loadedModuleSize)));
    SLANG_CHECK(loadedModuleData == nullptr);
    SLANG_CHECK(loadedModuleSize == 0);

    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(cachePath)));
}
