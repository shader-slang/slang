// slang-artifact-util.h
#ifndef SLANG_ARTIFACT_UTIL_H
#define SLANG_ARTIFACT_UTIL_H

#include "slang-artifact.h"

namespace Slang
{

class IArtifactUtil : public ISlangUnknown
{
	SLANG_COM_INTERFACE(0x882b25d7, 0xe300, 0x4b20, { 0xbe, 0xb, 0x26, 0xd2, 0x52, 0x3e, 0x70, 0x20 })

	virtual SLANG_NO_THROW SlangResult SLANG_MCALL createArtifact(const ArtifactDesc& desc, const char* name, IArtifact** outArtifact) = 0;

	virtual SLANG_NO_THROW SlangResult SLANG_MCALL createArtifactList(IArtifact* parent, IArtifactList** outArtifactList) = 0;

	virtual SLANG_NO_THROW ArtifactKind SLANG_MCALL getKindParent(ArtifactKind kind) = 0;
	virtual SLANG_NO_THROW UnownedStringSlice SLANG_MCALL getKindName(ArtifactKind kind) = 0;
	virtual SLANG_NO_THROW bool SLANG_MCALL isKindDerivedFrom(ArtifactKind kind, ArtifactKind base) = 0;

	virtual SLANG_NO_THROW ArtifactPayload SLANG_MCALL getPayloadParent(ArtifactPayload payload) = 0;
	virtual SLANG_NO_THROW UnownedStringSlice SLANG_MCALL getPayloadName(ArtifactPayload payload) = 0;
	virtual SLANG_NO_THROW bool SLANG_MCALL isPayloadDerivedFrom(ArtifactPayload payload, ArtifactPayload base) = 0;

	virtual SLANG_NO_THROW ArtifactStyle SLANG_MCALL getStyleParent(ArtifactStyle style) = 0;
	virtual SLANG_NO_THROW UnownedStringSlice SLANG_MCALL getStyleName(ArtifactStyle style) = 0;
	virtual SLANG_NO_THROW bool SLANG_MCALL isStyleDerivedFrom(ArtifactStyle style, ArtifactStyle base) = 0;

	virtual SLANG_NO_THROW SlangResult SLANG_MCALL createLockFile(const char* nameBase, ISlangMutableFileSystem* fileSystem, ILockFile** outLockFile) = 0;
};

class ArtifactUtilImpl : public IArtifactUtil
{
	// ISlangUnknown
	SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
	SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
	SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE;

	// IArtifactInterface
	virtual SLANG_NO_THROW SlangResult SLANG_MCALL createArtifact(const ArtifactDesc& desc, const char* name, IArtifact** outArtifact) SLANG_OVERRIDE;

	virtual SLANG_NO_THROW SlangResult SLANG_MCALL createArtifactList(IArtifact* parent, IArtifactList** outArtifactList) SLANG_OVERRIDE;

	virtual SLANG_NO_THROW ArtifactKind SLANG_MCALL getKindParent(ArtifactKind kind) SLANG_OVERRIDE;
	virtual SLANG_NO_THROW UnownedStringSlice SLANG_MCALL getKindName(ArtifactKind kind) SLANG_OVERRIDE;
	virtual SLANG_NO_THROW bool SLANG_MCALL isKindDerivedFrom(ArtifactKind kind, ArtifactKind base) SLANG_OVERRIDE;

	virtual SLANG_NO_THROW ArtifactPayload SLANG_MCALL getPayloadParent(ArtifactPayload payload) SLANG_OVERRIDE;
	virtual SLANG_NO_THROW UnownedStringSlice SLANG_MCALL getPayloadName(ArtifactPayload payload) SLANG_OVERRIDE;
	virtual SLANG_NO_THROW bool SLANG_MCALL isPayloadDerivedFrom(ArtifactPayload payload, ArtifactPayload base) SLANG_OVERRIDE;

	virtual SLANG_NO_THROW ArtifactStyle SLANG_MCALL getStyleParent(ArtifactStyle style) SLANG_OVERRIDE;
	virtual SLANG_NO_THROW UnownedStringSlice SLANG_MCALL getStyleName(ArtifactStyle style) SLANG_OVERRIDE;
	virtual SLANG_NO_THROW bool SLANG_MCALL isStyleDerivedFrom(ArtifactStyle style, ArtifactStyle base) SLANG_OVERRIDE;

	virtual SLANG_NO_THROW SlangResult SLANG_MCALL createLockFile(const char* nameBase, ISlangMutableFileSystem* fileSystem, ILockFile** outLockFile) SLANG_OVERRIDE;

	static IArtifactUtil* getSingleton() { return &g_singleton; }

protected:
	void* getInterface(const Guid& guid);

	static ArtifactUtilImpl g_singleton;
};

} // namespace Slang

#endif
