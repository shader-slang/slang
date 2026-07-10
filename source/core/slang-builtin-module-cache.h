// slang-builtin-module-cache.h

#ifndef SLANG_BUILTIN_MODULE_CACHE_H
#define SLANG_BUILTIN_MODULE_CACHE_H

#include "slang-blob.h"
#include "slang-string.h"

namespace Slang
{

/// Reads and writes the timestamp-prefixed cache used for serialized built-in modules.
///
/// The cache layout is [uint64_t library timestamp][serialized module bytes]. The timestamp is
/// stored in host byte order because the cache is tied to the shared-library build that created it,
/// rather than being a portable module archive. A timestamp of zero means that the library
/// timestamp is unavailable or that its modification time is exactly the epoch, so write() rejects
/// it and never creates a cache that could match the failure sentinel.
struct BuiltinModuleCache
{
    /// Reads a cache created for `expectedLibraryTimestamp` and returns its module payload.
    ///
    /// The output pointer refers to memory owned by outStorage and remains valid only while that
    /// storage stays alive and unmodified. Both output values are cleared when reading or
    /// validation fails.
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
