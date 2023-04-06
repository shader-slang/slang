// slang-artifact-container-util.cpp
#include "slang-artifact-container-util.h"

#include "slang-artifact-util.h"
#include "slang-artifact-desc-util.h"

#include "../core/slang-file-system.h"
#include "../core/slang-io.h"
#include "../core/slang-zip-file-system.h"
#include "../core/slang-castable.h"

namespace Slang {

/* What is the structure we want here?

Lets say we have an artifact with "blah.spv". One approach would be to take the base name and so end up with

```
blah/
blah/blah.spv
blah/associated/...
blah/children/...
```

This is relatively simple and certainly pretty simple to implement. 

This doesn't hold additional information - such as what might be held in the ArtifactDesc. We could use .dsc 
to describe that. We could also remove the kernels name, as we already have that encoded in the directory name. 
So 

```
blah/
blah/bin.spv
blah/desc.json
blah/associated/...
blah/children/...
```

What happens if we have data that *doesn't* have a name? We can just produce names by incrementing some counter.

This works but seems a little weird, in that the name of the output kernel is not reflected. We could just use an extension
that we use for directories for this additional data. Note that it would be "nice" to make this work with just the basename,
but there is a problem with files that don't have an extension (as seen with executables on unix-like scenarios).

```
blah.spv
blah.artifact/
blah.artifact/desc.json
blah.artifact/associated
blah.artifact/... can just hold children directly
```
If we just hold children directly then their names could clash with standard names. 

We might want an extension such that we know it is an artifact. 

Other considerations

* Wanting to be able to easily combine containers (by hand if necessary in a file system)
  * Doing so implies we don't want some kind of global, or directory manifest.
* Wanting to easily be able to 

Given these constraints it seems like a good combination is

* Everything associated with an artifact is in a directory on it's own
* The name of the item should probably be stored as is in the directory
* Any ArtifactDesc or additional information should be held in a file in the directory
* Children and associated items should be stored in their own directories (which will follow the same mechanisms recursively)

```
blah/
blah/blah.spv
blah/desc.json
blah/associated/...
blah/children/...
```
*/

struct ArtifactContainerWriter
{
    struct Entry
    {
        String path;
        Index uniqueIndex = 0;
    };

    struct Scope
    {
        SlangResult pushAndRequireDirectory(ArtifactContainerWriter* writer, const String& name)
        {
            SLANG_ASSERT(writer);
            SLANG_ASSERT(m_writer == nullptr);

            SlangResult res = writer->pushAndRequireDirectory(name);

            if (SLANG_SUCCEEDED(res))
            {
                m_writer = writer;
            }

            return res;
        }

        ~Scope()
        {
            if (m_writer)
            {
                m_writer->pop();
            }
        }

        ArtifactContainerWriter* m_writer = nullptr;
    };

    void push(const String& name)
    {
        auto path = Path::combine(m_entry.path, name);

        m_entryStack.add(m_entry);

        Entry entry;
        entry.path = path;

        m_entry = entry;
    }
    SlangResult pushAndRequireDirectory(const String& name)
    {
        push(name);

        const char*const path = m_entry.path.getBuffer();

        SlangPathType pathType;
        if (SLANG_SUCCEEDED(m_fileSystem->getPathType(path, &pathType)))
        {
            if (pathType != SLANG_PATH_TYPE_DIRECTORY)
            {
                return SLANG_FAIL;
            }
        }
        
        // Make sure there is a path to this
        return m_fileSystem->createDirectory(m_entry.path.getBuffer());
    }
    void pop()
    {
        SLANG_ASSERT(m_entryStack.getCount() > 0);
        m_entry = m_entryStack.getLast();
        m_entryStack.removeLast();
    }

    SlangResult getBaseName(IArtifact* artifact, String& out);

        /// Write the artifact in the current scope
    SlangResult write(IArtifact* artifact);
    SlangResult writeInDirectory(IArtifact* artifact, const String& baseName);

    ArtifactContainerWriter(ISlangMutableFileSystem* fileSystem):
        m_fileSystem(fileSystem)
    {
    }

    List<Entry> m_entryStack;
    Entry m_entry;

