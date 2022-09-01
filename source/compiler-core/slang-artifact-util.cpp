// slang-artifact.cpp
#include "slang-artifact-util.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"

#include "slang-artifact-desc-util.h"

#include "../core/slang-io.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactUtil !!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */ComPtr<IArtifactContainer> ArtifactUtil::createContainer(const ArtifactDesc& desc)
{
    const auto containerDesc = ArtifactDesc::make(ArtifactKind::Container, ArtifactPayload::CompileResults, desc.style);
    return ArtifactContainer::create(containerDesc);
}

/* static */ComPtr<IArtifactContainer> ArtifactUtil::createResultsContainer()
{
    return ArtifactContainer::create(ArtifactDesc::make(ArtifactKind::Container, ArtifactPayload::CompileResults));
}

/* static */ComPtr<IArtifact> ArtifactUtil::createArtifact(const ArtifactDesc& desc, const char* name)
{
    auto artifact = createArtifact(desc);
    artifact->setName(name);
    return artifact;
}

/* static */ComPtr<IArtifact> ArtifactUtil::createArtifact(const ArtifactDesc& desc)
{
    if (isDerivedFrom(desc.kind, ArtifactKind::Container))
    {
        auto container = ArtifactContainer::create(desc);

        ComPtr<IArtifact> artifact;
        artifact.attach(container.detach());
        return artifact;
    }
    else
    {
        return Artifact::create(desc);
    }
}

/* static */ComPtr<IArtifact> ArtifactUtil::createArtifactForCompileTarget(SlangCompileTarget target)
{
    auto desc = ArtifactDescUtil::makeDescForCompileTarget(target);
    return createArtifact(desc);
}

/* static */bool ArtifactUtil::isSignificant(IArtifact* artifact, void* data)
{
    SLANG_UNUSED(data);

    const auto desc = artifact->getDesc();

    // Containers are not significant as of themselves, they may contain something tho
    if (isDerivedFrom(desc.kind, ArtifactKind::Container))
    {
        return false;
    }

    // If it has no payload.. we are done
    if (desc.payload == ArtifactPayload::None ||
        desc.payload == ArtifactPayload::Invalid)
    {
        return false;
    }

    // If it's binary like or assembly/source we it's significant
    if (isDerivedFrom(desc.kind, ArtifactKind::CompileBinary) ||
        desc.kind == ArtifactKind::Assembly ||
        desc.kind == ArtifactKind::Source)
    {
        return true;
    }

    /* Hmm, we might want to have a base class for 'signifiant' payloads,
    where signifiance here means somewhat approximately 'the meat' of a compilation result,
    as contrasted with 'meta data', 'diagnostics etc'*/
    if (isDerivedFrom(desc.payload, ArtifactPayload::Metadata))
    {
        return false;
    }

    return true;
}

/* static */IArtifact* ArtifactUtil::findSignificant(IArtifact* artifact) 
{ 
    return artifact->findArtifactByPredicate(IArtifact::FindStyle::SelfOrChildren, &ArtifactUtil::isSignificant, nullptr);
}

/* static */String ArtifactUtil::getBaseName(IArtifact* artifact)
{
    if (auto fileRep = findRepresentation<IFileArtifactRepresentation>(artifact))
    {
        return ArtifactDescUtil::getBaseName(artifact->getDesc(), fileRep);
    }
    // Else use the name
    return artifact->getName();
}

/* static */String ArtifactUtil::getParentPath(IFileArtifactRepresentation* fileRep)
{
    UnownedStringSlice path(fileRep->getPath());
    return Path::getParentDirectory(path);
}

/* static */String ArtifactUtil::getParentPath(IArtifact* artifact)
{
    if (auto fileRep = findRepresentation<IFileArtifactRepresentation>(artifact))
    {
        return getParentPath(fileRep);
    }
    return String();
}

/* static */IFileArtifactRepresentation* ArtifactUtil::findFileSystemTemporaryFile(IArtifact* artifact)
{
    if (auto fileRep = findFileSystemFile(artifact))
    {
        return fileRep->getLockFile() ? fileRep : nullptr;
    }
    return nullptr;
}

