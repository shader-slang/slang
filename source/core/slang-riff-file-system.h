#ifndef SLANG_RIFF_FILE_SYSTEM_H
#define SLANG_RIFF_FILE_SYSTEM_H

#include "slang-archive-file-system.h"
#include "slang-memory-file-system.h"

#include "slang-riff.h"

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

        // Followed by the path (including terminating 0)
        // Followed by the compressed data
    };
};

/* RiffFileSystem is a FileSystem, that can save out it's contents (through the IArchiveFileSystem interface) to an archive

The 'RIFF' part provides the structure to store out the contents. The data is only accessed in the RIFF format when being 
read/written to an archive. Normal operations on the file system act in memory.

A RiffFileSystem also allows compression to be used on files. To use compression pass in a suitable ICompressionSystem 
implementation. If constructed without an ICompressionSystem, data is stored without compression. This is true of the in memory 
representation as well as in an archive. When compression is used, files 'contents' blob is actually the *compressed* version of the contents.
Calling loadFile/saveFile with uncompress/compress as need. If there is no compression contents is identical to the file contents.

NOTE:
* The RIFF chunk IDs are *slang specific*. It conforms to RIFF but is unlikely to be usable with other tooling.
* The RIFF chunk IDs are in RiffFileSystemBinary
*/
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