    ISlangMutableFileSystem* m_fileSystem;
};

SlangResult ArtifactContainerWriter::getBaseName(IArtifact* artifact, String& out)
{
    String baseName;


    const auto artifactDesc = artifact->getDesc();

    {
        auto artifactName = artifact->getName();
        if (artifactName && artifactName[0] != 0)
        {
            baseName = ArtifactDescUtil::getBaseNameFromPath(artifactDesc, UnownedStringSlice(artifactName));
        }
    }

    // If we don't have name, use a generated one 
    if (baseName.getLength() == 0)
    {
        baseName.append(m_entry.uniqueIndex++);
    }

    out = baseName;
    return SLANG_OK;
}

SlangResult ArtifactContainerWriter::writeInDirectory(IArtifact* artifact, const String& baseName)
{
    
    // TODO(JS):
    // We could now output information about the desc/artifact, say as some json.
    // For now we assume the extension is good enough for most purposes.
    
    {
        // We can't write it without a blob
        ComPtr<ISlangBlob> blob;
        const auto res = artifact->loadBlob(ArtifactKeep::No, blob.writeRef());

        if (SLANG_FAILED(res))
        {
            // If it failed and it's significant the whole write fails
            if (ArtifactUtil::isSignificant(artifact))
            {
                return res;
            }
        }
        
        {
            // Get the name of the artifact
            StringBuilder artifactName;
            SLANG_RETURN_ON_FAIL(ArtifactDescUtil::calcNameForDesc(artifact->getDesc(), baseName.getUnownedSlice(), artifactName));

            const auto combinedPath = Path::combine(m_entry.path, artifactName);
            // Write out the blob
            SLANG_RETURN_ON_FAIL(m_fileSystem->saveFileBlob(combinedPath.getBuffer(), blob));
        }
    }

    {
        auto children = artifact->getChildren();
        if (children.count)
        {
            Scope childrenScope;
            SLANG_RETURN_ON_FAIL(childrenScope.pushAndRequireDirectory(this, "children"));

            for (IArtifact* child : children)
            {
                SLANG_RETURN_ON_FAIL(write(child));
            }
        }
    }
    {
        auto associatedSlice = artifact->getAssociated();
        if (associatedSlice.count)
        {
            Scope associatedScope;
            SLANG_RETURN_ON_FAIL(associatedScope.pushAndRequireDirectory(this, "associated"));

            for (IArtifact* associated : associatedSlice)
            {
                SLANG_RETURN_ON_FAIL(write(associated));
            }
        }
    }

    return SLANG_OK;
}

SlangResult ArtifactContainerWriter::write(IArtifact* artifact)
{
    String baseName;
    SLANG_RETURN_ON_FAIL(getBaseName(artifact, baseName));

    Scope artifactScope;
    SLANG_RETURN_ON_FAIL(artifactScope.pushAndRequireDirectory(this, baseName));

    SLANG_RETURN_ON_FAIL(writeInDirectory(artifact, baseName));

    return SLANG_OK;
}

/* static */SlangResult ArtifactContainerUtil::writeContainer(IArtifact* artifact, const String& defaultFileName, ISlangMutableFileSystem* fileSystem)
{
    ArtifactContainerWriter writer(fileSystem);

    String baseName;
    
    {
        const char* name = artifact->getName();
        if (name == nullptr || name[0] == 0)
        {
            // Try to get the name from the defaultFileName
            baseName = Path::getFileNameWithoutExt(defaultFileName);
        }
    }

    // If it's still not set try generating it.
    if (baseName.getLength() == 0)
    {
        SLANG_RETURN_ON_FAIL(writer.getBaseName(artifact, baseName));            
    }
    
    SLANG_RETURN_ON_FAIL(writer.writeInDirectory(artifact, baseName));

    return SLANG_OK;
}

/* static */SlangResult ArtifactContainerUtil::writeLegacy(IArtifact* artifact, const String& fileName)
{
    ComPtr<ISlangBlob> containerBlob;
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::Yes, containerBlob.writeRef()));

    {
        FileStream stream;
        SLANG_RETURN_ON_FAIL(stream.init(fileName, FileMode::Create, FileAccess::Write, FileShare::ReadWrite));
        SLANG_RETURN_ON_FAIL(stream.write(containerBlob->getBufferPointer(), containerBlob->getBufferSize()));
    }

    auto parentPath = Path::getParentDirectory(fileName);

    // Lets look to see if we have any maps
    {
        Index nameCount = 0;

        for (auto associatedArtifact : artifact->getAssociated())
        {
            auto desc = associatedArtifact->getDesc();

            if (isDerivedFrom(desc.payload, ArtifactPayload::SourceMap))
            {
                StringBuilder artifactFilename;

                // Dump out
                const char* artifactName = associatedArtifact->getName();
                if (artifactName && artifactName[0] != 0)
                {
                    SLANG_RETURN_ON_FAIL(ArtifactUtil::calcName(associatedArtifact, UnownedStringSlice(artifactName), artifactFilename));
                }
                else
                {
                    // Perhaps we can generate the name from the output basename
                    StringBuilder baseName;

                    baseName << Path::getFileNameWithoutExt(fileName);

                    if (nameCount != 0)
                    {
                        baseName.appendChar('-');
                        baseName.append(nameCount);
                    }

                    SLANG_RETURN_ON_FAIL(ArtifactUtil::calcName(associatedArtifact, baseName.getUnownedSlice(), artifactFilename));

                    nameCount ++;
                }

                ComPtr<ISlangBlob> blob;
                SLANG_RETURN_ON_FAIL(associatedArtifact->loadBlob(ArtifactKeep::No, blob.writeRef()));

                // Try to write it out
                {
                    // Work out the path to the artifact
                    auto artifactPath = Path::combine(parentPath, artifactFilename);

                    // Write out the map
                    FileStream stream;
                    SLANG_RETURN_ON_FAIL(stream.init(artifactPath, FileMode::Create, FileAccess::Write, FileShare::ReadWrite));
                    SLANG_RETURN_ON_FAIL(stream.write(blob->getBufferPointer(), blob->getBufferSize()));
                }
            }
        }
    }

    return SLANG_OK;
}

