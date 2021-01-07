#ifndef SLANG_COMPRESSED_FILE_SYSTEM_H
#define SLANG_COMPRESSED_FILE_SYSTEM_H

#include "slang-basic.h"

#include "../../slang-com-ptr.h"

#include "slang-riff.h"

#include "slang-compression-system.h"
#include "slang-io.h"

#include "slang-string-slice-pool.h"

namespace Slang
{

class CompressedFileSystem : public RefObject, public ISlangMutableFileSystem
{
public:

        /// Loads an archive. 
    virtual SlangResult loadArchive(const void* archive, size_t archiveSizeInBytes) = 0;
        /// Get as an archive (that can be saved to disk)
        /// NOTE! If the blob is not owned, it's contents can be invalidated by any call to a method of the file system or loss of scope
    virtual SlangResult storeArchive(bool blobOwnsContent, ISlangBlob** outBlob) = 0;
        /// Set the compression - used for any subsequent items added
    virtual void setCompressionStyle(const CompressionStyle& style) = 0;
};

// The riff information used for 
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

class RiffCompressedFileSystem : public CompressedFileSystem
{
public:

    // ISlangUnknown 
    SLANG_REF_OBJECT_IUNKNOWN_ALL

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

    // ISlangModifyableFileSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveFile(const char* path, const void* data, size_t size) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char* path) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char* path) SLANG_OVERRIDE;

    // CompressedFileSystem

    virtual SlangResult loadArchive(const void* archive, size_t archiveSizeInBytes) SLANG_OVERRIDE;
    virtual SlangResult storeArchive(bool blobOwnsContent, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual void setCompressionStyle(const CompressionStyle& style) SLANG_OVERRIDE { m_compressionStyle = style; }

    RiffCompressedFileSystem(ICompressionSystem* compressionSystem);

protected:

    struct Entry : RefObject
    {
        SlangPathType m_type;
        String m_canonicalPath;
        size_t m_uncompressedSizeInBytes;
        ComPtr<ISlangBlob> m_compressedData;
    };

    ISlangMutableFileSystem* getInterface(const Guid& guid);

    SlangResult _calcCanonicalPath(const char* path, StringBuilder& out);
    Entry* _getEntryFromPath(const char* path, String* outPath = nullptr);
    Entry* _getEntryFromCanonicalPath(const String& canonicalPath);

    void _clear() { m_entries.Clear(); }

    // Maps a path to an entry
    Dictionary<String, RefPtr<Entry>> m_entries;

    ComPtr<ICompressionSystem> m_compressionSystem;

    CompressionStyle m_compressionStyle;
};

/* Maps an UnownedStringSlice to an index. All substrings are held internally in a StringSlicePool, and so
owned by the type. */
class StringSliceIndexMap
{
public:
        /// An index that identifies a key value pair. 
    typedef Index CountIndex;

        /// Adds a key, value pair. Returns the CountIndex of the pair.
        /// If there is already a value stored for the key it is replaced.
    CountIndex add(const UnownedStringSlice& key, Index valueIndex);

        /// Finds or adds the slice. If the slice is added the defaultValueIndex is set.
        /// If not the index associated with the slice remains the same.
        /// Returns the CountIndex where the key,value pair are stored
    CountIndex findOrAdd(const UnownedStringSlice& key, Index defaultValueIndex);

        /// Gets the index associated with the key. Returns -1 if there is no associated index.
    SLANG_FORCE_INLINE Index getValue(const UnownedStringSlice& key);

        /// Get the amount of pairs in the map
    Index getCount() const { return m_indexMap.getCount(); }

        /// Get the slice and the index at the specified index
    SLANG_INLINE KeyValuePair<UnownedStringSlice, Index> getAt(CountIndex countIndex) const;
    
        /// Clear the contents of the map
    void clear();

