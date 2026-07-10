// unit-test-builtin-module-cache.cpp

#include "core/slang-builtin-module-cache.h"
#include "core/slang-io.h"
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

    SLANG_CHECK(SLANG_SUCCEEDED(File::remove(cachePath)));
}
