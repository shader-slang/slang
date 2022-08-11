// slang-artifact.cpp
#include "slang-artifact-util.h"

#include "slang-artifact-impl.h"
#include "slang-artifact-representation-impl.h"

#include "slang-artifact-desc-util.h"

#include "../core/slang-castable-list-impl.h"

#include "../core/slang-file-system.h"
#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"

// For workaround for DownstreamResult
#include "slang-downstream-compiler.h"

namespace Slang {

// A temporary class that adapts `ISlangSharedLibrary_Dep1` to ISlangSharedLibrary

class SharedLibraryDep1Adapter : public ComBaseObject, public ISlangSharedLibrary
{
public:
	SLANG_COM_BASE_IUNKNOWN_ALL

		// ICastable
	virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE;

	// ISlangSharedLibrary
	virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) SLANG_OVERRIDE { return m_contained->findSymbolAddressByName(name); }

	SharedLibraryDep1Adapter(ISlangSharedLibrary_Dep1* dep1) :
		m_contained(dep1)
	{
	}

protected:
	void* getInterface(const Guid& guid)
	{
		if (guid == ISlangUnknown::getTypeGuid() ||
			guid == ICastable::getTypeGuid() ||
			guid == ISlangSharedLibrary::getTypeGuid())
		{
			return static_cast<ISlangSharedLibrary*>(this);
		}
		return nullptr;
	}
	void* getObject(const Guid& guid)
	{
		SLANG_UNUSED(guid);
		return nullptr;
	}

	ComPtr<ISlangSharedLibrary_Dep1> m_contained;
};

void* SharedLibraryDep1Adapter::castAs(const SlangUUID& guid)
{
	if (auto intf = getInterface(guid))
	{
		return intf;
	}
	return getObject(guid);
}

