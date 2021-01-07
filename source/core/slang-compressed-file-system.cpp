#include "slang-compressed-file-system.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-io.h"
#include "slang-string-util.h"
#include "slang-blob.h"
#include "slang-string-slice-pool.h"
#include "slang-uint-set.h"

// Compression systems
#include "slang-deflate-compression-system.h"
#include "slang-lz4-compression-system.h"

// Zip file system
#include "slang-zip-file-system.h"

#include "slang-riff.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;
static const Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;
static const Guid IID_ISlangMutableFileSystem = SLANG_UUID_ISlangMutableFileSystem;

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
    // Make sure they are the same type
    SLANG_ASSERT(m_map.getValueAt(countIndex) == pathType);
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SimpleCompressedFileSystem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


RiffCompressedFileSystem::RiffCompressedFileSystem(ICompressionSystem* compressionSystem):
    m_compressionSystem(compressionSystem)
{
}

ISlangMutableFileSystem* RiffCompressedFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt || guid == IID_ISlangMutableFileSystem) ? static_cast<ISlangMutableFileSystem*>(this) : nullptr;
}

SlangResult RiffCompressedFileSystem::_calcCanonicalPath(const char* path, StringBuilder& out)
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

RiffCompressedFileSystem::Entry* RiffCompressedFileSystem::_getEntryFromCanonicalPath(const String& canonicalPath)
{
    RefPtr<Entry>* entryPtr = m_entries.TryGetValue(canonicalPath);
    return  entryPtr ? *entryPtr : nullptr;
}

RiffCompressedFileSystem::Entry* RiffCompressedFileSystem::_getEntryFromPath(const char* path, String* outPath)
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

SlangResult RiffCompressedFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
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

SlangResult RiffCompressedFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    return getCanonicalPath(path, outUniqueIdentity);
}

SlangResult RiffCompressedFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
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

SlangResult RiffCompressedFileSystem::getPathType(const char* path, SlangPathType* outPathType)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);
    if (entry == nullptr)
    {
        // Could be an implicit path
        ImplicitDirectoryCollector collector(canonicalPath);
        for (const auto& pair : m_entries)
        {
            Entry* childEntry = pair.Value;
            collector.addPath(childEntry->m_type, childEntry->m_canonicalPath.getUnownedSlice());
            // If on adding a path we determine a directory exists, then we are done
            if (collector.getDirectoryExists())
            {
                *outPathType = SLANG_PATH_TYPE_DIRECTORY;
                return SLANG_OK;
            }
        }

        // If not implicit or explicit we are done.
        return SLANG_E_NOT_FOUND;
    }

    // Explicit type
    *outPathType = entry->m_type;
    return SLANG_OK;
}

SlangResult RiffCompressedFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    String simplifiedPath = Path::simplify(path);
    *outSimplifiedPath = StringUtil::createStringBlob(simplifiedPath).detach();
    return SLANG_OK;
}

SlangResult RiffCompressedFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    StringBuilder buffer;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, buffer));
    *outCanonicalPath = StringUtil::createStringBlob(buffer).detach();
    return SLANG_OK;
}


SlangResult RiffCompressedFileSystem::enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);
    if (entry && entry->m_type != SLANG_PATH_TYPE_DIRECTORY)
    {
        return SLANG_FAIL;
    }

    // If we didn't find an explicit directory, lets handle an implicit one
    ImplicitDirectoryCollector collector(canonicalPath);

    // If it is a directory, we need to see if there is anything in it
    for (const auto& pair : m_entries)
    {
        Entry* childEntry = pair.Value;
        collector.addPath(childEntry->m_type, childEntry->m_canonicalPath.getUnownedSlice());
    }

    return collector.enumerate(callback, userData);
}

SlangResult RiffCompressedFileSystem::saveFile(const char* path, const void* data, size_t size)
{
    StringBuilder canonicalPath;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, canonicalPath));

    // Lets try compressing the input
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(m_compressionSystem->compress(&m_compressionStyle, data, size, blob.writeRef()));

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

