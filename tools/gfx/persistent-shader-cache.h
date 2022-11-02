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

    // Fetch the cache entry corresponding to the provided key. If found, move the entry to
    // the front of entries and return the entry and the corresponding compiled code in
    // outCompiledCode. Else, return nullptr.
    LinkedNode<ShaderCacheEntry>* findEntry(const slang::Digest& key, ISlangBlob** outCompiledCode);

    // Add an entry to the cache with the provided key and contents hashes. If
    // adding an entry causes the cache to exceed size limitations, this will also
    // delete the least recently used entry.
    void addEntry(const slang::Digest& dependencyDigest, const slang::Digest& contentsDigest, ISlangBlob* compiledCode);

    // Update the contents hash for the specified entry in the cache and update the
    // corresponding file on disk.
    void updateEntry(
        LinkedNode<ShaderCacheEntry>* entryNode,
        const slang::Digest& dependencyDigest,
        const slang::Digest& contentsDigest,
        ISlangBlob* updatedCode);

private:
    // Load a previous cache index saved to disk. If not found, create a new cache index
    // and save it to disk as filename.
    void loadCacheFromFile();

//     // Update the cache index on disk. This should be called any time an entry changes.
//     void saveCacheToFile();

    // Delete the last entry (the least recently used) from entries, remove its key/value pair
    // from keyToEntry, and remove the corresponding file on disk. This should only be called
    // by addEntry() when the cache reaches maximum capacity.
    void deleteLRUEntry();

    // The shader cache's description.
    IDevice::ShaderCacheDesc desc;

    // Dictionary mapping each shader's key to its corresponding node (entry) in the list
    // of entries.
    Dictionary<slang::Digest, LinkedNode<ShaderCacheEntry>*> keyToEntry;

    // Linked list containing the entries stored in the shader cache in order
    // of most to least recently used.
    LinkedList<ShaderCacheEntry> entries;

    // The underlying file system used for the shader cache.
    ComPtr<ISlangMutableFileSystem> mutableShaderCacheFileSystem = nullptr;

    // A file stream to the index file opened during cache load.
    FileStream indexStream;

    // Dictionary mapping each entry to its offset in the file stream.
    Dictionary<ShaderCacheEntry, Int64> entryToOffset;
};

}