/* static */IFileArtifactRepresentation* ArtifactUtil::findFileSystemFile(IArtifact* artifact)
{
    for (auto rep : artifact->getRepresentations())
    {
        if (auto fileSystemRep = as<IFileArtifactRepresentation>(rep))
        {
            if (fileSystemRep->getFileSystem() == nullptr)
            {
                return fileSystemRep;
            }
        }
    }
    return nullptr;
}

/* static */IFileArtifactRepresentation* ArtifactUtil::findFileSystemPrimaryFile(IArtifact* artifact)
{
    for (auto rep : artifact->getRepresentations())
    {
        if (auto fileRep = as<IFileArtifactRepresentation>(rep))
        {
            // If it has a file system it's not on OS 
            // If it has a lock file it can be assumed to be temporary
            if (fileRep->getFileSystem() != nullptr ||
                fileRep->getLockFile())
            {
                continue;
            }

            // If it's a file that is persistant it will just be a reference to a pre-existing file
            const auto kind = fileRep->getKind();
            if (kind != IFileArtifactRepresentation::Kind::Reference)
            {
                continue;
            }

            return fileRep;
        }
    }
    return nullptr;
}

UnownedStringSlice ArtifactUtil::findPath(IArtifact* artifact)
{
    // If a name is set we'll just use that
    {
        const char* name = artifact->getName();
        if (name && name[0] != 0)
        {
            return UnownedStringSlice(name);
        }
    }

    // Find the *first* file rep and use it's path. 
    // This may not be the file on the file system - but is probably the path/name the user most associated with the artifact
    if (auto fileRep = findRepresentation<IFileArtifactRepresentation>(artifact))
    {
        // If there isn't a lock file it is
        if (fileRep->getLockFile() == nullptr)
        {
            return UnownedStringSlice(fileRep->getPath());
        }
    }

    return UnownedStringSlice();
}

/* static */UnownedStringSlice ArtifactUtil::inferExtension(IArtifact* artifact)
{
    for (auto rep :artifact->getRepresentations())
    {
        if (auto fileRep = as<IFileArtifactRepresentation>(rep))
        {
            const char* path = fileRep->getPath();
            auto ext = Path::getPathExt(UnownedStringSlice(path));
            if (ext.getLength())
            {
                return ext;
            }
        }
    }

    // Okay lets see if the name has an extension
    return Path::getPathExt(UnownedStringSlice(artifact->getName()));
}

static SlangResult _calcInferred(IArtifact* artifact, const UnownedStringSlice& basePath, StringBuilder& outPath)
{
    auto ext = ArtifactUtil::inferExtension(artifact);

    // If no extension was determined by inferring, go with unknown
    if (ext.begin() == nullptr)
    {
        ext = toSlice("unknown");
    }

    outPath.Clear();
    outPath.append(basePath);
    if (ext.getLength())
    {
        outPath.appendChar('.');
        outPath.append(ext);
    }
    return SLANG_OK;
}

/* static */SlangResult ArtifactUtil::calcPath(IArtifact* artifact, const UnownedStringSlice& basePath, StringBuilder& outPath)
{
    if (ArtifactDescUtil::hasDefinedNameForDesc(artifact->getDesc()))
    {
        return ArtifactDescUtil::calcPathForDesc(artifact->getDesc(), basePath, outPath);
    }
    else
    {
        return _calcInferred(artifact, basePath, outPath);
    }
}

/* static */SlangResult ArtifactUtil::calcName(IArtifact* artifact, const UnownedStringSlice& baseName, StringBuilder& outName)
{
    if (ArtifactDescUtil::hasDefinedNameForDesc(artifact->getDesc()))
    {
        return ArtifactDescUtil::calcNameForDesc(artifact->getDesc(), baseName, outName);
    }
    else
    {
        return _calcInferred(artifact, baseName, outName);
    }
}

} // namespace Slang
