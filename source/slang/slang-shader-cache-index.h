#pragma once
#include "../../slang.h"

#include "../core/slang-string.h"
#include "../core/slang-linked-list.h"
#include "../core/slang-dictionary.h"

namespace Slang
{
    class ShaderCacheIndex
    {
    public:
        ShaderCacheIndex(SlangInt size)
            : entryLimit(size)
        {}

        // Load a previous cache index saved to disk. If not found, create a new cache index
        // and save it to disk as filename.
        SlangResult loadCacheIndex(String filename);

        // Fetch the cache entry corresponding to the provided key. If found, store the
        // mapped AST hash for that entry in outContents and return SLANG_OK. Else, set
        // outContents to nullptr and return SLANG_FAIL.
        SlangResult fetchEntry(String key, String* outContents);

        // Add an entry to the cache with the provided key and contents hashes. If
        // adding an entry causes the cache to exceed size limitations, this will also
        // delete the least recently used entry.
        void addEntry(String key, String contents);

        // Update the contents hash for the specified entry in the cache.
        void updateEntry(String key, String newContents);

    private:
        // Update the entry for key in the cache index on disk. This should be called any
        // time keyToContents changes.
        SlangResult updateCacheIndex(String key);

        // Move the entry corresponding to key to the front of entries. This should only
        // be called by fetchEntry upon successfully finding an existing entry.
        void moveEntryToFront(String key);

        // Delete the last entry from entries and remove its key/value pair from keyToContents
        // as well as remove the corresponding file on disk. This should only be used when the
        // cache has reached the size limit specified by entryLimit to create space for a new entry.
        SlangResult deleteEntry();

        // Dictionary mapping each shader's key (dependency-based hash) in the cache
        // to its contents (AST-based hash).
        Dictionary<String, String> keyToContents;

        // Linked list containing the entries stored in the shader cache in order
        // of most to least recently used.
        LinkedList<String> entries;

        // The maximum number of cache entries allowed.
        SlangInt entryLimit = -1;
    }

    // Notes:
    // - Should cache index load/save take a filename? Easiest to maintain if the filename is something set,
    //   but maybe someone might use the same directory for multiple caches?
    // - Do we ever need to delete an entry aside from when the cache reaches size limitations?

}