        /// Get the key at the specified index
    UnownedStringSlice getKeyAt(CountIndex index) const { return m_pool.getSlice(StringSlicePool::Handle(index)); }
        /// Get the value at the specified index
    Index& getValueAt(CountIndex index) { return m_indexMap[index]; }

        /// Get the amount of key,value pairs
    Index getCount() { return m_indexMap.getCount(); }

        /// Ctor
    StringSliceIndexMap() :
        m_pool(StringSlicePool::Style::Empty)
    {
    }

protected:
    StringSlicePool m_pool;     ///< Pool holds the substrings
    List<Index> m_indexMap;     ///< Maps a pool index to the output index
};

// ---------------------------------------------------------------------------
Index StringSliceIndexMap::getValue(const UnownedStringSlice& key)
{
    const Index poolIndex = m_pool.findIndex(key);
    return (poolIndex >= 0) ? m_indexMap[poolIndex] : -1;
}

// ---------------------------------------------------------------------------
KeyValuePair<UnownedStringSlice, Index> StringSliceIndexMap::getAt(CountIndex countIndex) const
{
    KeyValuePair<UnownedStringSlice, Index> pair;
    pair.Key = m_pool.getSlice(StringSlicePool::Handle(countIndex));
    pair.Value = m_indexMap[countIndex];
    return pair;
}


/* This class helps to find the contents and/or existence of an implicit directory.This finds the contents of a directory.

This is achieved by using a path prefix that any contained path must at least match. If the remainder of the path contains a folder
 - detectable because it's not a leaf and so contains a delimiter - that directory is added. As a sub folder may contain many
 files, and the directory itself may also be defined, it is necessary to dedup. The deduping is handled by the StringSliceIndexMap. */
class ImplicitDirectoryCollector
{
public:
    
    enum class State
    {
        None,                   ///< Neither the directory or content have been found
        DirectoryExists,        ///< The directory exists
        HasContent,             ///< If it has content, the directory must exist
    };

        /// Get the current state
    State getState() const { return (m_map.getCount() > 0) ? State::HasContent : (m_directoryExists ? State::DirectoryExists : State::None); }
        /// True if collector at least has the specified state
    bool hasState(State state) { return Index(getState()) >= Index(state); }

        /// Set that it exists
    void setDirectoryExists(bool directoryExists) { m_directoryExists = directoryExists; }
        /// Get if it exists (implicitly or explicitly)
    bool getDirectoryExists() const { return m_directoryExists || m_map.getCount() > 0; }

        /// True if the path matches the prefix
    bool hasPrefix(const UnownedStringSlice& path) const { return path.startsWith(m_prefix.getUnownedSlice()); }

        /// True if the directory has content
    bool hasContent() const { return m_map.getCount() > 0; }

        /// Gets the remainder or path after the prefix
    UnownedStringSlice getRemainder(const UnownedStringSlice& path) const
    {
        SLANG_ASSERT(hasPrefix(path));
        return UnownedStringSlice(path.begin() + m_prefix.getLength(), path.end());
    }

        /// Add a remaining path
    void addRemainingPath(SlangPathType pathType, const UnownedStringSlice& inPathRemainder);
        /// Add a path
    void addPath(SlangPathType pathType, const UnownedStringSlice& canonicalPath);
        /// Enumerate the contents
    SlangResult enumerate(FileSystemContentsCallBack callback, void* userData);

        /// Ctor
    ImplicitDirectoryCollector(const String& canonicalPath, bool directoryExists = false);

    protected:
    StringSliceIndexMap m_map;
    String m_prefix;
    bool m_directoryExists;
};


SlangResult loadCompressedFileSystem(const void* data, size_t dataSizeInBytes, RefPtr<CompressedFileSystem>& outFileSystem);

enum CompressedFileSystemType
{
    Zip,
    RIFFDeflate,
    RIFFLZ4,
};

SlangResult createCompressedFileSystem(CompressedFileSystemType type, RefPtr<CompressedFileSystem>& outFileSystem);

}

#endif
