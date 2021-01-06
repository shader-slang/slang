#include "slang-compressed-file-system.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-io.h"
#include "slang-string-util.h"
#include "slang-blob.h"
#include "slang-string-slice-pool.h"
#include "slang-uint-set.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;
static const Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;
static const Guid IID_ISlangMutableFileSystem = SLANG_UUID_ISlangMutableFileSystem;

SimpleCompressedFileSystem::SimpleCompressedFileSystem(ICompressionSystem* compressionSystem):
    m_compressionSystem(compressionSystem)
{
}

ISlangMutableFileSystem* SimpleCompressedFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt || guid == IID_ISlangMutableFileSystem) ? static_cast<ISlangMutableFileSystem*>(this) : nullptr;
}

SlangResult SimpleCompressedFileSystem::_calcCanonicalPath(const char* path, StringBuilder& out)
{
    List<UnownedStringSlice> splitPath;
    Path::split(UnownedStringSlice(path), splitPath);

    // If the first part of a path is "", it means path of form "/some/path". Turn into "some/path".
    if (splitPath.getCount() > 1 && splitPath[0].getLength() == 0)
    {
        splitPath.removeAt(0);
    }

    Path::simplify(splitPath);

    if (splitPath.indexOf(UnownedStringSlice::fromLiteral("..")) >= 0)
    {
        return SLANG_E_NOT_FOUND;
    }

    if (splitPath.getCount() == 0)
    {
        // It's an empty path;
        return SLANG_FAIL;
    }

    Path::join(splitPath.getBuffer(), splitPath.getCount(), out);
    return SLANG_OK;
}

SimpleCompressedFileSystem::Entry* SimpleCompressedFileSystem::_getEntryFromCanonicalPath(const String& canonicalPath)
{
    RefPtr<Entry>* entryPtr = m_entries.TryGetValue(canonicalPath);
    return  entryPtr ? *entryPtr : nullptr;
}

SimpleCompressedFileSystem::Entry* SimpleCompressedFileSystem::_getEntryFromPath(const char* path, String* outPath)
{
    StringBuilder buffer;
    if (SLANG_FAILED(_calcCanonicalPath(path, buffer)))
    {
        return nullptr;
    }

    if (outPath)
    {
        *outPath = buffer;
    }
    return _getEntryFromCanonicalPath(buffer);
}

SlangResult SimpleCompressedFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    Entry* entry = _getEntryFromPath(path);
    if (entry == nullptr || entry->m_type != SLANG_PATH_TYPE_FILE)
    {
        return SLANG_E_NOT_FOUND;
    }

    // OKay lets decompress into a blob
    ScopedAllocation alloc;
    void* dst = alloc.allocate(entry->m_uncompressedSizeInBytes);

    ISlangBlob* compressedData = entry->m_compressedData;
    SLANG_RETURN_ON_FAIL(m_compressionSystem->decompress(compressedData->getBufferPointer(), compressedData->getBufferSize(), entry->m_uncompressedSizeInBytes, dst)); 

    auto blob = RawBlob::moveCreate(alloc);

    *outBlob = blob.detach();
    return SLANG_OK;
}

SlangResult SimpleCompressedFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    return getCanonicalPath(path, outUniqueIdentity);
}

SlangResult SimpleCompressedFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    String combinedPath;
    switch (fromPathType)
    {
        case SLANG_PATH_TYPE_FILE:
        {
            combinedPath = Path::combine(Path::getParentDirectory(fromPath), path);
            break;
        }
        case SLANG_PATH_TYPE_DIRECTORY:
        {
            combinedPath = Path::combine(fromPath, path);
            break;
        }
    }

    *pathOut = StringUtil::createStringBlob(combinedPath).detach();
    return SLANG_OK;
}

SlangResult SimpleCompressedFileSystem::getPathType(const char* path, SlangPathType* outPathType)
{
    Entry* entry = _getEntryFromPath(path);
    if (entry == nullptr)
    {
        return SLANG_E_NOT_FOUND;
    }
    *outPathType = entry->m_type;
    return SLANG_OK;
}

SlangResult SimpleCompressedFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    String simplifiedPath = Path::simplify(path);
    *outSimplifiedPath = StringUtil::createStringBlob(simplifiedPath).detach();
    return SLANG_OK;
}

SlangResult SimpleCompressedFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    StringBuilder buffer;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, buffer));
    *outCanonicalPath = StringUtil::createStringBlob(buffer).detach();
    return SLANG_OK;
}


SlangResult SimpleCompressedFileSystem::enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);
    if (entry && entry->m_type != SLANG_PATH_TYPE_DIRECTORY)
    {
        return SLANG_FAIL;
    }

    // If we didn't find an explicit directory, lets handle an implicit one
    SubStringIndexMap map;
    String prefixPath = ImplicitDirectoryUtil::getPathPrefix(canonicalPath.getUnownedSlice());

    // If it is a directory, we need to see if there is anything in it
    for (const auto& pair : m_entries)
    {
        const String& entryCanonicalPath = pair.Key;
        if (entryCanonicalPath.startsWith(prefixPath))
        {
            // Directory is not empty;
            return SLANG_FAIL;
        }

        Entry* childEntry = pair.Value;
        ImplicitDirectoryUtil::addPath(childEntry->m_type, prefixPath, pair.Key.getUnownedSlice(), map);
    }

    ImplicitDirectoryUtil::enumerate(map, callback, userData);

    return (map.getCount() == 0) ? SLANG_E_NOT_FOUND : SLANG_OK;
}

SlangResult SimpleCompressedFileSystem::saveFile(const char* path, const void* data, size_t size)
{
    StringBuilder canonicalPath;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, canonicalPath));

    CompressionStyle style;

    // Lets try compressing the input
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(m_compressionSystem->compress(&style, data, size, blob.writeRef()));

    Entry* entry = _getEntryFromCanonicalPath(canonicalPath);
    if (!entry)
    {
        entry = new Entry;
        entry->m_type = SLANG_PATH_TYPE_FILE;
        entry->m_canonicalPath = canonicalPath;
        entry->m_uncompressedSizeInBytes = size;

        m_entries.Add(canonicalPath, entry);
    }

    entry->m_uncompressedSizeInBytes = size;
    entry->m_compressedData = blob;

    return SLANG_OK;
}

SlangResult SimpleCompressedFileSystem::remove(const char* path)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);

    if (entry)
    {
        if (entry->m_type == SLANG_PATH_TYPE_FILE)
        {
            m_entries.Remove(canonicalPath);
            return SLANG_OK;
        }

        // It's a directory - make sure it's empty

        // Add the / at the end
        StringBuilder prefix;
        prefix.append(canonicalPath);
        prefix.append('/');

        // If it is a directory, we need to see if there is anything in it
        for (const auto& pair : m_entries)
        {
            const String& entryCanonicalPath = pair.Key;
            if (entryCanonicalPath.startsWith(prefix))
            {
                // Directory is not empty;
                return SLANG_FAIL;
            }
        }

        m_entries.Remove(canonicalPath);
        return SLANG_OK;
    }

    return SLANG_E_NOT_FOUND;
}

SlangResult SimpleCompressedFileSystem::createDirectory(const char* path)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);
    if (entry)
    {
        return SLANG_FAIL;
    }

    entry = new Entry;
    entry->m_type = SLANG_PATH_TYPE_DIRECTORY;
    entry->m_canonicalPath = canonicalPath;
    entry->m_uncompressedSizeInBytes = 0;

    m_entries.Add(canonicalPath, entry);
    return SLANG_OK;
}

} // namespace Slang
