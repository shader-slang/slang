// slang-artifact.cpp
#include "slang-artifact-util.h"

#include "slang-artifact-info.h"

#include "../core/slang-file-system.h"
#include "../core/slang-io.h"

namespace Slang {

/* static */ArtifactUtilImpl ArtifactUtilImpl::g_singleton;

SlangResult ArtifactUtilImpl::queryInterface(SlangUUID const& uuid, void** outObject)
{
	if (auto intf = getInterface(uuid))
	{
		*outObject = intf;
		return SLANG_OK;
	}
	return SLANG_E_NO_INTERFACE;
}

void* ArtifactUtilImpl::getInterface(const Guid& guid)
{
	if (guid == ISlangUnknown::getTypeGuid() || guid == IArtifactUtil::getTypeGuid())
	{
		return static_cast<IArtifactUtil*>(this);
	}
	return nullptr;
}


SlangResult ArtifactUtilImpl::createArtifact(const ArtifactDesc& desc, const char* inName, IArtifact** outArtifact)
{
	String name;
	if (inName)
	{
		name = inName;
	}

	ComPtr<IArtifact> artifact(new Artifact(desc, name));

	*outArtifact = artifact.detach();
	return SLANG_OK;
}

SlangResult ArtifactUtilImpl::createArtifactList(IArtifact* parent, IArtifactList** outArtifactList)
{
	ComPtr<IArtifactList> artifactList(new ArtifactList(parent));
	*outArtifactList = artifactList.detach();
	return SLANG_OK;
}

ArtifactKind ArtifactUtilImpl::getKindParent(ArtifactKind kind) { return getParent(kind); }
UnownedStringSlice ArtifactUtilImpl::getKindName(ArtifactKind kind) { return getName(kind); }
bool ArtifactUtilImpl::isKindDerivedFrom(ArtifactKind kind, ArtifactKind base) { return isDerivedFrom(kind, base); }

ArtifactPayload ArtifactUtilImpl::getPayloadParent(ArtifactPayload payload) { return getParent(payload); }
UnownedStringSlice ArtifactUtilImpl::getPayloadName(ArtifactPayload payload) { return getName(payload); }
bool ArtifactUtilImpl::isPayloadDerivedFrom(ArtifactPayload payload, ArtifactPayload base) { return isDerivedFrom(payload, base); }

ArtifactStyle ArtifactUtilImpl::getStyleParent(ArtifactStyle style) { return getParent(style); }
UnownedStringSlice ArtifactUtilImpl::getStyleName(ArtifactStyle style) { return getName(style); }
bool ArtifactUtilImpl::isStyleDerivedFrom(ArtifactStyle style, ArtifactStyle base) { return isDerivedFrom(style, base); }

SlangResult ArtifactUtilImpl::createLockFile(const char* inNameBase, ISlangMutableFileSystem* fileSystem, ILockFile** outLockFile)
{
	if (fileSystem)
	{
		if (fileSystem != OSFileSystem::getMutableSingleton())
		{
			// We can only create lock files, on the global OS file system
			return SLANG_E_NOT_AVAILABLE;
		}
		fileSystem = nullptr;
	}

	const UnownedStringSlice nameBase = inNameBase ? UnownedStringSlice(inNameBase) : UnownedStringSlice("unknown");

	String lockPath;
	SLANG_RETURN_ON_FAIL(File::generateTemporary(nameBase, lockPath));

	ComPtr<ILockFile> lockFile(new LockFile(lockPath, fileSystem));

	*outLockFile = lockFile.detach();
	return SLANG_OK;
}

} // namespace Slang
