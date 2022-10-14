// slang-shader-cache-index.cpp
#include "slang-shader-cache-index.h"

#include "slang-hash-utils.h"
#include "../core/slang-io.h"
#include "../core/slang-string-util.h"

namespace Slang
{

// Load a previous cache index saved to disk. If not found, create a new cache index
// and save it to disk as filename.
SlangResult ShaderCacheIndex::loadCacheIndexFromFile(String filename)
{
    ComPtr<ISlangBlob> indexBlob;
    if (SLANG_FAILED(shaderCacheFileSystem->loadFile(filename.getBuffer(), indexBlob.writeRef())))
    {
        // Cache index not found, so we'll create and save a new one.
        if (mutableShaderCacheFileSystem)
        {
            SLANG_RETURN_ON_FAIL(mutableShaderCacheFileSystem->saveFile(filename.getBuffer(), nullptr, 0));
            indexFilename = filename;
            return SLANG_OK;
        }
        // Cache index not found and we can't save a new one due to the file system being immutable.
        return SLANG_FAIL;
    }

    String indexString;
    File::readAllText(filename, indexString);

    List<UnownedStringSlice> lines;
    StringUtil::calcLines(indexString.getUnownedSlice(), lines);
    for (auto line : lines)
    {
        List<UnownedStringSlice> digests;
        StringUtil::split(line, ' ', digests); // This will return our two hashes as two elements in digests.
        auto dependencyDigest = stringToHash(digests[0]);
        auto astDigest = stringToHash(digests[1]);

        ShaderCacheEntry entry = { dependencyDigest, astDigest };
        auto entryNode = entries.AddFirst(entry);
        keyToEntry.Add(digests[0], entryNode);
    }
    return SLANG_OK;
}

LinkedNode<ShaderCacheIndex::ShaderCacheEntry>* ShaderCacheIndex::findEntry(const slang::Digest& key, ISlangBlob** outCompiledCode)
{
    LinkedNode<ShaderCacheEntry>* entryNode;
    auto keyString = hashToString(key);
    if (!keyToEntry.TryGetValue(keyString, entryNode))
    {
        // The key was not found in the cache, so we return nullptr.
        *outCompiledCode = nullptr;
        return nullptr;
    }

    // If the key is found, we need to move its corresponding entry to the front of
    // the list and load the stored contents from disk.
    entries.RemoveFromList(entryNode);
    entries.AddFirst(entryNode);
    shaderCacheFileSystem->loadFile(keyString.getBuffer(), outCompiledCode);
    return entryNode;
}

void ShaderCacheIndex::addEntry(const slang::Digest& dependencyDigest, const slang::Digest& astDigest, ISlangBlob* compiledCode)
{
    if (!mutableShaderCacheFileSystem)
    {
        // Should not save new entries if the underlying file system isn't mutable.
        return;
    }
    
    // Check that we do not exceed the cache's size limit by adding another entry. If so,
    // remove the least recently used entry first.
    while (entryCountLimit > 0 && entries.Count() >= entryCountLimit)
    {
        deleteLRUEntry();
    }

    auto digestString = hashToString(dependencyDigest);
    ShaderCacheEntry entry = { dependencyDigest, astDigest };
    auto entryNode = entries.AddFirst(entry);
    keyToEntry.Add(digestString, entryNode);

    mutableShaderCacheFileSystem->saveFileBlob(digestString.getBuffer(), compiledCode);

    saveCacheIndexToFile();
}

void ShaderCacheIndex::updateEntry(
    LinkedNode<ShaderCacheEntry>* entryNode,
    const slang::Digest& dependencyDigest,
    const slang::Digest& astDigest,
    ISlangBlob* updatedCode)
{
    if (!mutableShaderCacheFileSystem)
    {
        // Updating entries requires saving to disk in order to overwrite the old shader file
        // on disk, so we return if the underlying file system isn't mutable.
        return;
    }

    entryNode->Value.astBasedDigest = astDigest;
    mutableShaderCacheFileSystem->saveFileBlob(hashToString(dependencyDigest).getBuffer(), updatedCode);

    saveCacheIndexToFile();
}

SlangResult ShaderCacheIndex::saveCacheIndexToFile()
{
    if (!mutableShaderCacheFileSystem)
    {
        // Cannot save the index to disk if the underlying file system isn't mutable.
        // A file system of some kind is required for shader caching to even be enabled, so
        // this is not a failure.
        return SLANG_OK;
    }

    StringBuilder indexString;
    for (auto& entry : entries)
    {
        indexString.append(hashToString(entry.dependencyBasedDigest));
        indexString.append(" ");
        indexString.append(hashToString(entry.astBasedDigest));
        indexString.append("\n");
    }

    SLANG_RETURN_ON_FAIL(mutableShaderCacheFileSystem->saveFile(indexFilename.getBuffer(), indexString.getBuffer(), indexString.getLength()));

    return SLANG_OK;
}

void ShaderCacheIndex::deleteLRUEntry()
{
    if (!mutableShaderCacheFileSystem)
    {
        // This is here as a safety precaution but should never be hit as
        // addEntry() is the only function that should call this.
        return;
    }

    auto lruEntry = entries.LastNode();
    auto shaderKey = lruEntry->Value.dependencyBasedDigest;
    auto shaderKeyString = hashToString(shaderKey);

    keyToEntry.Remove(shaderKeyString);
    mutableShaderCacheFileSystem->remove(shaderKeyString.getBuffer());

    entries.Delete(lruEntry);
}

}
