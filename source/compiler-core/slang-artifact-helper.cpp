// slang-artifact-helper.cpp
#include "slang-artifact-helper.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"

#include "slang-artifact-desc-util.h"
#include "slang-artifact-util.h"

#include "../core/slang-castable-list-impl.h"
#include "../core/slang-castable-util.h"

#include "../core/slang-file-system.h"
#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! DefaultArtifactHelper !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */DefaultArtifactHelper DefaultArtifactHelper::g_singleton;

SlangResult DefaultArtifactHelper::queryInterface(SlangUUID const& uuid, void** outObject)
{
	if (auto intf = getInterface(uuid))
	{
		*outObject = intf;
		return SLANG_OK;
	}
	return SLANG_E_NO_INTERFACE;
}

void* DefaultArtifactHelper::castAs(const Guid& guid)
{
	if (auto ptr = getInterface(guid))
	{
		return ptr;
	}
	return getObject(guid);
}

void* DefaultArtifactHelper::getInterface(const Guid& guid)
{
	if (guid == ISlangUnknown::getTypeGuid() || 
		guid == IArtifactHelper::getTypeGuid())
	{
		return static_cast<IArtifactHelper*>(this);
	}
	return nullptr;
}

void* DefaultArtifactHelper::getObject(const Guid& guid)
{
	SLANG_UNUSED(guid);
	return nullptr;
}

SlangResult DefaultArtifactHelper::createArtifact(const ArtifactDesc& desc, const char* inName, IArtifact** outArtifact)
{
	*outArtifact = inName ?
		Artifact::create(desc, UnownedStringSlice(inName)).detach() : 
		Artifact::create(desc).detach();

	return SLANG_OK;
}

SlangResult DefaultArtifactHelper::createArtifactContainer(const ArtifactDesc& desc, const char* inName, IArtifactContainer** outArtifactContainer)
{
	*outArtifactContainer = inName ?
		ArtifactContainer::create(desc, UnownedStringSlice(inName)).detach() :
		ArtifactContainer::create(desc).detach();

	return SLANG_OK;
}

ArtifactKind DefaultArtifactHelper::getKindParent(ArtifactKind kind) { return getParent(kind); }
UnownedStringSlice DefaultArtifactHelper::getKindName(ArtifactKind kind) { return getName(kind); }
bool DefaultArtifactHelper::isKindDerivedFrom(ArtifactKind kind, ArtifactKind base) { return isDerivedFrom(kind, base); }

ArtifactPayload DefaultArtifactHelper::getPayloadParent(ArtifactPayload payload) { return getParent(payload); }
UnownedStringSlice DefaultArtifactHelper::getPayloadName(ArtifactPayload payload) { return getName(payload); }
bool DefaultArtifactHelper::isPayloadDerivedFrom(ArtifactPayload payload, ArtifactPayload base) { return isDerivedFrom(payload, base); }

ArtifactStyle DefaultArtifactHelper::getStyleParent(ArtifactStyle style) { return getParent(style); }
UnownedStringSlice DefaultArtifactHelper::getStyleName(ArtifactStyle style) { return getName(style); }
bool DefaultArtifactHelper::isStyleDerivedFrom(ArtifactStyle style, ArtifactStyle base) { return isDerivedFrom(style, base); }

SlangResult DefaultArtifactHelper::createLockFile(const char* inNameBase, ISlangMutableFileSystem* fileSystem, IFileArtifactRepresentation** outLockFile)
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

	const UnownedStringSlice nameBase = (inNameBase && inNameBase[0] != 0) ? UnownedStringSlice(inNameBase) : UnownedStringSlice("unknown");

	String lockPath;
	SLANG_RETURN_ON_FAIL(File::generateTemporary(nameBase, lockPath));

	ComPtr<IFileArtifactRepresentation> lockFile(new FileArtifactRepresentation(IFileArtifactRepresentation::Kind::Lock, lockPath.getUnownedSlice(), nullptr, fileSystem));

	*outLockFile = lockFile.detach();
	return SLANG_OK;
}

SlangResult DefaultArtifactHelper::calcArtifactPath(const ArtifactDesc& desc, const char* inBasePath, ISlangBlob** outPath)
{
	UnownedStringSlice basePath(inBasePath);
	StringBuilder path;
	SLANG_RETURN_ON_FAIL(ArtifactDescUtil::calcPathForDesc(desc, basePath, path));
	*outPath = StringBlob::create(path).detach();
	return SLANG_OK;
}

ArtifactDesc DefaultArtifactHelper::makeDescForCompileTarget(SlangCompileTarget target)
{
	return ArtifactDescUtil::makeDescForCompileTarget(target);
}

void DefaultArtifactHelper::getCastable(ISlangUnknown* unk, ICastable** outCastable)
{
	*outCastable = CastableUtil::getCastable(unk).detach();
}

SlangResult DefaultArtifactHelper::createCastableList(const Guid& guid, ICastableList** outList)
{
	auto list = new CastableList;
	if (auto ptr = list->getInterface(guid))
	{
		list->addRef();
		*outList = (ICastableList*)ptr;
		return SLANG_OK;
	}
	delete list;
	return SLANG_E_NO_INTERFACE;
}

} // namespace Slang