static SlangResult _remove(ISlangMutableFileSystem* fileSystem, const String& path)
{
    SlangPathType pathType;
    if (SLANG_SUCCEEDED(fileSystem->getPathType(path.getBuffer(), &pathType)))
    {
        fileSystem->remove(path.getBuffer());
    }
    return SLANG_OK;
}

/* static */SlangResult ArtifactContainerUtil::writeContainer(IArtifact* artifact, const String& fileName)
{
    auto osFileSystem = OSFileSystem::getMutableSingleton();

    const auto ext = Path::getPathExt(fileName);

    if (ext == "zip")
    {
        SLANG_RETURN_ON_FAIL(_remove(osFileSystem, fileName));
        // Create the zip

        ComPtr<ISlangMutableFileSystem> fileSystem;
        SLANG_RETURN_ON_FAIL(ZipFileSystem::create(fileSystem));

        // Write everything out
        SLANG_RETURN_ON_FAIL(writeContainer(artifact, fileName, fileSystem));

        // Now write out to the output file
        IArchiveFileSystem* archiveFileSystem = as<IArchiveFileSystem>(fileSystem);
        SLANG_ASSERT(archiveFileSystem);

        ComPtr<ISlangBlob> blob;
        SLANG_RETURN_ON_FAIL(archiveFileSystem->storeArchive(false, blob.writeRef()));

        // Okay we can now write out the zip
        SLANG_RETURN_ON_FAIL(osFileSystem->saveFileBlob(fileName.getBuffer(), blob));

        return SLANG_OK;
    }
    else if (ext == toSlice("dir"))
    {
        // We use the special extension "dir" to write out to a directory. 
        // This is a little hokey, but at list is consistent
        auto path = Path::getPathWithoutExt(fileName);

        SLANG_RETURN_ON_FAIL(_remove(osFileSystem, path));

        SLANG_RETURN_ON_FAIL(osFileSystem->createDirectory(path.getBuffer()));

        ComPtr<ISlangMutableFileSystem> fileSystem(new RelativeFileSystem(osFileSystem, path));

        SLANG_RETURN_ON_FAIL(writeContainer(artifact, fileName, fileSystem));
    }
    else
    {
        // This is the legacy way to write out the container artifact
        SLANG_RETURN_ON_FAIL(ArtifactContainerUtil::writeLegacy(artifact, fileName));
    }

    return SLANG_OK;
}

/* static */SlangResult ArtifactContainerUtil::readContainer(ISlangFileSystemExt* fileSystem, ComPtr<IArtifact>& outArtifact)
{
    SLANG_UNUSED(fileSystem);
    SLANG_UNUSED(outArtifact);

    // We traverse the file system creating the artifacts

    return SLANG_E_NOT_IMPLEMENTED;
}

} // namespace Slang