/* Hack to take into account downstream compilers shared library interface might need an adapter */
static SlangResult _getDownstreamSharedLibrary(DownstreamCompileResult* downstreamResult, ComPtr<ISlangSharedLibrary>& outSharedLibrary)
{
	ComPtr<ISlangSharedLibrary> lib;
	SLANG_RETURN_ON_FAIL(downstreamResult->getHostCallableSharedLibrary(lib));

	if (SLANG_SUCCEEDED(lib->queryInterface(ISlangSharedLibrary::getTypeGuid(), (void**)outSharedLibrary.writeRef())))
	{
		return SLANG_OK;
	}

	ComPtr<ISlangSharedLibrary_Dep1> libDep1;
	if (SLANG_SUCCEEDED(lib->queryInterface(ISlangSharedLibrary_Dep1::getTypeGuid(), (void**)libDep1.writeRef())))
	{
		// Okay, we need to adapt for now
		outSharedLibrary = new SharedLibraryDep1Adapter(libDep1);
		return SLANG_OK;
	}
	return SLANG_E_NOT_FOUND;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! DefaultArtifactHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */DefaultArtifactHandler DefaultArtifactHandler::g_singleton;

SlangResult DefaultArtifactHandler::queryInterface(SlangUUID const& uuid, void** outObject)
{
	if (auto ptr = getInterface(uuid))
	{
		addRef();
		*outObject = static_cast<IArtifactHandler*>(this);
		return SLANG_OK;
	}
	return SLANG_E_NO_INTERFACE;
}

void* DefaultArtifactHandler::castAs(const Guid& guid)
{
	if (auto ptr = getInterface(guid))
	{
		return ptr;
	}
	return getObject(guid);
}

void* DefaultArtifactHandler::getInterface(const Guid& uuid)
{
	if (uuid == ISlangUnknown::getTypeGuid() ||
		uuid == ICastable::getTypeGuid() ||
		uuid == IArtifactHandler::getTypeGuid())
	{
		return static_cast<IArtifactHandler*>(this);
	}

	return nullptr;
}

void* DefaultArtifactHandler::getObject(const Guid& uuid)
{
	SLANG_UNUSED(uuid);
	return nullptr;
}

SlangResult DefaultArtifactHandler::_addRepresentation(IArtifact* artifact, ArtifactKeep keep, ISlangUnknown* rep, ICastable** outCastable)
{
	SLANG_ASSERT(rep);

	// See if it implements ICastable
	{
		ComPtr<ICastable> castable;
		if (SLANG_SUCCEEDED(rep->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
		{
			return _addRepresentation(artifact, keep, castable, outCastable);
		}
	}

	// We have to wrap 
	ComPtr<IUnknownCastableAdapter> adapter(new UnknownCastableAdapter(rep));
	return _addRepresentation(artifact, keep, adapter, outCastable);
}

SlangResult DefaultArtifactHandler::_addRepresentation(IArtifact* artifact, ArtifactKeep keep, ICastable* castable, ICastable** outCastable)
{
	SLANG_ASSERT(castable);

	if (canKeep(keep))
	{
		artifact->addRepresentation(castable);
	}

	castable->addRef();
	*outCastable = castable;
	return SLANG_OK;
}

SlangResult DefaultArtifactHandler::expandChildren(IArtifactContainer* container)
{
	SlangResult res = container->getExpandChildrenResult();
	if (res != SLANG_E_UNINITIALIZED)
	{
		// It's already expanded
		return res;
	}

	// For the generic container type, we just expand as empty
	const auto desc = container->getDesc();
	if (desc.kind == ArtifactKind::Container)
	{
		container->setChildren(nullptr, 0);
		return SLANG_OK;
	}
	// TODO(JS):
	// Proper implementation should (for example) be able to expand a Zip file etc.
	return SLANG_E_NOT_IMPLEMENTED;
}



SlangResult DefaultArtifactHandler::getOrCreateRepresentation(IArtifact* artifact, const Guid& guid, ArtifactKeep keep, ICastable** outCastable)
{
	// See if we already have a rep of this type
	{
		for (ICastable* rep : artifact->getRepresentations())
		{
			if (rep->castAs(guid))
			{
				rep->addRef();
				*outCastable = rep;
				return SLANG_OK;
			}
		}
	}

	// TODO(JS): Temporary whilst DownstreamCompileResult is 
	// Special handling for DownstreamCompileResult
	if (auto downstreamResult = findRepresentation<DownstreamCompileResult>(artifact))
	{
		if (guid == ISlangBlob::getTypeGuid())
		{
			ComPtr<ISlangBlob> blob;
			SLANG_RETURN_ON_FAIL(downstreamResult->getBinary(blob));
			return _addRepresentation(artifact, keep, blob, outCastable);
		}
		else if (guid == ISlangSharedLibrary::getTypeGuid())
		{
			ComPtr<ISlangSharedLibrary> lib;
			SLANG_RETURN_ON_FAIL(_getDownstreamSharedLibrary(downstreamResult, lib));
			return _addRepresentation(artifact, keep, lib, outCastable);
		}
	}

	// Normal construction
	if (guid == ISlangBlob::getTypeGuid())
	{
		ComPtr<ISlangBlob> blob;
		SLANG_RETURN_ON_FAIL(_loadBlob(artifact, keep, blob.writeRef()));
		return _addRepresentation(artifact, keep, blob, outCastable);
	}
	else if (guid == ISlangSharedLibrary::getTypeGuid())
	{
		ComPtr<ISlangSharedLibrary> sharedLib;
		SLANG_RETURN_ON_FAIL(_loadSharedLibrary(artifact, keep, sharedLib.writeRef()));
		return _addRepresentation(artifact, keep, sharedLib, outCastable);
	}

	return SLANG_E_NOT_AVAILABLE;
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

SlangResult DefaultArtifactHandler::getOrCreateFileRepresentation(IArtifact* artifact, ArtifactKeep keep, ISlangMutableFileSystem* fileSystem, IFileArtifactRepresentation** outFileRep)
{
	// See if we already have it
	if (auto fileRep = as<IFileArtifactRepresentation>(artifact->findRepresentationWithPredicate(&_isFileSystemFile, fileSystem)))
	{
		fileRep->addRef();
		*outFileRep = fileRep;
		return SLANG_OK;
	}

	auto util = ArtifactUtilImpl::getSingleton();

	// If we are going to access as a file we need to be able to write it, and to do that we need a blob
	ComPtr<ISlangBlob> blob;
	SLANG_RETURN_ON_FAIL(artifact->loadBlob(getIntermediateKeep(keep), blob.writeRef()));

	// Okay we need to store as a temporary. Get a lock file.
	ComPtr<IFileArtifactRepresentation> lockFile;
	SLANG_RETURN_ON_FAIL(util->createLockFile(artifact->getName(), fileSystem, lockFile.writeRef()));

	// Now we need the appropriate name for this item
	ComPtr<ISlangBlob> pathBlob;
	SLANG_RETURN_ON_FAIL(util->calcArtifactPath(artifact->getDesc(), lockFile->getPath(), pathBlob.writeRef()));

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
	*outFileRep = fileRep.detach();
	return SLANG_OK;
}

SlangResult DefaultArtifactHandler::_loadSharedLibrary(IArtifact* artifact, ArtifactKeep keep, ISlangSharedLibrary** outSharedLibrary)
{
	// If it is 'shared library' for a CPU like thing, we can try and load it
	const auto desc = artifact->getDesc();
	if ((isDerivedFrom(desc.kind, ArtifactKind::HostCallable) ||
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

SlangResult DefaultArtifactHandler::_loadBlob(IArtifact* artifact, ArtifactKeep keep, ISlangBlob** outBlob)
{
	SLANG_UNUSED(keep);

	ComPtr<ISlangBlob> blob;

	// Look for a representation that we can serialize into a blob
	for (auto rep : artifact->getRepresentations())
	{
		if (auto artifactRep = as<IArtifactRepresentation>(rep))
		{
			SlangResult res = artifactRep->writeToBlob(blob.writeRef());
			if (SLANG_SUCCEEDED(res) && blob)
			{
				break;
			}
		}
	}

	// Wasn't able to construct
	if (!blob)
	{
		return SLANG_E_NOT_FOUND;
	}

	*outBlob = blob.detach();
	return SLANG_OK;
}

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
	*outArtifact = inName ?
		Artifact::create(desc, inName).detach() : 
		Artifact::create(desc).detach();

	return SLANG_OK;
}

SlangResult ArtifactUtilImpl::createArtifactContainer(const ArtifactDesc& desc, const char* inName, IArtifactContainer** outArtifactContainer)
{
	*outArtifactContainer = inName ?
		ArtifactContainer::create(desc, inName).detach() :
		ArtifactContainer::create(desc).detach();

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

ArtifactDesc ArtifactUtilImpl::makeDescFromCompileTarget(SlangCompileTarget target)
{
	return ArtifactDescUtil::makeDescFromCompileTarget(target);
}

} // namespace Slang
