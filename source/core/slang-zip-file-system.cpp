#include "slang-zip-file-system.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-io.h"
#include "slang-string-util.h"
#include "slang-blob.h"

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

    SlangResult init(const uint8_t* archive, size_t size);

protected:

    bool _hasArchive() { return m_data.getSizeInBytes() > 0; }
    SlangResult _getFixedPath(const char* path, String& outPath);
    SlangResult _findEntryIndex(const char* path, mz_uint& outIndex);

    ISlangMutableFileSystem* getInterface(const Guid& guid);

    ScopedAllocation m_data;
    mz_zip_archive m_archive;
};

ISlangMutableFileSystem* ZipFileSystem::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangFileSystem || guid == IID_ISlangFileSystemExt || guid == IID_ISlangMutableFileSystem) ? static_cast<ISlangMutableFileSystem*>(this) : nullptr;
}

ZipFileSystem::ZipFileSystem()
{
}

SlangResult ZipFileSystem::init(const uint8_t* archive, size_t size)
{
     // Store a copy
     if (!m_data.set(archive, size))
     {
         return SLANG_E_OUT_OF_MEMORY;
     }

     // Initialize archive
     memset(&m_archive, 0, sizeof(m_archive));
     // Read the contents of the archive
     if (!mz_zip_reader_init_mem(&m_archive, m_data.getData(), m_data.getSizeInBytes(), 0))
     {
         m_data.deallocate();
         return SLANG_FAIL;
     }

     return SLANG_OK;
}

SlangResult ZipFileSystem::_getFixedPath(const char* path, String& outPath)
{
    // If we have no archive we have no path conversion
    if (!_hasArchive())
    {
        return SLANG_FAIL;
    }

    String simplifiedPath = Path::simplify(UnownedStringSlice(path));
    if (Path::hasRelativeElement(simplifiedPath))
    {
        // If it still has a relative element, then it must be 'outside' of the archive
        return SLANG_E_NOT_FOUND;
    }

    outPath = simplifiedPath;
    return SLANG_OK;
}

SlangResult ZipFileSystem::_findEntryIndex(const char* path, mz_uint& outIndex)
{
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));

    mz_uint32 index;
    const mz_uint flags = 0;
    if (!mz_zip_reader_locate_file_v2(&m_archive, path, NULL, flags, &index))
    {
        return SLANG_E_NOT_FOUND;
    }

    outIndex = index;
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
    String fixedPath;
    SLANG_RETURN_ON_FAIL(_getFixedPath(path, fixedPath));

    StringBuilder buf;

    // Okay - I want to iterate through all of the entries and look for the ones with this prefix
    const Index entryCount = Index(mz_zip_reader_get_num_files(&m_archive));
    for (Index i = 0; i < entryCount; ++i)
    {
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
    SLANG_UNUSED(path);
    SLANG_UNUSED(data);
    SLANG_UNUSED(size);

    return SLANG_E_NOT_IMPLEMENTED;
}
SlangResult ZipFileSystem::remove(const char* path)
{
    SLANG_UNUSED(path);


    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult ZipFileSystem::createDirectory(const char* path)
{
    SLANG_UNUSED(path);

    return SLANG_E_NOT_IMPLEMENTED;
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
