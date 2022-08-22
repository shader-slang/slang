// slang-downstream-dep1.cpp
#include "slang-downstream-dep1.h"

#include "slang-artifact-util.h"
#include "slang-artifact-associated-impl.h"
#include "slang-artifact-desc-util.h"

#include "../core/slang-castable-util.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!! DownstreamArtifactRepresentation_Dep1 !!!!!!!!!!!!!!!!!!!!!!!! */

class DownstreamResultArtifactRepresentationAdapater_Dep1 : public ComBaseObject, public IArtifactRepresentation
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE;

    // IArtifactRepresentation
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createRepresentation(const Guid& typeGuid, ICastable** outCastable) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE { return true; }

    DownstreamResultArtifactRepresentationAdapater_Dep1(DownstreamCompileResult_Dep1* result):
        m_result(result)
    {
    }

    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    RefPtr<DownstreamCompileResult_Dep1> m_result;
};

void* DownstreamResultArtifactRepresentationAdapater_Dep1::castAs(const SlangUUID& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void* DownstreamResultArtifactRepresentationAdapater_Dep1::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IArtifactRepresentation::getTypeGuid())
    {
        IArtifactRepresentation* rep = this;
        return rep;
    }
    return nullptr;
}

void* DownstreamResultArtifactRepresentationAdapater_Dep1::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

