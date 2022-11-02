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
    ComPtr<ISlangBlob> indexBlob;
    if (SLANG_FAILED(desc.shaderCacheFileSystem->loadFile(desc.cacheFilename, indexBlob.writeRef())))
    {
        // Cache index not found and we can't save a new one due to the file system being immutable.
        if (!mutableShaderCacheFileSystem)
        {
            return;
        }
        // Cache index not found, so we'll create and save a new one.
        mutableShaderCacheFileSystem->saveFile(desc.cacheFilename, nullptr, 0);
    }
    else
    {
        auto indexString = UnownedStringSlice((char*)indexBlob->getBufferPointer());

        List<UnownedStringSlice> lines;
        StringUtil::calcLines(indexString, lines);
        if (desc.entryCountLimit > 0 && lines.getCount() > desc.entryCountLimit)
        {
            // If the size limit for the current cache is smaller than the cache that produced the file we're trying to
            // load, re-create the entire file.
            //
            // FileStream does not currently have any methods for truncating an existing file, so in this case, our cache
            // index would no longer accurately reflect the state of our cache due to the extra now-garbage lines present.
            // While this has no impact on cache operation, it could be problematic for debugging purposes, etc.
            mutableShaderCacheFileSystem->saveFile(desc.cacheFilename, nullptr, 0);
        }
        else
        {
            // Index is not guaranteed to be ordered by most recent access, so we need a temporary list to store
            // all the entries.
            List<ShaderCacheEntry> tempEntries;
            size_t offset = 0;
            for (auto line : lines)
            {
                List<UnownedStringSlice> digests;
                StringUtil::split(line, ' ', digests); // This will return our two hashes as two elements in digests, unless we've reached the end.
                if (digests.getCount() != 3)
                    continue;
                auto dependencyDigest = DigestUtil::fromString(digests[0]);
                auto contentsDigest = DigestUtil::fromString(digests[1]);
                double lastAccessedTime = 0;
                for (Index i = 0; i < digests[2].getLength(); ++i)
                {
                    lastAccessedTime += (double)CharUtil::getHexDigitValue(digests[2][i]) * (double)pow(16, digests[2].getLength() - 1 - i);
                }

                ShaderCacheEntry entry = { dependencyDigest, contentsDigest, lastAccessedTime };
                entryToOffset.Add(entry, offset);
                tempEntries.add(entry);
                offset += line.getLength();
            }

            // We will need to sort tempEntries by last accessed time before we can add entries to our linked list.
            tempEntries.quickSort(tempEntries.getBuffer(), 0, tempEntries.getCount() - 1, [](ShaderCacheEntry a, ShaderCacheEntry b) { return a.lastAccessedTime > b.lastAccessedTime; });
            for (auto& entry : tempEntries)
            {
                // If we reach this point, then the current cache is at least the same size in entries as the cache
                // that produced the index we're reading in, so we don't need to check if we're exceeding capacity.
                auto entryNode = entries.AddLast(entry);
                keyToEntry.Add(entry.dependencyBasedDigest, entryNode);
            }
        }   
    }
    
    indexStream.init(String(desc.cacheFilename), FileMode::Append, FileAccess::ReadWrite, FileShare::ReadWrite);
}

