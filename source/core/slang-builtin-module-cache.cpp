// slang-builtin-module-cache.cpp

#include "slang-builtin-module-cache.h"

#include "slang-io.h"

namespace Slang
{

SlangResult BuiltinModuleCache::read(
    const String& cachePath,
    uint64_t expectedLibraryTimestamp,
    ScopedAllocation& outStorage,
    const void*& outModuleData,
    size_t& outModuleSize)
{
    outModuleData = nullptr;
    outModuleSize = 0;

    SLANG_RETURN_ON_FAIL(File::readAllBytes(cachePath, outStorage));
    if (outStorage.getSizeInBytes() < sizeof(uint64_t))
        return SLANG_FAIL;

    uint64_t cacheTimestamp = 0;
    memcpy(&cacheTimestamp, outStorage.getData(), sizeof(cacheTimestamp));
    if (cacheTimestamp != expectedLibraryTimestamp)
        return SLANG_FAIL;

    outModuleData = (const uint8_t*)outStorage.getData() + sizeof(cacheTimestamp);
    outModuleSize = outStorage.getSizeInBytes() - sizeof(cacheTimestamp);
    return SLANG_OK;
}

SlangResult BuiltinModuleCache::write(
    const String& cachePath,
    uint64_t libraryTimestamp,
    const void* moduleData,
    size_t moduleSize)
{
    if (libraryTimestamp == 0)
        return SLANG_FAIL;

    FileStream fileStream;
    SLANG_RETURN_ON_FAIL(fileStream.init(cachePath, FileMode::Create));
    SLANG_RETURN_ON_FAIL(fileStream.write(&libraryTimestamp, sizeof(libraryTimestamp)));
    SLANG_RETURN_ON_FAIL(fileStream.write(moduleData, moduleSize));
    return SLANG_OK;
}

} // namespace Slang