SlangResult DownstreamResultArtifactRepresentationAdapater_Dep1::createRepresentation(const Guid& typeGuid, ICastable** outCastable)
{
    if (typeGuid == ISlangSharedLibrary::getTypeGuid())
    {
        ComPtr<ISlangSharedLibrary> lib;
        SLANG_RETURN_ON_FAIL(DownstreamUtil_Dep1::getDownstreamSharedLibrary(m_result, lib));

        *outCastable = lib.detach();
        return SLANG_OK;
    }
    else if (typeGuid == ISlangBlob::getTypeGuid())
    {
        ComPtr<ISlangBlob> blob;
        SLANG_RETURN_ON_FAIL(m_result->getBinary(blob));

        *outCastable = CastableUtil::getCastable(blob).detach();
        return SLANG_OK;
    }

    return SLANG_E_NOT_AVAILABLE;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! DownstreamCompilerAdapter_Dep1 !!!!!!!!!!!!!!!!!!!!!!!! */

DownstreamCompilerAdapter_Dep1::DownstreamCompilerAdapter_Dep1(DownstreamCompiler_Dep1* dep, ArtifactPayload disassemblyPayload) :
    m_dep(dep),
    m_disassemblyPayload(disassemblyPayload)
{
    auto desc = dep->getDesc();
    m_desc = DownstreamCompilerDesc(desc.type, desc.majorVersion, desc.minorVersion);
}
SlangResult DownstreamCompilerAdapter_Dep1::compile(const CompileOptions& inOptions, IArtifact** outArtifact)
{
    typedef DownstreamCompileOptions_Dep1::SomeEnum SomeEnum;

    // Convert to the Deps1 compile options

    DownstreamCompileOptions_Dep1 options;

    options.optimizationLevel = SomeEnum(inOptions.optimizationLevel);
    options.debugInfoType = SomeEnum(inOptions.debugInfoType);
    options.targetType = inOptions.targetType;
    options.sourceLanguage = inOptions.sourceLanguage;
    options.floatingPointMode = SomeEnum(inOptions.floatingPointMode);
    options.pipelineType = SomeEnum(inOptions.pipelineType);
    options.matrixLayout = inOptions.matrixLayout;

    options.flags = inOptions.flags;
    options.platform = SomeEnum(inOptions.platform);

    options.modulePath = inOptions.modulePath;

    for (auto& src : inOptions.defines)
    {
        DownstreamCompileOptions_Dep1::Define dst;

        dst.nameWithSig = src.nameWithSig;
        dst.value = src.value;

        options.defines.add(dst);
    }

    options.sourceContents = inOptions.sourceContents;
    options.sourceContentsPath = inOptions.sourceContentsPath;

    options.sourceFiles = inOptions.sourceFiles;

    options.includePaths = inOptions.includePaths;
    options.libraryPaths = inOptions.libraryPaths;

    options.libraries = inOptions.libraries;

    for (auto& src : inOptions.requiredCapabilityVersions)
    {
        DownstreamCompileOptions_Dep1::CapabilityVersion capVer;
        capVer.kind = SomeEnum(src.kind);

        auto& srcVer = src.version;

        capVer.version.m_major = srcVer.m_major;
        capVer.version.m_minor = srcVer.m_minor;
        capVer.version.m_patch = uint16_t(srcVer.m_patch);

        options.requiredCapabilityVersions.add(capVer);
    }

    options.entryPointName = inOptions.entryPointName;
    options.profileName = inOptions.profileName;

    options.stage = inOptions.stage;

    options.compilerSpecificArguments = inOptions.compilerSpecificArguments;

    options.fileSystemExt = inOptions.fileSystemExt;
    options.sourceManager = inOptions.sourceManager;

    RefPtr<DownstreamCompileResult_Dep1> result;
    SLANG_RETURN_ON_FAIL(m_dep->compile(options, result));

    typedef CharSliceCaster Caster;

    ComPtr<IArtifact> artifact = ArtifactUtil::createArtifactForCompileTarget(options.targetType);

    // Convert the diagnostics

    auto dstDiagnostics = ArtifactDiagnostics::create();
    const DownstreamDiagnostics_Dep1* srcDiagnostics = &result->getDiagnostics();

    dstDiagnostics->setResult(srcDiagnostics->result);
    dstDiagnostics->setRaw(Caster::asCharSlice(srcDiagnostics->rawDiagnostics));

    for (const auto& srcDiagnostic : srcDiagnostics->diagnostics)
    {
        IArtifactDiagnostics::Diagnostic dstDiagnostic;

        dstDiagnostic.severity = ArtifactDiagnostic::Severity(srcDiagnostic.severity);
        dstDiagnostic.stage = ArtifactDiagnostic::Stage(srcDiagnostic.stage);

        dstDiagnostic.code = Caster::asTerminatedCharSlice(srcDiagnostic.code);
        dstDiagnostic.text = Caster::asTerminatedCharSlice(srcDiagnostic.text);
        dstDiagnostic.filePath = Caster::asTerminatedCharSlice(srcDiagnostic.filePath);

        dstDiagnostic.location.line = srcDiagnostic.fileLine;
    }

    artifact->addAssociated(dstDiagnostics);

    // We need to add a representation that can produce shared libraries/blobs on demand

    auto rep = new DownstreamResultArtifactRepresentationAdapater_Dep1(result);
    artifact->addRepresentation(rep);

    *outArtifact = artifact.detach();
    return SLANG_OK;
}

bool DownstreamCompilerAdapter_Dep1::canConvert(const ArtifactDesc& from, const ArtifactDesc& to)
{
    // Can only disassemble blobs that are DXBC
    return ArtifactDescUtil::isDissassembly(from, to) && from.payload == m_disassemblyPayload;
}

SlangResult DownstreamCompilerAdapter_Dep1::convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact)
{
    if (!canConvert(from->getDesc(), to))
    {
        return SLANG_FAIL;
    }

    ComPtr<ISlangBlob> fromBlob;
    SLANG_RETURN_ON_FAIL(from->loadBlob(ArtifactKeep::No, fromBlob.writeRef()));

    const auto compileTarget = ArtifactDescUtil::getCompileTargetFromDesc(from->getDesc());

    // Do the disassembly
    ComPtr<ISlangBlob> dstBlob;
    SLANG_RETURN_ON_FAIL(m_dep->disassemble(compileTarget, fromBlob->getBufferPointer(), fromBlob->getBufferSize(), dstBlob.writeRef()));

    auto artifact = ArtifactUtil::createArtifact(to);

    artifact->addRepresentationUnknown(dstBlob);

    *outArtifact = artifact.detach();
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! SharedLibraryDep1Adapter !!!!!!!!!!!!!!!!!!!!!!!! */

// A temporary class that adapts `ISlangSharedLibrary_Dep1` to ISlangSharedLibrary
class SharedLibraryAdapter_Dep1 : public ComBaseObject, public ISlangSharedLibrary
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE;

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) SLANG_OVERRIDE { return m_contained->findSymbolAddressByName(name); }

    SharedLibraryAdapter_Dep1(ISlangSharedLibrary_Dep1* dep1) :
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

void* SharedLibraryAdapter_Dep1::castAs(const SlangUUID& guid)
{
	if (auto intf = getInterface(guid))
	{
		return intf;
	}
	return getObject(guid);
}

/* Hack to take into account downstream compilers shared library interface might need an adapter */
/* static */SlangResult DownstreamUtil_Dep1::getDownstreamSharedLibrary(DownstreamCompileResult_Dep1* downstreamResult, ComPtr<ISlangSharedLibrary>& outSharedLibrary)
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
		outSharedLibrary = new SharedLibraryAdapter_Dep1(libDep1);
		return SLANG_OK;
	}
	return SLANG_E_NOT_FOUND;
}

}
