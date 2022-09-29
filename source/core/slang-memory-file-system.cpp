#include "slang-memory-file-system.h"

// For Path::
#include "slang-io.h"
#include "slang-blob.h"

// For ImplicitDirectoryCollector
#include "slang-archive-file-system.h"

namespace Slang
{

void* MemoryFileSystem::getInterface(const Guid& guid)
{
    if  (   guid == ISlangUnknown::getTypeGuid() || 
            guid == ISlangCastable::getTypeGuid() || 
            guid == ISlangFileSystem::getTypeGuid() || 
            guid == ISlangFileSystemExt::getTypeGuid() || 
            guid == ISlangMutableFileSystem::getTypeGuid())
    {
        return static_cast<ISlangMutableFileSystem*>(this);
    }
    return nullptr;
}

void* MemoryFileSystem::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* MemoryFileSystem::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

SlangResult MemoryFileSystem::_calcCanonicalPath(const char* path, StringBuilder& out)
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

MemoryFileSystem::Entry* MemoryFileSystem::_getEntryFromCanonicalPath(const String& canonicalPath)
{
    return m_entries.TryGetValue(canonicalPath);
}

MemoryFileSystem::Entry* MemoryFileSystem::_getEntryFromPath(const char* path, String* outPath)
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

SlangResult MemoryFileSystem::_loadFile(const char* path, Entry** outEntry)
{
    *outEntry = nullptr;
    Entry* entry = _getEntryFromPath(path);
    if (entry == nullptr || entry->m_type != SLANG_PATH_TYPE_FILE)
    {
        return SLANG_E_NOT_FOUND;
    }
    *outEntry = entry;
    return SLANG_OK;
}

SlangResult MemoryFileSystem::loadFile(char const* path, ISlangBlob** outBlob)
{
    Entry* entry;
    SLANG_RETURN_ON_FAIL(_loadFile(path, &entry));
    
    ISlangBlob* contents = entry->m_contents;
    contents->addRef();
    *outBlob = contents;

    return SLANG_OK;
}

SlangResult MemoryFileSystem::getFileUniqueIdentity(const char* path, ISlangBlob** outUniqueIdentity)
{
    return getCanonicalPath(path, outUniqueIdentity);
}

SlangResult MemoryFileSystem::calcCombinedPath(SlangPathType fromPathType, const char* fromPath, const char* path, ISlangBlob** pathOut)
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

    *pathOut = StringBlob::moveCreate(combinedPath).detach();
    return SLANG_OK;
}

SlangResult MemoryFileSystem::getPathType(const char* path, SlangPathType* outPathType)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);
    if (entry == nullptr)
    {
        // Could be an implicit path
        ImplicitDirectoryCollector collector(canonicalPath);
        for (const auto& pair : m_entries)
        {
            const Entry* childEntry = &pair.Value;
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

SlangResult MemoryFileSystem::getSimplifiedPath(const char* path, ISlangBlob** outSimplifiedPath)
{
    String simplifiedPath = Path::simplify(path);
    *outSimplifiedPath = StringBlob::moveCreate(simplifiedPath).detach();
    return SLANG_OK;
}

SlangResult MemoryFileSystem::getCanonicalPath(const char* path, ISlangBlob** outCanonicalPath)
{
    StringBuilder buffer;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, buffer));
    *outCanonicalPath = StringBlob::moveCreate(buffer).detach();
    return SLANG_OK;
}

SlangResult MemoryFileSystem::enumeratePathContents(const char* path, FileSystemContentsCallBack callback, void* userData)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);

    const bool foundDirectory = (entry && entry->m_type == SLANG_PATH_TYPE_DIRECTORY);

    // We allow implicit directories, so this works even if there isn't an explicit one
    ImplicitDirectoryCollector collector(canonicalPath, foundDirectory);

    // If it is a directory, we need to see if there is anything in it
    for (const auto& pair : m_entries)
    {
        const Entry* childEntry = &pair.Value;
        collector.addPath(childEntry->m_type, childEntry->m_canonicalPath.getUnownedSlice());
    }

    return collector.enumerate(callback, userData);
}

SlangResult MemoryFileSystem::saveFile(const char* path, const void* data, size_t size)
{
    Entry* entry;
    SLANG_RETURN_ON_FAIL(_requireFile(path, &entry));
    auto contents = RawBlob::create(data, size);
    entry->setContents(size, contents);
    return SLANG_OK;
}

SlangResult MemoryFileSystem::_requireFile(const char* path, Entry** outEntry)
{
    *outEntry = nullptr;

    StringBuilder canonicalPath;
    SLANG_RETURN_ON_FAIL(_calcCanonicalPath(path, canonicalPath));

    Entry* foundEntry = _getEntryFromCanonicalPath(canonicalPath);

    if (foundEntry)
    {
        if (foundEntry->m_type != SLANG_PATH_TYPE_FILE)
        {
            // Can only set if it's already a file, if it's anything else it's an error
            return SLANG_FAIL;
        }
    }
    else
    {
        Entry entry;
        entry.initFile(canonicalPath);
        m_entries.Add(canonicalPath, entry);

        foundEntry = _getEntryFromCanonicalPath(canonicalPath);
    }

    // It must be found and be a file
    SLANG_ASSERT(foundEntry && foundEntry->m_type == SLANG_PATH_TYPE_FILE && foundEntry->m_canonicalPath == canonicalPath);

    *outEntry = foundEntry;
    return SLANG_OK;
}

SlangResult MemoryFileSystem::remove(const char* path)
{
    String canonicalPath;
    Entry* entry = _getEntryFromPath(path, &canonicalPath);

    if (entry)
    {
        if (entry->m_type == SLANG_PATH_TYPE_DIRECTORY)
        {
            ImplicitDirectoryCollector collector(canonicalPath);

            // If it is a directory, we need to see if there is anything in it
            for (const auto& pair : m_entries)
            {
                const Entry* childEntry = &pair.Value;
                collector.addPath(childEntry->m_type, childEntry->m_canonicalPath.getUnownedSlice());
                if (collector.hasContent())
                {
                    // Directory is not empty
                    return SLANG_FAIL;
                }
            }
        }

        // Reset so doesn't hold references/keep memory in scope
        entry->reset();
        m_entries.Remove(canonicalPath);
        return SLANG_OK;
    }

    return SLANG_E_NOT_FOUND;
}

SlangResult MemoryFileSystem::createDirectory(const char* path)
{
    String canonicalPath;
    if (_getEntryFromPath(path, &canonicalPath))
    {
        return SLANG_FAIL;
    }

    Entry entry;
    entry.initDirectory(canonicalPath);
    m_entries.Add(canonicalPath, entry);
    return SLANG_OK;
}

} // namespace Slang
