#ifndef SLANG_ARCHIVE_FILE_SYSTEM_H
#define SLANG_ARCHIVE_FILE_SYSTEM_H

#include "slang-basic.h"

#include "../../slang-com-ptr.h"

#include "slang-compression-system.h"

#include "slang-string-slice-pool.h"

namespace Slang
{

class ArchiveFileSystem : public RefObject, public ISlangMutableFileSystem
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


SlangResult loadArchiveFileSystem(const void* data, size_t dataSizeInBytes, RefPtr<ArchiveFileSystem>& outFileSystem);
SlangResult createArchiveFileSystem(SlangArchiveType type, RefPtr<ArchiveFileSystem>& outFileSystem);

}

#endif
