#include "slang-archive-file-system.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-io.h"
#include "slang-string-util.h"
#include "slang-blob.h"
#include "slang-string-slice-pool.h"
#include "slang-uint-set.h"

#include "slang-riff-file-system.h"

// Compression systems
#include "slang-deflate-compression-system.h"
#include "slang-lz4-compression-system.h"

// Zip file system
#include "slang-zip-file-system.h"

#include "slang-riff.h"

namespace Slang
{

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringSliceIndexMap !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

StringSliceIndexMap::CountIndex StringSliceIndexMap::add(const UnownedStringSlice& key, Index valueIndex)
{
    StringSlicePool::Handle handle;
    m_pool.findOrAdd(key, handle);
    const CountIndex countIndex = StringSlicePool::asIndex(handle);
    if (countIndex >= m_indexMap.getCount())
    {
        SLANG_ASSERT(countIndex == m_indexMap.getCount());
        m_indexMap.add(valueIndex);
    }
    else
    {
        m_indexMap[countIndex] = valueIndex;
    }
    return countIndex;
}

StringSliceIndexMap::CountIndex StringSliceIndexMap::findOrAdd(const UnownedStringSlice& key, Index defaultValueIndex)
{
    StringSlicePool::Handle handle;
    m_pool.findOrAdd(key, handle);
    const CountIndex countIndex = StringSlicePool::asIndex(handle);
    if (countIndex >= m_indexMap.getCount())
    {
        SLANG_ASSERT(countIndex == m_indexMap.getCount());
        m_indexMap.add(defaultValueIndex);
    }
    return countIndex;
}

void StringSliceIndexMap::clear()
{
    m_pool.clear();
    m_indexMap.clear();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ImplicitDirectoryCollector !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ImplicitDirectoryCollector::ImplicitDirectoryCollector(const String& canonicalPath, bool directoryExists) :
    m_directoryExists(directoryExists)
{
    StringBuilder buffer;
    if (canonicalPath != ".")
    {
        buffer << canonicalPath;
        buffer.append('/');
    }
    m_prefix = buffer.ProduceString();
}

void ImplicitDirectoryCollector::addRemainingPath(SlangPathType pathType, const UnownedStringSlice& inPathRemainder)
{
    // If it's zero length we probably don't want to add it
    if (inPathRemainder.getLength() == 0)
    {
        // It's empty so don't add normal way - implies the directory exists
        m_directoryExists = true;
        return;
    }

    UnownedStringSlice pathRemainder(inPathRemainder);
    const Index slashIndex = pathRemainder.indexOf('/');

    // If we have a following / that means it's an implicit directory.
    if (slashIndex >= 0)
    {
        pathType = SLANG_PATH_TYPE_DIRECTORY;
        pathRemainder = UnownedStringSlice(pathRemainder.begin(), pathRemainder.begin() + slashIndex);
    }

    const Index countIndex = m_map.findOrAdd(pathRemainder, pathType);
    SLANG_UNUSED(countIndex);
    // Make sure they are the same type
    SLANG_ASSERT(SlangPathType(m_map.getValueAt(countIndex)) == pathType);
}

void ImplicitDirectoryCollector::addPath(SlangPathType pathType, const UnownedStringSlice& canonicalPath)
{
    if (hasPrefix(canonicalPath))
    {
        UnownedStringSlice remainder = getRemainder(canonicalPath);
        addRemainingPath(pathType, remainder);
    }
}

SlangResult ImplicitDirectoryCollector::enumerate(FileSystemContentsCallBack callback, void* userData)
{
    const Int count = m_map.getCount();

    for (Index i = 0; i < count; ++i)
    {
        const auto& pair = m_map.getAt(i);

        UnownedStringSlice path = pair.Key;
        SlangPathType pathType = SlangPathType(pair.Value);

        // Note *is* 0 terminated in the pool
        // Let's check tho
        SLANG_ASSERT(path.begin()[path.getLength()] == 0);
        callback(pathType, path.begin(), userData);
    }

    return getDirectoryExists() ? SLANG_OK : SLANG_E_NOT_FOUND;
}

SlangResult loadArchiveFileSystem(const void* data, size_t dataSizeInBytes, RefPtr<ArchiveFileSystem>& outFileSystem)
{
    RefPtr<ArchiveFileSystem> fileSystem;
    if (ZipFileSystem::isArchive(data, dataSizeInBytes))
    {
        // It's a zip
        SLANG_RETURN_ON_FAIL(ZipFileSystem::create(fileSystem));
    }
    else if (RiffFileSystem::isArchive(data, dataSizeInBytes))
    {
        // It's riff contained (Slang specific)
       fileSystem = new RiffFileSystem(nullptr);
    }
    else
    {
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(fileSystem->loadArchive(data, dataSizeInBytes));

    outFileSystem = fileSystem;
    return SLANG_OK;
}
    
SlangResult createArchiveFileSystem(SlangArchiveType type, RefPtr<ArchiveFileSystem>& outFileSystem)
{
    switch (type)
    {
        case SLANG_ARCHIVE_TYPE_ZIP:
        {
            return ZipFileSystem::create(outFileSystem);
        }
        case SLANG_ARCHIVE_TYPE_RIFF:
        {
            outFileSystem = new RiffFileSystem(nullptr);
            return SLANG_OK;
        }
        case SLANG_ARCHIVE_TYPE_RIFF_DEFLATE:
        {
            outFileSystem = new RiffFileSystem(DeflateCompressionSystem::getSingleton());
            return SLANG_OK;
        }
        case SLANG_ARCHIVE_TYPE_RIFF_LZ4:
        {
            outFileSystem = new RiffFileSystem(LZ4CompressionSystem::getSingleton());
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

} // namespace Slang