SlangResult RiffCompressedFileSystem::remove(const char* path)
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

        ImplicitDirectoryCollector collector(canonicalPath);

        // If it is a directory, we need to see if there is anything in it
        for (const auto& pair : m_entries)
        {
            Entry* childEntry = pair.Value;
            collector.addPath(childEntry->m_type, childEntry->m_canonicalPath.getUnownedSlice());
            if (collector.hasContent())
            {
                // Directory is not empty
                return SLANG_FAIL;
            }
        }

        m_entries.Remove(canonicalPath);
        return SLANG_OK;
    }

    return SLANG_E_NOT_FOUND;
}

SlangResult RiffCompressedFileSystem::createDirectory(const char* path)
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




SlangResult RiffCompressedFileSystem::loadArchive(const void* archive, size_t archiveSizeInBytes)
{
    // Load the riff
    RiffContainer container;

    MemoryStreamBase stream(FileAccess::Read, archive, archiveSizeInBytes);
    SLANG_RETURN_ON_FAIL(RiffUtil::read(&stream, container));

    RiffContainer::ListChunk* rootList = container.getRoot();
    // Make sure it's the right type
    if (rootList == nullptr || rootList->m_fourCC != RiffFileSystemBinary::kContainerFourCC)
    {
        return SLANG_FAIL;
    }

    // Clear the contents
    _clear();

    // Find the header
    const auto header = rootList->findContainedData<RiffFileSystemBinary::Header>(RiffFileSystemBinary::kHeaderFourCC);

    CompressionSystemType compressionType = CompressionSystemType(header->compressionSystemType);
    switch (compressionType)
    {
        case CompressionSystemType::Deflate:
        {
            m_compressionSystem = DeflateCompressionSystem::getSingleton();
            break;
        }
        case CompressionSystemType::LZ4:
        {
            m_compressionSystem = LZ4CompressionSystem::getSingleton();
            break;
        }
        default: return SLANG_FAIL;
    }

    // Read all of the contained data

    {
        List<RiffContainer::DataChunk*> srcEntries;
        rootList->findContained(RiffFileSystemBinary::kEntryFourCC, srcEntries);

        for (auto chunk : srcEntries)
        {
            auto data = chunk->getSingleData();

            const uint8_t* srcData = (const uint8_t*)data->getPayload();
            const size_t dataSize = data->getSize();

            if (dataSize < sizeof(RiffFileSystemBinary::Entry))
            {
                return SLANG_FAIL;
            }

            auto srcEntry = (const RiffFileSystemBinary::Entry*)srcData;
            srcData += sizeof(*srcEntry);

            // Check if seems plausible
            if (sizeof(RiffFileSystemBinary::Entry) + srcEntry->compressedSize + srcEntry->pathSize != dataSize)
            {
                return SLANG_FAIL;
            }

            RefPtr<Entry> dstEntry = new Entry;

            const char* path = (const char*)srcData;
            srcData += srcEntry->pathSize;

            dstEntry->m_canonicalPath = UnownedStringSlice(path, srcEntry->pathSize - 1);
            dstEntry->m_type = (SlangPathType)srcEntry->pathType;
            dstEntry->m_uncompressedSizeInBytes = srcEntry->uncompressedSize;
            
            switch (dstEntry->m_type)
            {
                case SLANG_PATH_TYPE_FILE:
                {
                    if (srcData + srcEntry->compressedSize != data->getPayloadEnd())
                    {
                        return SLANG_FAIL;
                    }

                    // Get the compressed data
                    dstEntry->m_compressedData = new RawBlob(srcData, srcEntry->compressedSize);
                    break;
                }
                case SLANG_PATH_TYPE_DIRECTORY: break;
                default: return SLANG_FAIL;
            }

            // Add to the list of entries
            m_entries.Add(dstEntry->m_canonicalPath, dstEntry);
        }
    }

    return SLANG_OK;
}

