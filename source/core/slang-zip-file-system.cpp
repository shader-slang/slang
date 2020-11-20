#include "slang-zip-file-system.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-io.h"
#include "slang-string-util.h"
#include "slang-blob.h"
#include "slang-string-slice-pool.h"
#include "slang-uint-set.h"

// These match what is in amalgamate.sh - so we can just
// use the github repro with the appropriate tag (and submodules)

#ifdef _MSC_VER
// Disable warning on VS
// warning C4334: '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
#   pragma warning(disable:4334)
// warning C4100: : unreferenced formal parameter
#   pragma warning(disable:4100)
// warning C4127: conditional expression is constant
#   pragma warning(disable:4127)
#endif

#include "../../external/miniz/miniz.h"
#include "../../external/miniz/miniz_common.h"
#include "../../external/miniz/miniz_tdef.h"
#include "../../external/miniz/miniz_tinfl.h"
#include "../../external/miniz/miniz_zip.h"

#include "../../external/miniz/miniz.c"
#include "../../external/miniz/miniz_tdef.c"
#include "../../external/miniz/miniz_tinfl.c"
#include "../../external/miniz/miniz_zip.c"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangFileSystem = SLANG_UUID_ISlangFileSystem;
static const Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;
static const Guid IID_ISlangMutableFileSystem = SLANG_UUID_ISlangMutableFileSystem;

class ZipFileSystem : public RefObject, public ISlangMutableFileSystem
{
public:
    // ISlangUnknown 
   // override ref counting, as DefaultFileSystem is singleton
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

    ZipFileSystem();
    ~ZipFileSystem();

    SlangResult init(const uint8_t* archive, size_t size);

protected:

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
        Index get(const UnownedStringSlice& slice)
        {
            const Index poolIndex = m_pool.findIndex(slice);
            return (poolIndex >= 0) ? m_indexMap[poolIndex] : -1;
        }

        void clear()
        {
            m_pool.clear();
            m_indexMap.clear();
        }

        SubStringIndexMap():
            m_pool(StringSlicePool::Style::Empty)
        {
        }

        StringSlicePool m_pool;     ///< Pool holds the substrings
        List<Index> m_indexMap;     ///< Maps a pool index to the output index
    };

    enum class Mode
    {
        None,
        Read,
        Write,
    };

    SlangResult _requireMode(Mode mode);
        /// Do the mode change.
    SlangResult _requireModeImpl(Mode newMode);

    bool _hasArchive() { return m_mode != Mode::None; }
    SlangResult _getFixedPath(const char* path, String& outPath);
    SlangResult _findEntryIndex(const char* path, mz_uint& outIndex);
    SlangResult _findEntryIndexFromFixedPath(const String& fixedPath, mz_uint& outIndex);

    SlangResult _copyToAndInitWriter(mz_zip_archive& outWriter);

    void _rebuildMap();

        /// Returns true if the named item is at the index
    UnownedStringSlice _getPathAtIndex(Index index);

    ISlangMutableFileSystem* getInterface(const Guid& guid);

    SubStringIndexMap m_pathMap;
    // If bit is set (at the archive index) this index has been deleted.
    UIntSet m_removedSet;

    ScopedAllocation m_data;

    mz_uint m_compressionLevel = MZ_BEST_COMPRESSION;
    Mode m_mode = Mode::None;

    mz_zip_archive m_archive;           
};

ISlangMutableFileSystem* ZipFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt || guid == IID_ISlangMutableFileSystem) ? static_cast<ISlangMutableFileSystem*>(this) : nullptr;
}

ZipFileSystem::ZipFileSystem():
    m_mode(Mode::None)
{
}

 ZipFileSystem::~ZipFileSystem()
 {
     _requireMode(Mode::None);
 }

