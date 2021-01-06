#ifndef SLANG_COMPRESSED_FILE_SYSTEM_H
#define SLANG_COMPRESSED_FILE_SYSTEM_H

#include "slang-basic.h"

#include "../../slang-com-ptr.h"

#include "slang-compression-system.h"
#include "slang-io.h"

#include "slang-string-slice-pool.h"

namespace Slang
{

class CompressedFileSystem : public RefObject, public ISlangMutableFileSystem
{
public:
        /// Get as an archive (that can be saved to disk)
    virtual ConstArrayView<uint8_t> getArchive() = 0;
        /// Set the compression - used for any subsequent items added
    virtual void setCompressionStyle(const CompressionStyle& style) = 0;
};

class SimpleCompressedFileSystem : public CompressedFileSystem
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

    SimpleCompressedFileSystem(ICompressionSystem* compressionSystem);

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

    // Maps a path to an entry
    Dictionary<String, RefPtr<Entry>> m_entries;

    ComPtr<ICompressionSystem> m_compressionSystem;
};

/// Maps a SubString (owned) to an index
struct SubStringIndexMap
{
    void set(const UnownedStringSlice& slice, Index index)
    {
        StringSlicePool::Handle handle;
        m_pool.findOrAdd(slice, handle);
        const Index poolIndex = StringSlicePool::asIndex(handle);

        if (poolIndex >= m_indexMap.getCount())
        {
            SLANG_ASSERT(poolIndex == m_indexMap.getCount());
            m_indexMap.add(index);
        }
        else
        {
            m_indexMap[poolIndex] = index;
        }
    }
    Index findOrAdd(const UnownedStringSlice& slice, Index indexForSlice)
    {
        StringSlicePool::Handle handle;
        m_pool.findOrAdd(slice, handle);
        const Index poolIndex = StringSlicePool::asIndex(handle);

        if (poolIndex >= m_indexMap.getCount())
        {
            SLANG_ASSERT(poolIndex == m_indexMap.getCount());
            m_indexMap.add(indexForSlice);
        }
        return poolIndex;
    }

    Index get(const UnownedStringSlice& slice)
    {
        const Index poolIndex = m_pool.findIndex(slice);
        return (poolIndex >= 0) ? m_indexMap[poolIndex] : -1;
    }

    Index getCount() const { return m_indexMap.getCount(); }

    KeyValuePair<UnownedStringSlice, Index> getAt(Index index) const
    {
        KeyValuePair<UnownedStringSlice, Index> pair;
        pair.Key = m_pool.getSlice(StringSlicePool::Handle(index));
        pair.Value = m_indexMap[index];
        return pair;
    }

    void clear()
    {
        m_pool.clear();
        m_indexMap.clear();
    }

    UnownedStringSlice getSliceAt(Index index) { return m_pool.getSlice(StringSlicePool::Handle(index)); }
    Index& getIndexAt(Index index) { return m_indexMap[index]; }

    Index getCount() { return m_indexMap.getCount(); }

    SubStringIndexMap() :
        m_pool(StringSlicePool::Style::Empty)
    {
    }

    StringSlicePool m_pool;     ///< Pool holds the substrings
    List<Index> m_indexMap;     ///< Maps a pool index to the output index
};

/* This finds the contents of a directory. It's somewhat complicated because it allows for implicit paths
defined by the existence of a directory in a path */

struct ImplicitDirectoryCollector
{
    ImplicitDirectoryCollector(const String& canonicalPath, bool directoryExists = false):
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

    enum class State
    {
        Undefined,                ///< 
        DirectoryExists,        ///< The directory exists
        HasContent,             ///< If it has content, the directory must exist
    };

    State getState() const { return (m_map.getCount() > 0) ? State::HasContent : (m_directoryExists ? State::DirectoryExists : State::Undefined); }
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
    void addRemainingPath(SlangPathType pathType, const UnownedStringSlice& inPathRemainder)
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

        const Index index = m_map.findOrAdd(pathRemainder, pathType);
        // Make sure they are the same type
        SLANG_ASSERT(pathMap.getIndexAt(Index) == pathType);
    }

        /// Add a path
    void addPath(SlangPathType pathType, const UnownedStringSlice& canonicalPath)
    {
        if (hasPrefix(canonicalPath))
        {
            UnownedStringSlice remainder = getRemainder(canonicalPath);
            addRemainingPath(pathType, remainder);
        }
    }

        /// Enumerate the contents
    SlangResult enumerate(FileSystemContentsCallBack callback, void* userData)
    {
        const Int count = m_map.getCount(); 

        for (Index i = 0; i < count; ++i)
        {
            const auto& pair = m_map.getAt(i);

            UnownedStringSlice path = pair.Key;
            SlangPathType pathType = SlangPathType(pair.Value);

            // Note *is* 0 terminated in the pool
            // Let's check tho
            SLANG_ASSERT(path.begin()[path.getCount()] == 0);
            callback(pathType, path.begin(), userData);
        }

        return getDirectoryExists() ? SLANG_OK : SLANG_E_NOT_FOUND;
    }

    SubStringIndexMap m_map;
    String m_prefix;
    bool m_directoryExists;
};

}

#endif
