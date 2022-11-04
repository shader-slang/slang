// slang-shader-cache-index.h
#pragma once
#include "../../slang.h"
#include "../../slang-gfx.h"
#include "../../slang-com-ptr.h"

#include "../../source/core/slang-string.h"
#include "../../source/core/slang-dictionary.h"
#include "../../source/core/slang-linked-list.h"
#include "../../source/core/slang-stream.h"

namespace gfx
{

using namespace Slang;

struct ShaderCacheEntry
{
    slang::Digest dependencyBasedDigest;
    slang::Digest contentsBasedDigest;
    double lastAccessedTime;

    bool operator==(const ShaderCacheEntry& rhs)
    {
        return dependencyBasedDigest == rhs.dependencyBasedDigest
            && contentsBasedDigest == rhs.contentsBasedDigest
            && lastAccessedTime == rhs.lastAccessedTime;
    }

    uint32_t getHashCode()
    {
        return dependencyBasedDigest.getHashCode();
    }
};

class PersistentShaderCache : public RefObject
{
public:
    PersistentShaderCache(const IDevice::ShaderCacheDesc& inDesc);
    ~PersistentShaderCache();

    // Fetch the cache entry corresponding to the provided key. If found, move the entry to
    // the front of entries and return the entry and the corresponding compiled code in
    // outCompiledCode. Else, return nullptr.
    ShaderCacheEntry* findEntry(const slang::Digest& key, ISlangBlob** outCompiledCode);

    // Add an entry to the cache with the provided key and contents hashes. If
    // adding an entry causes the cache to exceed size limitations, this will also
    // delete the least recently used entry.
    void addEntry(const slang::Digest& dependencyDigest, const slang::Digest& contentsDigest, ISlangBlob* compiledCode);

    // Update the contents hash for the specified entry in the cache and update the
    // corresponding file on disk.
    void updateEntry(const slang::Digest& dependencyDigest, const slang::Digest& contentsDigest, ISlangBlob* updatedCode);

private:
    // Load a previous cache index saved to disk. If not found, create a new cache index
    // and save it to disk as filename.
    void loadCacheFromFile();

    // Delete the last entry (the least recently used) from entries, remove its key/value pair
    // from keyToEntry, and remove the corresponding file on disk. Returns the index in 'entries'
    // of the removed entry so addEntry() can overwrite the corresponding entry in 'entries'
    // with the new entry. This should only be called by addEntry() when the cache reaches maximum capacity.
    Index deleteLRUEntry();

    // These functions perform the same tasks as their counterparts defined above. However, without access
    // to a physical file path, in-memory file systems cannot leverage file streams and need to fall back
    // on a different implementation.
    void loadCacheFromMemory();
    ShaderCacheEntry* findEntryInMemory(const slang::Digest& key, ISlangBlob** outCompiledCode);
    void addEntryToMemory(const slang::Digest& dependencyDigest, const slang::Digest& contentsDigest, ISlangBlob* compiledCode);
    void updateEntryInMemory(const slang::Digest& dependencyDigest, const slang::Digest& contentsDigest, ISlangBlob* updatedCode);
    void saveCacheToMemory();

    // The shader cache's description.
    IDevice::ShaderCacheDesc desc;

    // The underlying file system used for the shader cache.
    ComPtr<ISlangMutableFileSystem> mutableShaderCacheFileSystem = nullptr;
    bool isMemoryFileSystem = false;

    // A file stream to the index file opened during cache load. This will only
    // exist for a cache that exists on-disk.
    FileStream indexStream;

    // Dictionary mapping each shader's key to its corresponding node (entry) in the
    // linked list 'orderedEntries'.
    Dictionary<slang::Digest, LinkedNode<Index>*> keyToEntry;

    // Linked list containing the corresponding indices in 'entries' for entries in the
    // shader cache ordered from most to least recently used.
    LinkedList<Index> orderedEntries;

    // List of entries in the shader cache. This list is not guaranteed to be in order of recency
    // as the main and fall back implementations handle outputting to the file differently.
    List<ShaderCacheEntry> entries;
};

}
