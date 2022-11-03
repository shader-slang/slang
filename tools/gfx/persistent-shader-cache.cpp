// slang-shader-cache-index.cpp
#include "persistent-shader-cache.h"

#include "../../source/core/slang-digest-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-file-system.h"

#include "../../source/core/slang-char-util.h"

#include <chrono>

namespace gfx
{

using namespace std::chrono;

PersistentShaderCache::PersistentShaderCache(const IDevice::ShaderCacheDesc& inDesc)
{
    desc = inDesc;

    // If a path is provided, we will want our underlying file system to be initialized using that path.
    if (desc.shaderCachePath)
    {
        if (!desc.shaderCacheFileSystem)
        {
            // Only a path was provided, so we get a mutable file system
            // using OSFileSystem::getMutableSingleton.
            desc.shaderCacheFileSystem = OSFileSystem::getMutableSingleton();
        }
        desc.shaderCacheFileSystem = new RelativeFileSystem(desc.shaderCacheFileSystem, desc.shaderCachePath);
    }

    // If our shader cache has an underlying file system, check if it's mutable. If so, store a pointer
    // to the mutable version for operations which require writing to disk.
    if (desc.shaderCacheFileSystem)
    {
        desc.shaderCacheFileSystem->queryInterface(ISlangMutableFileSystem::getTypeGuid(), (void**)mutableShaderCacheFileSystem.writeRef());
    }

    loadCacheFromFile();
}

// Load a previous cache index saved to disk. If not found, create a new cache index
// and save it to disk as filename.
void PersistentShaderCache::loadCacheFromFile()
{
    // We will need to combine the filename with the cache path in order to have the correct
    // file path for initializing the stream. This needs to be done separately because there
    // is no guarantee that the underlying file system is mutable.
    String filePath;
    if (mutableShaderCacheFileSystem)
    {
        ComPtr<ISlangBlob> fullPath;
        mutableShaderCacheFileSystem->getPath(PathKind::OperatingSystem, desc.cacheFilename, fullPath.writeRef());
        filePath = String((char*)fullPath->getBufferPointer());
    }
    else
    {
        filePath = Path::combine(String(desc.shaderCachePath), String(desc.cacheFilename));
    }

    if (SLANG_FAILED(indexStream.init(filePath, FileMode::Open, FileAccess::ReadWrite, FileShare::ReadWrite)))
    {
        // If we failed to open a stream to the file, then the file does not yet exist on disk.
        // We will create the index file if our underlying file system is mutable.
        if (mutableShaderCacheFileSystem)
        {
            indexStream.init(filePath, FileMode::Create, FileAccess::ReadWrite, FileShare::ReadWrite);
        }
        return;
    }
    else
    {
        const auto start = indexStream.getPosition();
        indexStream.seek(SeekOrigin::End, 0);
        const auto end = indexStream.getPosition();
        indexStream.seek(SeekOrigin::Start, 0);
        const Index numEntries = (Index)(end - start) / sizeof(ShaderCacheEntry);

        if (desc.entryCountLimit > 0 && numEntries > desc.entryCountLimit)
        {
            // If the size limit for the current cache is smaller than the cache that produced the file we're trying to
            // load, re-create the entire file.
            //
            // FileStream does not currently have any methods for truncating an existing file, so in this case, our cache
            // index would no longer accurately reflect the state of our cache due to the extra now-garbage lines present.
            // While this has no impact on cache operation, it could be problematic for debugging purposes, etc.
            indexStream.close();
            indexStream.init(filePath, FileMode::Create, FileAccess::ReadWrite, FileShare::ReadWrite);
            return;
        }
        else
        {
            // The cache index is not guaranteed to be ordered by most recent access, so we need a temporary list to store
            // all the entries in order to sort them before filling in our linked list.
            List<ShaderCacheEntry> tempEntries;
            size_t offset = 0;
            for (Index i = 0; i < numEntries; ++i)
            {
                ShaderCacheEntry entry;
                size_t bytesRead;
                indexStream.read(&entry, sizeof(ShaderCacheEntry), bytesRead); // TODO: Error checking?

                tempEntries.add(entry);
            }

            // We will need to sort tempEntries by last accessed time before we can add entries to our linked list.
            tempEntries.quickSort(tempEntries.getBuffer(), 0, tempEntries.getCount() - 1, [](ShaderCacheEntry a, ShaderCacheEntry b) { return a.lastAccessedTime > b.lastAccessedTime; });
            for (auto& entry : tempEntries)
            {
                // If we reach this point, then the current cache is at least the same size in entries as the cache
                // that produced the index we're reading in, so we don't need to check if we're exceeding capacity.
                entries.add(entry);
                auto entryIndexNode = orderedEntries.AddLast(entries.getCount() - 1);
                keyToEntry.Add(entry.dependencyBasedDigest, entryIndexNode);
            }
        }   
    }
}

ShaderCacheEntry* PersistentShaderCache::findEntry(const slang::Digest& key, ISlangBlob** outCompiledCode)
{
    LinkedNode<Index>* entryIndexNode;
    if (!keyToEntry.TryGetValue(key, entryIndexNode))
    {
        // The key was not found in the cache, so we return nullptr.
        *outCompiledCode = nullptr;
        return nullptr;
    }

    // If the key is found, load the stored contents from disk. We then move the corresponding
    // entry to the front of the linked list and update the cache file on disk
    desc.shaderCacheFileSystem->loadFile(DigestUtil::toString(key).getBuffer(), outCompiledCode);
    auto index = entryIndexNode->Value;
    entries[index].lastAccessedTime = (double)high_resolution_clock::now().time_since_epoch().count();
    if (orderedEntries.FirstNode() != entryIndexNode)
    {
        orderedEntries.RemoveFromList(entryIndexNode);
        orderedEntries.AddFirst(entryIndexNode);
        if (mutableShaderCacheFileSystem)
        {
            auto offset = index * sizeof(ShaderCacheEntry);
            indexStream.seek(SeekOrigin::Start, offset + 2 * sizeof(slang::Digest));
            indexStream.write(&entries[index].lastAccessedTime, sizeof(double));
            indexStream.flush();
        }
    }
    return &entries[index];
}

void PersistentShaderCache::addEntry(const slang::Digest& dependencyDigest, const slang::Digest& contentsDigest, ISlangBlob* compiledCode)
{
    if (!mutableShaderCacheFileSystem)
    {
        // Should not save new entries if the underlying file system isn't mutable.
        return;
    }
    
    // Check that we do not exceed the cache's size limit by adding another entry. If so,
    // remove the least recently used entry first.
    //
    // In theory, the cache could be more than just one entry over the entry count limit.
    // However, this is impossible in practice because we fully re-create the entry list
    // and cache index file if the size of the current cache is smaller than the cache
    // that generated the index file we loaded. In any case, the initial number of entries
    // in the cache will always be fewer than the size limit and this check will be hit
    // on the first entry added that exceeds the cache's size.
    Index index = entries.getCount();
    if (desc.entryCountLimit > 0 && orderedEntries.Count() >= desc.entryCountLimit)
    {
        index = deleteLRUEntry();
    }

    auto lastAccessedTime = (double)high_resolution_clock::now().time_since_epoch().count();

    ShaderCacheEntry entry = { dependencyDigest, contentsDigest, lastAccessedTime };
    if (index == entries.getCount())
    {
        entries.add(entry);
    }
    else
    {
        entries[index] = entry;
    }
    auto entryNode = orderedEntries.AddFirst(index);
    keyToEntry.Add(dependencyDigest, entryNode);

    mutableShaderCacheFileSystem->saveFileBlob(DigestUtil::toString(dependencyDigest).getBuffer(), compiledCode);

    indexStream.seek(SeekOrigin::End, 0);
    indexStream.write(&entry, sizeof(ShaderCacheEntry));
    indexStream.flush();
}

void PersistentShaderCache::updateEntry(
    const slang::Digest& dependencyDigest,
    const slang::Digest& contentsDigest,
    ISlangBlob* updatedCode)
{
    if (!mutableShaderCacheFileSystem)
    {
        // Updating entries requires saving to disk in order to overwrite the old shader file
        // on disk, so we return if the underlying file system isn't mutable.
        return;
    }

    // Unlike in addEntry(), we only update the contents digest here because the last accessed time will have already
    // been updated while finding the entry.
    auto entryIndexNode = *keyToEntry.TryGetValue(dependencyDigest);
    auto index = entryIndexNode->Value;
    entries[index].contentsBasedDigest = contentsDigest;
    mutableShaderCacheFileSystem->saveFileBlob(DigestUtil::toString(dependencyDigest).getBuffer(), updatedCode);

    auto offset = index * sizeof(ShaderCacheEntry);
    indexStream.seek(SeekOrigin::Start, offset + sizeof(slang::Digest));
    indexStream.write(&contentsDigest, sizeof(slang::Digest));
    indexStream.flush();
}

Index PersistentShaderCache::deleteLRUEntry()
{
    if (!mutableShaderCacheFileSystem)
    {
        // This is here as a safety precaution but should never be hit as
        // addEntry() is the only function that should call this.
        return -1;
    }

    auto lruEntry = orderedEntries.LastNode();
    auto index = lruEntry->Value;
    auto shaderKey = entries[index].dependencyBasedDigest;

    keyToEntry.Remove(shaderKey);
    mutableShaderCacheFileSystem->remove(DigestUtil::toString(shaderKey).getBuffer());

    orderedEntries.Delete(lruEntry);
    return index;
}

}
