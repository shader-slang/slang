// slang-artifact.cpp
#include "slang-artifact-util.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"

#include "slang-artifact-desc-util.h"

#include "../core/slang-file-system.h"
#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactUtilImpl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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

SlangResult ArtifactUtilImpl::createLockFile(const char* inNameBase, ISlangMutableFileSystem* fileSystem, IFileArtifactRepresentation** outLockFile)
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

	ComPtr<IFileArtifactRepresentation> lockFile(new FileArtifactRepresentation(IFileArtifactRepresentation::Kind::Lock, lockPath, nullptr, fileSystem));

	*outLockFile = lockFile.detach();
	return SLANG_OK;
}

SlangResult ArtifactUtilImpl::calcArtifactPath(const ArtifactDesc& desc, const char* inBasePath, ISlangBlob** outPath)
{
	UnownedStringSlice basePath(inBasePath);
	StringBuilder path;
	SLANG_RETURN_ON_FAIL(ArtifactDescUtil::calcPathForDesc(desc, basePath, path));
	*outPath = StringBlob::create(path).detach();
	return SLANG_OK;
}

static bool _isFileSystemFile(ICastable* castable, void* data)
{
	if (auto fileRep = as<IFileArtifactRepresentation>(castable))
	{
		ISlangMutableFileSystem* fileSystem = (ISlangMutableFileSystem*)data;
		if (fileRep->getFileSystem() == fileSystem)
		{
			return true;
		}
	}
	return false;
}

SlangResult ArtifactUtilImpl::requireFileDefaultImpl(IArtifact* artifact, ArtifactKeep keep, ISlangMutableFileSystem* fileSystem, IFileArtifactRepresentation** outFile)
{
	// See if we already have it
	if (auto fileRep = as<IFileArtifactRepresentation>(artifact->findRepresentationWithPredicate(&_isFileSystemFile, fileSystem)))
	{
		fileRep->addRef();
		*outFile = fileRep;
		return SLANG_OK;
	}

	// If we are going to access as a file we need to be able to write it, and to do that we need a blob
	ComPtr<ISlangBlob> blob;
	SLANG_RETURN_ON_FAIL(artifact->loadBlob(getIntermediateKeep(keep), blob.writeRef()));

	// Okay we need to store as a temporary. Get a lock file.
	ComPtr<IFileArtifactRepresentation> lockFile;
	SLANG_RETURN_ON_FAIL(createLockFile(artifact->getName(), fileSystem, lockFile.writeRef()));

	// Now we need the appropriate name for this item
	ComPtr<ISlangBlob> pathBlob;
	SLANG_RETURN_ON_FAIL(calcArtifactPath(artifact->getDesc(), lockFile->getPath(), pathBlob.writeRef()));

	const auto path = StringUtil::getString(pathBlob);

	// Write the contents
	SLANG_RETURN_ON_FAIL(File::writeAllBytes(path, blob->getBufferPointer(), blob->getBufferSize()));

	ComPtr<IFileArtifactRepresentation> fileRep;

	// TODO(JS): This path comparison is perhaps not perfect, in that it assumes the path is not changed
	// in any way. For example an impl of calcArtifactPath that changed slashes or used a canonical path
	// might mean the lock file and the rep have the same path. 
	// As it stands calcArtifactPath impl doesn't do that, but that is perhaps somewhatfragile

	// If the paths are identical, we can just use the lock file for the rep
	if (UnownedStringSlice(lockFile->getPath()) == path.getUnownedSlice())
	{
		fileRep.swap(lockFile);
	}
	else
	{
		// Create a new rep that references the lock file
		fileRep = new FileArtifactRepresentation(IFileArtifactRepresentation::Kind::Owned, path, lockFile, lockFile->getFileSystem());
	}

	// Create the rep
	if (canKeep(keep))
	{
		artifact->addRepresentation(fileRep);
	}

	// Return the file
	*outFile = fileRep.detach();
	return SLANG_OK;
}

ArtifactDesc ArtifactUtilImpl::makeDescFromCompileTarget(SlangCompileTarget target)
{
	return ArtifactDescUtil::makeDescFromCompileTarget(target);
}

SlangResult ArtifactUtilImpl::getChildrenDefaultImpl(IArtifact* artifact, IArtifactList** outList)
{
	auto desc = artifact->getDesc();

	// If it's a container type for now, just create empty list of children
	if (isDerivedFrom(desc.kind, ArtifactKind::Container))
	{
		*outList = ComPtr<IArtifactList>(new ArtifactList(artifact)).detach();
		return SLANG_OK;
	}

	return SLANG_E_NOT_AVAILABLE;
}

SlangResult ArtifactUtilImpl::loadSharedLibraryDefaultImpl(IArtifact* artifact, ArtifactKeep keep, ISlangSharedLibrary** outSharedLibrary)
{
	if (auto sharedLibrary = findRepresentation<ISlangSharedLibrary>(artifact))
	{
		sharedLibrary->addRef();
		*outSharedLibrary = sharedLibrary;
		return SLANG_OK;
	}

	// If it is 'shared library' for a CPU like thing, we can try and load it
	const auto desc = artifact->getDesc();
	if ((isDerivedFrom(desc.kind,ArtifactKind::HostCallable) || 
		isDerivedFrom(desc.kind, ArtifactKind::SharedLibrary)) &&
		isDerivedFrom(desc.payload, ArtifactPayload::CPULike))
	{
		// Get as a file represenation on the OS file system
		ComPtr<IFileArtifactRepresentation> fileRep;
		SLANG_RETURN_ON_FAIL(artifact->requireFile(ArtifactKeep::No, nullptr, fileRep.writeRef()));

		// We requested on the OS file system, just check that's what we got...
		SLANG_ASSERT(fileRep->getFileSystem() == nullptr);

		// Try loading the shared library
		SharedLibrary::Handle handle;
		if (SLANG_FAILED(SharedLibrary::loadWithPlatformPath(fileRep->getPath(), handle)))
		{
			return SLANG_FAIL;
		}

		// The ScopeSharedLibrary will keep the fileRep in scope as long as is needed
		auto sharedLibrary = new ScopeSharedLibrary(handle, fileRep);

		if (canKeep(keep))
		{
			// We want to keep the fileRep, as that is necessary for the sharedLibrary to even work
			artifact->addRepresentation(fileRep);
			// Keep the shared library
			artifact->addRepresentation(sharedLibrary);
		}

		// Output
		sharedLibrary->addRef();
		*outSharedLibrary = sharedLibrary;
		
		return SLANG_OK;
	}

	return SLANG_FAIL;
}

} // namespace Slang
