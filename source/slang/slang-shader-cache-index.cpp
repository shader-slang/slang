#include "slang-shader-cache-index.h"

#include "slang-hash-utils.h"
#include "../core/slang-io.h"

namespace Slang
{

// Load a previous cache index saved to disk. If not found, create a new cache index
// and save it to disk as filename.
SlangResult ShaderCacheIndex::loadCacheIndexFromFile(String filename)
{
    ComPtr<ISlangBlob> indexBlob;
    if (SLANG_FAILED(shaderCacheFileSystem->loadFile(filename.getBuffer(), indexBlob.writeRef())))// Should use the mutable system if available
    {
        // Cache index not found, so we'll create and save a new one.
        if (mutableShaderCacheFileSystem)
        {
            SLANG_RETURN_ON_FAIL(mutableShaderCacheFileSystem->saveFile(filename.getBuffer(), nullptr, 0));
            indexFilename = filename;
            return SLANG_OK;
        }
    }

    String indexString;
    File::readAllText(filename, indexString);
    for (Index i = 0; i < indexString.getLength(); i += 66)
    {
        auto depString = String(indexString.subString(66 * i, 32)); // 66 characters per row, 32 per hash plus either space or newline
        slang::Digest dependencyDigest = stringToHash(depString);

        auto astString = String(indexString.subString(66 * i + 33, 32));
        slang::Digest astDigest = stringToHash(astString);

        ShaderCacheEntry entry = { dependencyDigest, astDigest };
        auto entryNode = entries.AddFirst(entry);
        keyToEntry.Add(dependencyDigest, entryNode);
    }
}

ShaderCacheIndex::ShaderCacheEntry* ShaderCacheIndex::findEntry(const slang::Digest& key)
{
    LinkedNode<ShaderCacheEntry>* entryNode;
    if (!keyToEntry.TryGetValue(key, entryNode))
    {
        // The key was not found in the cache, so we return nullptr.
        return nullptr;
    }

    // If the key is found, we need to move its corresponding entry to the front of
    // the list.
    entries.MoveToFirst(entryNode);
    return &entryNode->Value;
}

void ShaderCacheIndex::addEntry(const slang::Digest& dependencyDigest, const slang::Digest& astDigest, ISlangBlob* compiledCode)
{
    // Check that we do not exceed the cache's size limit by adding another entry. If so,
    // remove the least recently used entry first.
    if (entries.Count() == entryCountLimit)
    {
        deleteLRUEntry();
    }

    ShaderCacheEntry entry = { dependencyDigest, astDigest };
    auto entryNode = entries.AddFirst(entry);
    keyToEntry.Add(dependencyDigest, entryNode);

    if (mutableShaderCacheFileSystem)
    {
        mutableShaderCacheFileSystem->saveFileBlob(hashToString(dependencyDigest).getBuffer(), compiledCode);
    }

    saveCacheIndexToFile();
}

void ShaderCacheIndex::updateEntry(const slang::Digest& dependencyDigest, const slang::Digest& astDigest, ISlangBlob* updatedCode)
{
    LinkedNode<ShaderCacheEntry>* entryNode;
    keyToEntry.TryGetValue(dependencyDigest, entryNode);
    entryNode->Value.astBasedDigest = astDigest;

    if (mutableShaderCacheFileSystem)
    {
        mutableShaderCacheFileSystem->saveFileBlob(hashToString(dependencyDigest).getBuffer(), updatedCode);
    }

    saveCacheIndexToFile();
}

SlangResult ShaderCacheIndex::saveCacheIndexToFile()
{
    StringBuilder indexString;
    for (auto& entry : entries)
    {
        indexString.append(hashToString(entry.dependencyBasedDigest));
        indexString.append(" ");
        indexString.append(hashToString(entry.astBasedDigest));
        indexString.append("\n");
    }

    if (mutableShaderCacheFileSystem)
    {
        SLANG_RETURN_ON_FAIL(mutableShaderCacheFileSystem->saveFile(indexFilename.getBuffer(), indexString.getBuffer(), indexString.getLength()));
    }
    return SLANG_OK;
}

void ShaderCacheIndex::deleteLRUEntry()
{
    auto lruEntry = entries.LastNode();
    auto shaderKey = lruEntry->Value.dependencyBasedDigest;

    keyToEntry.Remove(shaderKey);
    if (mutableShaderCacheFileSystem)
    {
        mutableShaderCacheFileSystem->remove(hashToString(shaderKey).getBuffer());
    }

    entries.Delete(lruEntry);
}

}
