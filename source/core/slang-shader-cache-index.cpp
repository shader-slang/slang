// slang-shader-cache-index.cpp
#include "slang-shader-cache-index.h"

#include "../slang/slang-hash-utils.h"
#include "slang-io.h"
#include "slang-string-util.h"
#include "slang-file-system.h"

namespace Slang
{

ShaderCacheIndex::ShaderCacheIndex(SlangInt size, String path, ISlangFileSystem* fileSystem, String filename)
{
    entryCountLimit = size;
    shaderCacheFileSystem = fileSystem;
    indexFilename = filename;

    if (!path.getBuffer())
    {
        if (!shaderCacheFileSystem)
        {
            // Only a path was provided, so we get a mutable file system
            // using OSFileSystem::getMutableSingleton.
            shaderCacheFileSystem = OSFileSystem::getMutableSingleton();
        }
        shaderCacheFileSystem = new RelativeFileSystem(shaderCacheFileSystem, path);
    }

    // If our shader cache has an underlying file system, check if it's mutable. If so, store a pointer
    // to the mutable version in order to save new entries later.
    if (shaderCacheFileSystem)
    {
        shaderCacheFileSystem->queryInterface(ISlangMutableFileSystem::getTypeGuid(), (void**)mutableShaderCacheFileSystem.writeRef());
    }

    loadCacheIndexFromFile();
}

// Load a previous cache index saved to disk. If not found, create a new cache index
// and save it to disk as filename.
void ShaderCacheIndex::loadCacheIndexFromFile()
{
    ComPtr<ISlangBlob> indexBlob;
    if (SLANG_FAILED(shaderCacheFileSystem->loadFile(indexFilename.getBuffer(), indexBlob.writeRef())))
    {
        // Cache index not found, so we'll create and save a new one.
        if (mutableShaderCacheFileSystem)
        {
            mutableShaderCacheFileSystem->saveFile(indexFilename.getBuffer(), nullptr, 0);
            return;
        }
        // Cache index not found and we can't save a new one due to the file system being immutable.
        return;
    }

    String indexString;
    File::readAllText(indexFilename, indexString);

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
        keyToEntry.Add(dependencyDigest, entryNode);
    }
}

LinkedNode<ShaderCacheEntry>* ShaderCacheIndex::findEntry(const slang::Digest& key, ISlangBlob** outCompiledCode)
{
    LinkedNode<ShaderCacheEntry>* entryNode;
    if (!keyToEntry.TryGetValue(key, entryNode))
    {
        // The key was not found in the cache, so we return nullptr.
        *outCompiledCode = nullptr;
        return nullptr;
    }

    // If the key is found, we need to move its corresponding entry to the front of
    // the list and load the stored contents from disk.
    entries.RemoveFromList(entryNode);
    entries.AddFirst(entryNode);
    shaderCacheFileSystem->loadFile(hashToString(key).getBuffer(), outCompiledCode);
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

    ShaderCacheEntry entry = { dependencyDigest, astDigest };
    auto entryNode = entries.AddFirst(entry);
    keyToEntry.Add(dependencyDigest, entryNode);

    mutableShaderCacheFileSystem->saveFileBlob(hashToString(dependencyDigest).getBuffer(), compiledCode);

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

void ShaderCacheIndex::saveCacheIndexToFile()
{
    if (!mutableShaderCacheFileSystem)
    {
        // Cannot save the index to disk if the underlying file system isn't mutable.
        return;
    }

    StringBuilder indexString;
    for (auto& entry : entries)
    {
        indexString.append(hashToString(entry.dependencyBasedDigest));
        indexString.append(" ");
        indexString.append(hashToString(entry.astBasedDigest));
        indexString.append("\n");
    }

    mutableShaderCacheFileSystem->saveFile(indexFilename.getBuffer(), indexString.getBuffer(), indexString.getLength());
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

    keyToEntry.Remove(shaderKey);
    mutableShaderCacheFileSystem->remove(hashToString(shaderKey).getBuffer());

    entries.Delete(lruEntry);
}

}