SlangResult ZipFileSystem::init(const uint8_t* archive, size_t size)
{
     SLANG_RETURN_ON_FAIL(_requireMode(Mode::None));

     // Store a copy
     if (!m_data.set(archive, size))
     {
         return SLANG_E_OUT_OF_MEMORY;
     }

     // Initialize archive
     mz_zip_zero_struct(&m_archive);

     // Read the contents of the archive
     if (!mz_zip_reader_init_mem(&m_archive, m_data.getData(), m_data.getSizeInBytes(), 0))
     {
         m_data.deallocate();
         return SLANG_FAIL;
     }

     m_mode = Mode::Read;

     // Set up the mapping from paths to indices
     _rebuildMap();

     return SLANG_OK;
}

void ZipFileSystem::_rebuildMap()
{
    m_pathMap.clear();

    const mz_uint entryCount = mz_zip_reader_get_num_files(&m_archive);

    m_removedSet.resizeAndClear(0);

    for (mz_uint i = 0; i < entryCount; ++i)
    {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&m_archive, mz_uint(i), &fileStat))
        {
            continue;
        }

        UnownedStringSlice currentName(fileStat.m_filename);

        // Get rid of '/'
        currentName = currentName.trim('/');

        m_pathMap.set(currentName, Index(i));
    }
}

UnownedStringSlice ZipFileSystem::_getPathAtIndex(Index index)
{
    SLANG_ASSERT(m_mode != Mode::None);

    mz_zip_archive_file_stat fileStat;
    // Check it's added at the end
    if (!mz_zip_reader_file_stat(&m_archive, mz_uint(index), &fileStat))
    {
        return UnownedStringSlice();
    }

    return UnownedStringSlice(fileStat.m_filename).trim('/');
}

SlangResult ZipFileSystem::_copyToAndInitWriter(mz_zip_archive& outWriter)
{
    mz_zip_zero_struct(&outWriter);
    switch (m_mode)
    {
        case Mode::None:
        {
            mz_zip_writer_init(&outWriter, 0);
            return SLANG_OK;
        }
        case Mode::Read:
        case Mode::Write:
        {
            mz_zip_writer_init(&outWriter, 0);

            const mz_uint entryCount = mz_zip_reader_get_num_files(&m_archive);

            for (mz_uint i = 0; i < entryCount; ++i)
            {
                if (m_removedSet.contains(i))
                {
                    continue;
                }

                // Hmm.. Not clear if this will work, because m_archive might not be a reader.
                // And on the other hand if it's a writer, it's not clear how to convert a writer to a reader *selectively* which
                // we require if we are going to lazily handle removals
                if (! mz_zip_writer_add_from_zip_reader(&outWriter, &m_archive, i))
                {
                    mz_zip_end(&outWriter);
                    return SLANG_FAIL;
                }
            }

            return SLANG_OK;
        }

        default: break;
    }
    return SLANG_FAIL;   
}

SlangResult ZipFileSystem::_requireModeImpl(Mode newMode)
{
    SLANG_ASSERT(newMode != m_mode);

    switch (m_mode)
    {
        case Mode::None:
        {
            switch (newMode)
            {
                case Mode::Read:
                {
                    mz_uint flags = 0;
                    mz_zip_zero_struct(&m_archive);
                    mz_zip_reader_init(&m_archive, 0, flags);
                    break;
                }
                case Mode::Write:
                {
                    mz_zip_zero_struct(&m_archive);
                    mz_zip_writer_init(&m_archive, 0);
                    break;
                }
                default: break;
            }
            break;
        }
        case Mode::Read:
        {
            switch (newMode)
            {
                case Mode::None:
                {
                    m_data.deallocate();
                    mz_zip_end(&m_archive);
                    break;
                }
                case Mode::Write:
                {
                    // If nothing is removed, we can just convert
                    if (m_removedSet.isEmpty())
                    {
                        // Convert the reader into the writer
                        if (!mz_zip_writer_init_from_reader(&m_archive, nullptr))
                        {
                            return SLANG_FAIL;
                        }
                    }
                    else
                    {
                        // Copy into a new writer
                        mz_zip_archive writer;
                        SLANG_RETURN_ON_FAIL(_copyToAndInitWriter(writer));

                        // In the process we have removed anything that was deleted
                        m_removedSet.clear();
                        // Don't need the read data anymore
                        m_data.deallocate();

                        // Free the current archive
                        mz_zip_end(&m_archive);
                        // Make the writer current
                        m_archive = writer;
                        break;
                    }
                    break;
                }
            }
            break;
        }
        case Mode::Write:
        {
            switch (newMode)
            {
                case Mode::None:
                {
                    mz_zip_writer_end(&m_archive);
                    break;
                }
                case Mode::Read:
                {
                    // If anything has been removed we copy selectively into a new writer, and then convert that
                    if (!m_removedSet.isEmpty())
                    {
                        // There are entries that are deleted... so we need to copy selectively
                        mz_zip_archive writer;
                        SLANG_RETURN_ON_FAIL(_copyToAndInitWriter(writer));

                        // In the process we have removed anything that was deleted
                        m_removedSet.clear();

                        // Get rid of the old writer
                        mz_zip_writer_end(&m_archive);
                        m_archive = writer;
                    }

                    void* buf;
                    size_t size;
                    mz_zip_writer_finalize_heap_archive(&m_archive, &buf, &size);
                    m_data.attach(buf, size);

                    mz_zip_writer_end(&m_archive);

                    // Read
                    mz_zip_zero_struct(&m_archive);
                    if (!mz_zip_reader_init_mem(&m_archive, m_data.getData(), m_data.getSizeInBytes(), 0))
                    {
                        m_data.deallocate();
                        return SLANG_FAIL;
                    }
                    break;
                }
                default: break;
            }
        }
    }

    // Set the new mode
    m_mode = newMode;
    return SLANG_OK;
}

