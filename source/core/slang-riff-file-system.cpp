#include "slang-riff-file-system.h"

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

namespace Slang
{

RiffFileSystem::RiffFileSystem(ICompressionSystem* compressionSystem):
    m_compressionSystem(compressionSystem)
{
}

ISlangMutableFileSystem* RiffFileSystem::getInterface(const Guid& guid)
{
    return (guid == ISlangUnknown::getTypeGuid() || guid == ISlangFileSystem::getTypeGuid() || guid == ISlangFileSystemExt::getTypeGuid() || guid == ISlangMutableFileSystem::getTypeGuid()) ? static_cast<ISlangMutableFileSystem*>(this) : nullptr;
}

SlangResult RiffFileSystem::_calcCanonicalPath(const char* path, StringBuilder& out)
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

RiffFileSystem::Entry* RiffFileSystem::_getEntryFromCanonicalPath(const String& canonicalPath)
{
    RefPtr<Entry>* entryPtr = m_entries.TryGetValue(canonicalPath);
    return  entryPtr ? *entryPtr : nullptr;
}

RiffFileSystem::Entry* RiffFileSystem::_getEntryFromPath(const char* path, String* outPath)
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

SlangResult RiffFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    Entry* entry = _getEntryFromPath(path);
    if (entry == nullptr || entry->m_type != SLANG_PATH_TYPE_FILE)
    {
        return SLANG_E_NOT_FOUND;
    }

    if (m_compressionSystem)
    {
        // Okay lets decompress into a blob
        ScopedAllocation alloc;
        void* dst = alloc.allocate(entry->m_uncompressedSizeInBytes);

        ISlangBlob* compressedData = entry->m_contents;
        SLANG_RETURN_ON_FAIL(m_compressionSystem->decompress(compressedData->getBufferPointer(), compressedData->getBufferSize(), entry->m_uncompressedSizeInBytes, dst)); 

        auto blob = RawBlob::moveCreate(alloc);

        *outBlob = blob.detach();
    }
    else
    {
        // We don't have any compression, so can just return the blob
        ISlangBlob* contents = entry->m_contents;
        contents->addRef();
        *outBlob = contents;
    }

    return SLANG_OK;
}

SlangResult RiffFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    return getCanonicalPath(path, outUniqueIdentity);
}

SlangResult RiffFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
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

SlangResult RiffFileSystem::getPathType(const char* path, SlangPathType* outPathType)
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

SlangResult RiffFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    String simplifiedPath = Path::simplify(path);
    *outSimplifiedPath = StringUtil::createStringBlob(simplifiedPath).detach();
    return SLANG_OK;
}

SlangResult RiffFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    StringBuilder buffer;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, buffer));
    *outCanonicalPath = StringUtil::createStringBlob(buffer).detach();
    return SLANG_OK;
}


SlangResult RiffFileSystem::enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData)
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

SlangResult RiffFileSystem::saveFile(const char* path, const void* data, size_t size)
{
    StringBuilder canonicalPath;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, canonicalPath));

    ComPtr<ISlangBlob> contents;

    if (m_compressionSystem)
    {
        // Lets try compressing the input
        SLANG_RETURN_ON_FAIL(m_compressionSystem->compress(&m_compressionStyle, data, size, contents.writeRef()));
    }
    else
    {
        // Just store the data directly.
        contents = RawBlob::create(data, size);
    }

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
    entry->m_contents = contents;

    return SLANG_OK;
}

SlangResult RiffFileSystem::remove(const char* path)
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

SlangResult RiffFileSystem::createDirectory(const char* path)
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

SlangResult RiffFileSystem::loadArchive(const void* archive, size_t archiveSizeInBytes)
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
        case CompressionSystemType::None:
        {
            // Null m_compressionSystem means no compression
            m_compressionSystem.setNull();
            break;
        }
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
                    dstEntry->m_contents = RawBlob::create(srcData, srcEntry->compressedSize);
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

SlangResult RiffFileSystem::storeArchive(bool blobOwnsContent, ISlangBlob** outBlob)
{
    // All blobs are owned in this style
    SLANG_UNUSED(blobOwnsContent)

    RiffContainer container;
    RiffContainer::ScopeChunk scopeContainer(&container, RiffContainer::Chunk::Kind::List, RiffFileSystemBinary::kContainerFourCC);

    {
        RiffFileSystemBinary::Header header;
        CompressionSystemType compressionSystemType = m_compressionSystem ? m_compressionSystem->getSystemType() : CompressionSystemType::None;
        header.compressionSystemType = uint32_t(compressionSystemType);
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

        ISlangBlob* blob = srcEntry->m_contents;

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

    List<uint8_t> data;
    stream.swapContents(data);

    *outBlob = ListBlob::moveCreate(data).detach();
    return SLANG_OK;
}

/* static */bool RiffFileSystem::isArchive(const void* data, size_t sizeInBytes)
{
    MemoryStreamBase stream(FileAccess::Read, data, sizeInBytes);
    RiffListHeader header;
    return SLANG_SUCCEEDED(RiffUtil::readHeader(&stream, header)) && header.subType == RiffFileSystemBinary::kContainerFourCC;
}

} // namespace Slang
