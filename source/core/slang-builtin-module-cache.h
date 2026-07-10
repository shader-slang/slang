// slang-builtin-module-cache.h

#ifndef SLANG_BUILTIN_MODULE_CACHE_H
#define SLANG_BUILTIN_MODULE_CACHE_H

#include "slang-blob.h"
#include "slang-string.h"

namespace Slang
{

/// Reads and writes the timestamp-prefixed cache used for serialized built-in modules.
struct BuiltinModuleCache
{
    /// Reads a cache created for `expectedLibraryTimestamp` and returns its module payload.
    static SlangResult read(
        const String& cachePath,
        uint64_t expectedLibraryTimestamp,
        ScopedAllocation& outStorage,
        const void*& outModuleData,
        size_t& outModuleSize);

    /// Writes `moduleData` with the library timestamp required by the runtime cache loader.
    static SlangResult write(
        const String& cachePath,
        uint64_t libraryTimestamp,
        const void* moduleData,
        size_t moduleSize);
};

} // namespace Slang

#endif // SLANG_BUILTIN_MODULE_CACHE_H