SlangResult ZipFileSystem::_requireMode(Mode newMode)
{
    if (newMode == m_mode)
    {
        return SLANG_OK;
    }

    SlangResult res = _requireModeImpl(newMode); 
    if (SLANG_FAILED(res))
    {
        // If we failed torch the archive
        if (m_mode != Mode::None)
        {
            mz_zip_end(&m_archive);
            mz_zip_zero_struct(&m_archive);
        }

        // Set the mode to none
        m_mode = Mode::None;

        // Clear the map/s and data
        m_pathMap.clear();
        m_removedSet.clearAndDeallocate();
        m_data.deallocate();

        return res;
    }

    _rebuildMap();
    return SLANG_OK;
}

SlangResult ZipFileSystem::_getFixedPath(const char* path, String& outPath)
{
    String simplifiedPath = Path::simplify(UnownedStringSlice(path));
    // Can simplify to just ., thats okay, if it otherwise has something relative it means it couldn't be simplified into the
    // contents of the archive
    if (simplifiedPath != "." && Path::hasRelativeElement(simplifiedPath))
    {
        // If it still has a relative element, then it must be 'outside' of the archive
        return SLANG_E_NOT_FOUND;
    }

    outPath = simplifiedPath;
    return SLANG_OK;
}

SlangResult ZipFileSystem::_findEntryIndexFromFixedPath(const String& fixedPath, mz_uint& outIndex)
{
    const Index index = m_pathMap.get(fixedPath.getUnownedSlice());

    // If not in list or deleted - it is removed
    if (index < 0 || m_removedSet.contains(index))
    {
        return SLANG_E_NOT_FOUND;
    }

    outIndex = mz_uint(index);
    return SLANG_OK;
}

SlangResult ZipFileSystem::_findEntryIndex(const char* path, mz_uint& outIndex)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));
    SLANG_RETURN_ON_FAIL(_findEntryIndexFromFixedPath(fixedPath, outIndex));
    return SLANG_OK;
}

SlangResult ZipFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    mz_uint index;
    SLANG_RETURN_ON_FAIL(_findEntryIndex(path, index));

    // Check it's a file
    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&m_archive, index, &fileStat) || fileStat.m_is_directory)
    {
        return SLANG_E_NOT_FOUND;
    }

    ScopedAllocation alloc;
    if (alloc.allocate(fileStat.m_uncomp_size) == nullptr)
    {
        return SLANG_E_OUT_OF_MEMORY;
    }

    const mz_uint flags = 0;

    // Extract to memory
    if (!mz_zip_reader_extract_to_mem(&m_archive, index, alloc.getData(), alloc.getSizeInBytes(), flags))
    {
        return SLANG_FAIL;
    }

    *outBlob = RawBlob::moveCreate(alloc).detach();
    return SLANG_OK;
}

