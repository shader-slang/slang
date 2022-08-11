// slang-artifact.cpp
#include "slang-artifact-util.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"

#include "slang-artifact-desc-util.h"

#include "../core/slang-castable-list-impl.h"

#include "../core/slang-io.h"

namespace Slang {

/* static */ComPtr<IArtifactContainer> ArtifactUtil::createContainer(const ArtifactDesc& desc)
{
    const auto containerDesc = ArtifactDesc::make(ArtifactKind::Container, ArtifactPayload::CompileResults, desc.style);
    return ArtifactContainer::create(containerDesc);
}

/* static */ComPtr<IArtifactContainer> ArtifactUtil::createResultsContainer()
{
    return ArtifactContainer::create(ArtifactDesc::make(ArtifactKind::Container, ArtifactPayload::CompileResults));
}

/* static */ComPtr<IArtifact> ArtifactUtil::createArtifactForCompileTarget(SlangCompileTarget target)
{
    auto desc = ArtifactDescUtil::makeDescFromCompileTarget(target);
    return Artifact::create(desc);
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
    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike) ||
        desc.kind == ArtifactKind::Assembly ||
        desc.kind == ArtifactKind::Source)
    {
        return true;
    }


    /* Hmm, we might want to have a base class for 'signifiant' payloads,
    where signifiance here means somewhat approximately 'the meat' of a compilation result,
    as contrasted with 'meta data', 'diagnostics etc'*/
    if (isDerivedFrom(desc.payload, ArtifactPayload::MetaData))
    {
        return false;
    }

    return true;
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


} // namespace Slang
