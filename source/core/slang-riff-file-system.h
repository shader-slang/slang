#ifndef SLANG_RIFF_FILE_SYSTEM_H
#define SLANG_RIFF_FILE_SYSTEM_H

#include "slang-archive-file-system.h"

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

class RiffFileSystem : public ISlangMutableFileSystem, public IArchiveFileSystem, public ComBaseObject
{
public:

    // ISlangUnknown 
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // ISlangFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const* path, ISlangBlob** outBlob) SLANG_OVERRIDE;

    // ISlangFileSystemExt
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getFileUniqueIdentity(const char* path, ISlangBlob** uniqueIdentityOut) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(const char* path, SlangPathType* pathTypeOut) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL clearCache() SLANG_OVERRIDE {}
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() SLANG_OVERRIDE { return OSPathKind::None; }

    // ISlangModifyableFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveFile(const char* path, const void* data, size_t size) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char* path) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char* path) SLANG_OVERRIDE;

    // ArchiveFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadArchive(const void* archive, size_t archiveSizeInBytes) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL storeArchive(bool blobOwnsContent, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setCompressionStyle(const CompressionStyle& style) SLANG_OVERRIDE { m_compressionStyle = style; }

        /// Pass in nullptr, if no compression is wanted. In that scenario the contents will be stored in memory as is
    explicit RiffFileSystem(ICompressionSystem* compressionSystem);

        /// True if this appears to be Riff archive
    static bool isArchive(const void* data, size_t sizeInBytes);

protected:

    struct Entry
    {
        void reset() 
        {
            m_type = SLANG_PATH_TYPE_FILE;
            m_canonicalPath = String();
            m_uncompressedSizeInBytes = 0;
            m_contents.setNull();
        }

        void initDirectory(const String& canonicalPath)
        {
            m_type = SLANG_PATH_TYPE_DIRECTORY;
            m_canonicalPath = canonicalPath;
            m_uncompressedSizeInBytes = 0;
            m_contents.setNull();
        }
        void initFile(const String& canonicalPath, size_t uncompressedSize, ISlangBlob* blob)
        {
            m_type = SLANG_PATH_TYPE_FILE;
            m_canonicalPath = canonicalPath;
            setContents(uncompressedSize, blob);
        }
        void setContents(size_t uncompressedSize, ISlangBlob* blob)
        {
            SLANG_ASSERT(m_type == SLANG_PATH_TYPE_FILE);
            SLANG_ASSERT(blob);
            m_uncompressedSizeInBytes = uncompressedSize;
            m_contents = blob;
        }

        SlangPathType m_type;
        String m_canonicalPath;
        size_t m_uncompressedSizeInBytes;       ///< Needed if m_contents is compressed.
        ComPtr<ISlangBlob> m_contents;          ///< Can be compressed or not
    };

    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    SlangResult _calcCanonicalPath(const char* path, StringBuilder& out);
    Entry* _getEntryFromPath(const char* path, String* outPath = nullptr);
    Entry* _getEntryFromCanonicalPath(const String& canonicalPath);

        /// Clear, ensures any backing memory is also freed
    void _clear() { m_entries = Dictionary<String, Entry>(); }

    // Maps a path to an entry
    Dictionary<String, Entry> m_entries;

    ComPtr<ICompressionSystem> m_compressionSystem;

    CompressionStyle m_compressionStyle;
};

}

#endif