SlangResult RiffCompressedFileSystem::storeArchive(bool blobOwnsContent, ISlangBlob** outBlob)
{
    // All blobs are owned in this style
    SLANG_UNUSED(blobOwnsContent)

    RiffContainer container;
    RiffContainer::ScopeChunk scopeContainer(&container, RiffContainer::Chunk::Kind::List, RiffFileSystemBinary::kContainerFourCC);

    {
        RiffFileSystemBinary::Header header;
        header.compressionSystemType = uint32_t(m_compressionSystem->getSystemType());
        container.addDataChunk(RiffFileSystemBinary::kHeaderFourCC, &header, sizeof(header));
    }

    for (const auto& pair : m_entries)
    {
        RiffContainer::ScopeChunk scopeData(&container, RiffContainer::Chunk::Kind::Data, RiffFileSystemBinary::kEntryFourCC);

        const Entry* srcEntry = pair.Value;

        RiffFileSystemBinary::Entry dstEntry;
        dstEntry.uncompressedSize = 0;
        dstEntry.compressedSize = 0;
        dstEntry.pathSize = uint32_t(srcEntry->m_canonicalPath.getLength() + 1);
        dstEntry.pathType = srcEntry->m_type;

        ISlangBlob* blob = srcEntry->m_compressedData;

        if (srcEntry->m_type == SLANG_PATH_TYPE_FILE)
        {
            dstEntry.compressedSize = uint32_t(blob->getBufferSize());
            dstEntry.uncompressedSize = uint32_t(srcEntry->m_uncompressedSizeInBytes);
        }

        // Entry header
        container.write(&dstEntry, sizeof(dstEntry));

        // Path
        container.write(srcEntry->m_canonicalPath.getBuffer(), srcEntry->m_canonicalPath.getLength() + 1);

        // Add the contained data without copying
        if (blob)
        {
            RiffContainer::Data* data = container.addData();
            container.setUnowned(data, const_cast<void*>(blob->getBufferPointer()), blob->getBufferSize());
        }
    }

    OwnedMemoryStream stream(FileAccess::Write);
    // We now write the RiffContainer to the stream
    SLANG_RETURN_ON_FAIL(RiffUtil::write(container.getRoot(), true, &stream));

    RefPtr<ListBlob> blob = new ListBlob;
    stream.swapContents(blob->m_data);

    *outBlob = blob.detach();
    return SLANG_OK;
}

SlangResult loadArchiveFileSystem(const void* data, size_t dataSizeInBytes, RefPtr<CompressedFileSystem>& outFileSystem)
{
    if (dataSizeInBytes < sizeof(FourCC))
    {
        return SLANG_FAIL;
    }

    FourCC fourCC = 0;
    ::memcpy(&fourCC, data, sizeof(FourCC));

    RefPtr<CompressedFileSystem> fileSystem;

    // https://en.wikipedia.org/wiki/List_of_file_signatures
    switch (fourCC)
    {
        case SLANG_FOUR_CC(0x50, 0x4B, 0x03, 0x04):
        case SLANG_FOUR_CC(0x50, 0x4B, 0x05, 0x06):
        case SLANG_FOUR_CC(0x50, 0x4B, 0x07, 0x08):
        {
            // It's a zip
            SLANG_RETURN_ON_FAIL(ZipFileSystem::create(fileSystem));
            break;
        }
        case RiffFourCC::kRiff:
        {
            MemoryStreamBase stream(FileAccess::Read, data, dataSizeInBytes);

            RiffListHeader header;
            SLANG_RETURN_ON_FAIL(RiffUtil::readHeader(&stream, header));

            if (header.subType != RiffFileSystemBinary::kContainerFourCC)
            {
                return SLANG_FAIL;
            }

            // It's riff contained (Slang specific)
            fileSystem = new RiffCompressedFileSystem(nullptr);
            break;
        }
        default: return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(fileSystem->loadArchive(data, dataSizeInBytes));

    outFileSystem = fileSystem;
    return SLANG_OK;
}


SlangResult createArchiveFileSystem(SlangArchiveType type, RefPtr<CompressedFileSystem>& outFileSystem)
{
    switch (type)
    {
        case SLANG_ARCHIVE_TYPE_ZIP:
        {
            return ZipFileSystem::create(outFileSystem);
        }
        case SLANG_ARCHIVE_TYPE_RIFF_DEFLATE:
        {
            outFileSystem = new RiffCompressedFileSystem(DeflateCompressionSystem::getSingleton());
            return SLANG_OK;
        }
        case SLANG_ARCHIVE_TYPE_RIFF_LZ4:
        {
            outFileSystem = new RiffCompressedFileSystem(LZ4CompressionSystem::getSingleton());
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

} // namespace Slang