SlangResult ZipFileSystem::getPathType(const char* path, SlangPathType* outPathType)
{
    mz_uint index;
    SLANG_RETURN_ON_FAIL(_findEntryIndex(path, index));

    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&m_archive, index, &fileStat))
    {
        return SLANG_FAIL;
    }

    *outPathType = fileStat.m_is_directory ? SLANG_PATH_TYPE_DIRECTORY : SLANG_PATH_TYPE_FILE;
    return SLANG_OK;
}

SlangResult ZipFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    mz_uint index;
    SLANG_RETURN_ON_FAIL(_findEntryIndex(path, index));

    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&m_archive, index, &fileStat))
    {
        return SLANG_FAIL;
    }

    // Use the path in the archive itself
    *outCanonicalPath = StringUtil::createStringBlob(fileStat.m_filename).detach();
    return SLANG_OK;
}

SlangResult ZipFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    return getCanonicalPath(path, outUniqueIdentity);
}

SlangResult ZipFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
{
    String relPath;
    switch (fromPathType)
    {
        case SLANG_PATH_TYPE_FILE:
        {
            relPath = Path::combine(Path::getParentDirectory(fromPath), path);
            break;
        }
        case SLANG_PATH_TYPE_DIRECTORY:
        {
            relPath = Path::combine(fromPath, path);
            break;
        }
    }

    *pathOut = StringUtil::createStringBlob(relPath).detach();
    return SLANG_OK;
}

SlangResult ZipFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    *outSimplifiedPath = StringUtil::createStringBlob(Path::simplify(path)).detach();
    return SLANG_OK;
}

SlangResult ZipFileSystem::enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData)
{
    if (!_hasArchive())
    {
        return SLANG_E_NOT_FOUND;
    }

    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));

    if (fixedPath == ".")
    {
        fixedPath = "";
    }

    StringBuilder buf;

    // Okay - I want to iterate through all of the entries and look for the ones with this prefix
    const Index entryCount = Index(mz_zip_reader_get_num_files(&m_archive));
    for (Index i = 0; i < entryCount; ++i)
    {
        // Skip if it's been deleted.
        if (m_removedSet.contains(i))
        {
            continue;
        }

        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&m_archive, mz_uint(i), &fileStat))
        {
            continue;
        }

        UnownedStringSlice currentPath(fileStat.m_filename);
        if (!currentPath.startsWith(fixedPath.getUnownedSlice()))
        {
            continue;
        }

        UnownedStringSlice remaining(currentPath.begin() + fixedPath.getLength(), currentPath.end());

        // Trim delimiter from start and end
        remaining = remaining.trim('/').trim('\\');

        // If it contains a delimiter, then it must be in a child directory 
        if (remaining.indexOf('/') >= 0 || remaining.indexOf('\\') >= 0)
        {
            continue;
        }

        const SlangPathType pathType = fileStat.m_is_directory ? SLANG_PATH_TYPE_DIRECTORY : SLANG_PATH_TYPE_FILE;

        // We need zero termination
        buf.Clear();
        buf.append(remaining);

        callback(pathType, buf.getBuffer(), userData);
    }

    return SLANG_OK;
}

SlangResult ZipFileSystem::saveFile(const char* path, const void* data, size_t size)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));

    mz_uint32 index;
    if (SLANG_SUCCEEDED(_findEntryIndexFromFixedPath(fixedPath, index)))
    {
        // Mark as removed
        m_removedSet.add(index);
    }

    // We need to be able to write to the archive
    _requireMode(Mode::Write);

    // TODO(JS):
    // We need to check the directory exists that holds the path exists

    // Need to add to the end of the file
    const mz_uint32 entryCount = mz_zip_reader_get_num_files(&m_archive);
    if (!mz_zip_writer_add_mem(&m_archive, fixedPath.getBuffer(), data, size, m_compressionLevel))
    {
        return SLANG_FAIL;
    }

    // Make sure it is added at expended index
    SLANG_ASSERT(_getPathAtIndex(entryCount) == fixedPath.getUnownedSlice());

    // Set in the map
    m_pathMap.set(fixedPath.getUnownedSlice(), entryCount);
    return SLANG_OK;
}

