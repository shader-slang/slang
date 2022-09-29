#ifndef SLANG_RIFF_FILE_SYSTEM_H
#define SLANG_RIFF_FILE_SYSTEM_H

#include "slang-archive-file-system.h"

#include "slang-memory-file-system.h"

#include "slang-riff.h"
#include "slang-io.h"

namespace Slang
{

// The riff information used for RiffArchiveFileSystem
struct RiffFileSystemBinary
{
    static const FourCC kContainerFourCC = SLANG_FOUR_CC('S', 'c', 'o', 'n');
    static const FourCC kEntryFourCC = SLANG_FOUR_CC('S', 'f', 'i', 'l');
    static const FourCC kHeaderFourCC = SLANG_FOUR_CC('S', 'h', 'e', 'a');

    struct Header
    {
        uint32_t compressionSystemType;         /// One of CompressionSystemType
    };

    struct Entry
    {
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        uint32_t pathSize;                  ///< The size of the path in bytes, including terminating 0
        uint32_t pathType;                  ///< One of SlangPathType

        // Followed by the path (including terminating0)
        // Followed by the compressed data
    };
};

class RiffFileSystem : public MemoryFileSystem, public IArchiveFileSystem
{
public:
    typedef MemoryFileSystem Super;

    // ISlangUnknown 
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const* path, ISlangBlob** outBlob) SLANG_OVERRIDE;

    // ISlangModifyableFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveFile(const char* path, const void* data, size_t size) SLANG_OVERRIDE;
    
    // IArchiveFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadArchive(const void* archive, size_t archiveSizeInBytes) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL storeArchive(bool blobOwnsContent, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setCompressionStyle(const CompressionStyle& style) SLANG_OVERRIDE { m_compressionStyle = style; }

        /// Pass in nullptr, if no compression is wanted. 
    explicit RiffFileSystem(ICompressionSystem* compressionSystem);

        /// True if this appears to be Riff archive
    static bool isArchive(const void* data, size_t sizeInBytes);

protected:
    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    ComPtr<ICompressionSystem> m_compressionSystem;

    CompressionStyle m_compressionStyle;
};

}

#endif