LinkedNode<ShaderCacheEntry>* PersistentShaderCache::findEntry(const slang::Digest& key, ISlangBlob** outCompiledCode)
{
    LinkedNode<ShaderCacheEntry>* entryNode;
    if (!keyToEntry.TryGetValue(key, entryNode))
    {
        // The key was not found in the cache, so we return nullptr.
        *outCompiledCode = nullptr;
        return nullptr;
    }

    // If the key is found, load the stored contents from disk. We then move the corresponding
    // entry to the front of the linked list and update the cache file on disk
    desc.shaderCacheFileSystem->loadFile(DigestUtil::toString(key).getBuffer(), outCompiledCode);
    if (entries.FirstNode() != entryNode)
    {
        entries.RemoveFromList(entryNode);
        entries.AddFirst(entryNode);
        if (mutableShaderCacheFileSystem)
        {
            auto lastAccessedString = String(duration_cast<duration<double>>(high_resolution_clock::now().time_since_epoch()).count());
            // TODO: Is this offset always correct due to needing to turn the time into a string?
            auto offset = entryToOffset.TryGetValue(entryNode->Value);
            indexStream.seek(SeekOrigin::Start, *offset + 36);
            indexStream.write(lastAccessedString.getBuffer(), lastAccessedString.getLength());
        }
    }
    return entryNode;
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
    // In theory, this could loop infinitely since deleteLRUEntry() immediately returns if
    // mutableShaderCacheFileSystem is not set. However, this situation is functionally impossible
    // because we check immediately before this as well.
    while (desc.entryCountLimit > 0 && entries.Count() >= desc.entryCountLimit)
    {
        deleteLRUEntry();
    }

    auto lastAccessedTime = duration_cast<duration<double>>(high_resolution_clock::now().time_since_epoch()).count();

    ShaderCacheEntry entry = { dependencyDigest, contentsDigest, lastAccessedTime };
    auto entryNode = entries.AddFirst(entry);
    keyToEntry.Add(dependencyDigest, entryNode);

    mutableShaderCacheFileSystem->saveFileBlob(DigestUtil::toString(dependencyDigest).getBuffer(), compiledCode);

    indexStream.seek(SeekOrigin::End, 0);
    entryToOffset.Add(entry, indexStream.getPosition());
    StringBuilder entrySb;
    entrySb << dependencyDigest;
    entrySb << " ";
    entrySb << contentsDigest;
    entrySb << " ";
    entrySb << lastAccessedTime;
    entrySb << "\n";
    indexStream.write(entrySb.getBuffer(), entrySb.getLength());
}

void PersistentShaderCache::updateEntry(
    LinkedNode<ShaderCacheEntry>* entryNode,
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

    auto lastAccessedTime = duration_cast<duration<double>>(high_resolution_clock::now().time_since_epoch()).count();
    entryNode->Value.contentsBasedDigest = contentsDigest;
    entryNode->Value.lastAccessedTime = lastAccessedTime;
    mutableShaderCacheFileSystem->saveFileBlob(DigestUtil::toString(dependencyDigest).getBuffer(), updatedCode);

    auto offset = entryToOffset.TryGetValue(entryNode->Value);
    indexStream.seek(SeekOrigin::Start, *offset + 33);
    StringBuilder entrySb;
    entrySb << contentsDigest;
    entrySb << " ";
    entrySb << lastAccessedTime;
    entrySb << "\n";
    indexStream.write(entrySb.getBuffer(), entrySb.getLength());
}

// void PersistentShaderCache::saveCacheToFile()
// {
//     if (!mutableShaderCacheFileSystem)
//     {
//         // Cannot save the index to disk if the underlying file system isn't mutable.
//         return;
//     }
// 
//     StringBuilder indexSb;
//     for (auto& entry : entries)
//     {
//         indexSb << entry.dependencyBasedDigest;
//         indexSb << " ";
//         indexSb << entry.contentsBasedDigest;
//         indexSb << "\n";
//     }
// 
//     mutableShaderCacheFileSystem->saveFile(desc.cacheFilename, indexSb.getBuffer(), indexSb.getLength());
// }

void PersistentShaderCache::deleteLRUEntry()
{
    if (!mutableShaderCacheFileSystem)
    {
        // This is here as a safety precaution but should never be hit as
        // addEntry() is the only function that should call this.
        return;
    }

    auto lruEntry = entries.LastNode();
    auto shaderKey = lruEntry->Value.dependencyBasedDigest;

    keyToEntry.Remove(shaderKey);
    mutableShaderCacheFileSystem->remove(DigestUtil::toString(shaderKey).getBuffer());

    entries.Delete(lruEntry);
}

}