SlangResult ZipFileSystem::remove(const char* path)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));

    mz_uint32 index;
    SLANG_RETURN_ON_FAIL(_findEntryIndexFromFixedPath(fixedPath, index));

    mz_zip_archive_file_stat fileStat;
    if (!mz_zip_reader_file_stat(&m_archive, index, &fileStat))
    {
        return SLANG_FAIL;
    }

    if (fileStat.m_is_directory)
    {
        // We need to see if there is anything in this directory
        fixedPath.appendChar('/');

        const mz_uint32 entryCount = mz_zip_reader_get_num_files(&m_archive);

        for (mz_uint32 i = 0; i < entryCount; ++i)
        {
            if (i == index || m_removedSet.contains(i))
            {
                continue;
            }

            if (!mz_zip_reader_file_stat(&m_archive, i, &fileStat))
            {
                return SLANG_FAIL;
            }

            // Does this start with 
            if (UnownedStringSlice(fileStat.m_filename).startsWith(fixedPath.getUnownedSlice()))
            {
                // If it contains children we can't remove it
                return SLANG_FAIL;
            }
        }
    }

    // Mark as removed
    m_removedSet.add(index);
    
    return SLANG_OK;
}

SlangResult ZipFileSystem::createDirectory(const char* path)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));

    // If we find something with this name, we can't create it
    mz_uint32 index;
    if (SLANG_SUCCEEDED(_findEntryIndexFromFixedPath(fixedPath, index)))
    {
        return SLANG_FAIL;
    }

    // Make writable
    SLANG_RETURN_ON_FAIL(_requireMode(Mode::Write));

    const mz_uint entryCount = mz_zip_reader_get_num_files(&m_archive);

    // The terminating / in the path indicates it's a directory
    {
        String dirPath(fixedPath);
        dirPath.appendChar('/');
        if (!mz_zip_writer_add_mem(&m_archive, dirPath.getBuffer(), nullptr, 0, m_compressionLevel))
        {
            return SLANG_FAIL;
        }
    }

    SLANG_ASSERT(_getPathAtIndex(entryCount) == fixedPath.getUnownedSlice());

    // Set the index, that we added at end
    m_pathMap.set(fixedPath.getUnownedSlice(), entryCount); 
    return SLANG_OK;
}

/* static */SlangResult ZipCompressionUtil::create(const void* data, size_t size, ComPtr<ISlangMutableFileSystem>& out)
{
    RefPtr<ZipFileSystem> fileSystem(new ZipFileSystem);
    SLANG_RETURN_ON_FAIL(fileSystem->init((const uint8_t*)data, size));

    out = fileSystem;
    return SLANG_OK;
}

/* static */void ZipCompressionUtil::unitTest()
{
    static const char input[] = "Hello world!";

    List<uint8_t> compressedInput;

    {
        const mz_ulong inputCount = mz_ulong(SLANG_COUNT_OF(input));

        const mz_ulong compressedInputBoundCount = mz_compressBound(inputCount);

        compressedInput.setCount(compressedInputBoundCount);

        mz_ulong compressedInputCount = 0;

        const int status = mz_compress(compressedInput.getBuffer(), &compressedInputCount, (const uint8_t*)input, inputCount);

        SLANG_ASSERT(status == MZ_OK);

        compressedInput.setCount(Index(compressedInputCount));
    }

    //SLANG_CHECK(_checkLines(UnownedStringSlice::fromLiteral(""), checkLines, SLANG_COUNT_OF(checkLines)));

#if 0
    List<char> output;
    {
        mz_deflateBound(
            const int status = mz_uncompress(pUncomp, &uncomp_len, compressedInput.getBuffer(), compressedInput.getCount());

        SLANG_CHECK(status == MZ_OK);
    }
#endif
}


} // namespace Slang
