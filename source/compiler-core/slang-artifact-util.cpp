// slang-artifact.cpp
#include "slang-artifact-util.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"

#include "slang-artifact-desc-util.h"

#include "../core/slang-io.h"

namespace Slang {

static bool _checkSelf(ArtifactUtil::FindStyle findStyle)
{
    return Index(findStyle) <= Index(ArtifactUtil::FindStyle::SelfOrChildren);
}

static bool _checkChildren(ArtifactUtil::FindStyle findStyle)
{
    return Index(findStyle) >= Index(ArtifactUtil::FindStyle::SelfOrChildren);
}

static bool _checkRecursive(ArtifactUtil::FindStyle findStyle)
{
    return findStyle == ArtifactUtil::FindStyle::Recursive || 
        findStyle == ArtifactUtil::FindStyle::ChildrenRecursive;
}
 
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
    return findArtifactByPredicate(artifact, FindStyle::SelfOrChildren, &ArtifactUtil::isSignificant, nullptr);
}

UnownedStringSlice ArtifactUtil::findPath(IArtifact* artifact)
{
    // If a name is set we'll just use that
    {
        const UnownedStringSlice name(artifact->getName());
        if (name.getLength())
        {
            return name;
        }
    }

    IPathArtifactRepresentation* bestRep = nullptr;

    // Look for a rep with a path. Prefer IExtFile because a IOSFile might be a temporary file
    for (auto rep : artifact->getRepresentations())
    {
        if (auto pathRep = as<IPathArtifactRepresentation>(rep))
        {
            if (pathRep->getPathType() == SLANG_PATH_TYPE_FILE && 
                (bestRep == nullptr || as<IExtFileArtifactRepresentation>(rep)))
            {
                bestRep = pathRep;
            }
        }
    }

    const UnownedStringSlice name = bestRep ? UnownedStringSlice(bestRep->getPath()) : UnownedStringSlice();
    return name.getLength() ? name : UnownedStringSlice();
}

/* static */UnownedStringSlice ArtifactUtil::inferExtension(IArtifact* artifact)
{
    const UnownedStringSlice path = findPath(artifact);
    if (path.getLength())
    {
        auto ext = Path::getPathExt(UnownedStringSlice(path));
        if (ext.getLength())
        {
            return ext;
        }
    }
    return UnownedStringSlice();
}

/* static */UnownedStringSlice ArtifactUtil::findName(IArtifact* artifact)
{
    const UnownedStringSlice path = findPath(artifact);
    const Index pos = Path::findLastSeparatorIndex(path);
    return (pos >= 0) ? path.tail(pos + 1) : path;
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

static bool _isByDerivedDesc(IArtifact* artifact, void* data)
{
    const ArtifactDesc& desc = *(const ArtifactDesc*)data;
    return ArtifactDescUtil::isDescDerivedFrom(artifact->getDesc(), desc);
}

static bool _isDesc(IArtifact* artifact, void* data)
{
    const ArtifactDesc& desc = *(const ArtifactDesc*)data;
    return artifact->getDesc() == desc;
}

static bool _isName(IArtifact* artifact, void* data)
{
    const char* name = (const char*)data;
    const auto artifactName = artifact->getName();

    if (name == nullptr || artifactName == nullptr)
    {
        return name == artifactName;
    }
    return ::strcmp(name, artifactName) == 0;
}

/* static */IArtifact* ArtifactUtil::findArtifactByDerivedDesc(IArtifact* artifact, FindStyle findStyle, const ArtifactDesc& desc)
{
    return findArtifactByPredicate(artifact, findStyle, _isByDerivedDesc, &const_cast<ArtifactDesc&>(desc));
}

/* static */IArtifact* ArtifactUtil::findArtifactByName(IArtifact* artifact, FindStyle findStyle, const char* name)
{
    return findArtifactByPredicate(artifact, findStyle, _isName, const_cast<char*>(name));
}

/* static */IArtifact* ArtifactUtil::findArtifactByDesc(IArtifact* artifact, FindStyle findStyle, const ArtifactDesc& desc)
{
    return findArtifactByPredicate(artifact, findStyle, _isDesc, &const_cast<ArtifactDesc&>(desc));
}

/* static */IArtifact* ArtifactUtil::findArtifactByPredicate(IArtifact* artifact, FindStyle findStyle, ArtifactFindFunc func, void* data)
{
    if (_checkSelf(findStyle) && func(artifact, data))
    {
        return artifact;
    }

    if (!_checkChildren(findStyle))
    {
        return nullptr;
    }

    // We could force an expansion of children here...
    // 
  
    auto children = artifact->getChildren();
    if (children.count == 0)
    {
        return nullptr;
    }

    // Check the children
    for (auto child : children)
    {
        if (func(child, data))
        {
            return child;
        }
    }

    // If it's recursive, we check all the children of children 
    if (_checkRecursive(findStyle))
    {
        for (auto child : children)
        {
            if (auto found = findArtifactByPredicate(child, FindStyle::ChildrenRecursive, func, data))
            {
                return found;
            }
        }
    }
 
    return nullptr;
}

} // namespace Slang
