// slang-entry-point.h
#pragma once

//
// This file provides the `EntryPoint` type, which is used
// to represent an entry point that has been identified
// and validated by the compiler front-end inside of some
// `Module`.
//
// Note that the `FrontEndEntryPointRequest` type, corresponding
// to use of the `-entry` command-line option, is not
// declared here, and instead comes from `slang-compile-request.h`.
//

#include "slang-linkable.h"

namespace Slang
{

/// Describes an entry point for the purposes of layout and code generation.
///
/// This type intentionally does not distinguish between entry
/// points that were discovered because of a `[shader(...)]` attribute
/// and those that were identified by a `FrontEndEntryPointRequest`
/// (e.g., as a result of a `-entry` command-line option). The
/// intention is that *how* an `EntryPoint` came to be is a purely
/// front-end consideration, and the back-end of the compiler should
/// never depend on that information.
///
/// This class also tracks any generic arguments to the entry point,
/// in the case that it is a specialization of a generic entry point.
///
/// There is also a provision for creating a "dummy" entry point for
/// the purposes of pass-through compilation modes. Only the
/// `getName()` and `getProfile()` methods should be expected to
/// return useful data on pass-through entry points.
///
class EntryPoint : public ComponentType, public slang::IEntryPoint
{
    typedef ComponentType Super;

public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    ISlangUnknown* getInterface(const Guid& guid);


    // Forward `IComponentType` methods

    SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
    {
        return Super::getSession();
    }

    SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getLayout(targetIndex, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getTargetCode(targetIndex, outCode, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointMetadata(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointMetadata(
            entryPointIndex,
            targetIndex,
            outMetadata,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getTargetMetadata(targetIndex, outMetadata, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCompileResult(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointCompileResult(
            entryPointIndex,
            targetIndex,
            outCompileResult,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCompileResult(
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getTargetCompileResult(targetIndex, outCompileResult, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) SLANG_OVERRIDE
    {
        return Super::getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::specialize(
            specializationArgs,
            specializationArgCount,
            outSpecializedComponentType,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) SLANG_OVERRIDE
    {
        return Super::renameEntryPoint(newName, outEntryPoint);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::link(outLinkedComponentType, outDiagnostics);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL linkWithOptions(
        slang::IComponentType** outLinkedComponentType,
        uint32_t count,
        slang::CompilerOptionEntry* entries,
        ISlangBlob** outDiagnostics) override
    {
        return Super::linkWithOptions(outLinkedComponentType, count, entries, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointHostCallable(
            entryPointIndex,
            targetIndex,
            outSharedLibrary,
            outDiagnostics);
    }

    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outHash) SLANG_OVERRIDE
    {
        return Super::getEntryPointHash(entryPointIndex, targetIndex, outHash);
    }

    virtual void buildHash(DigestBuilder<SHA1>& builder) SLANG_OVERRIDE;

    /// Create an entry point that refers to the given function.
    static RefPtr<EntryPoint> create(
        Linkage* linkage,
        DeclRef<FuncDecl> funcDeclRef,
        Profile profile);

    /// Get the function decl-ref, including any generic arguments.
    DeclRef<FuncDecl> getFuncDeclRef() { return m_funcDeclRef; }

    /// Get the function declaration (without generic arguments).
    FuncDecl* getFuncDecl() { return m_funcDeclRef.getDecl(); }

    /// Get the name of the entry point
    Name* getName() { return m_name; }

    /// Get the profile associated with the entry point
    ///
    /// Note: only the stage part of the profile is expected
    /// to contain useful data, but certain legacy code paths
    /// allow for "shader model" information to come via this path.
    ///
    Profile getProfile() { return m_profile; }

    /// Get the stage that the entry point is for.
    Stage getStage() { return m_profile.getStage(); }

    /// Get the module that contains the entry point.
    Module* getModule();

    /// Get a list of modules that this entry point depends on.
    ///
    /// This will include the module that defines the entry point (see `getModule()`),
    /// but may also include modules that are required by its generic type arguments.
    ///
    List<Module*> const& getModuleDependencies()
        SLANG_OVERRIDE; // { return getModule()->getModuleDependencies(); }
    List<SourceFile*> const& getFileDependencies()
        SLANG_OVERRIDE; // { return getModule()->getFileDependencies(); }

    /// Create a dummy `EntryPoint` that is only usable for pass-through compilation.
    static RefPtr<EntryPoint> createDummyForPassThrough(
        Linkage* linkage,
        Name* name,
        Profile profile);

    /// Create a dummy `EntryPoint` that stands in for a serialized entry point
    static RefPtr<EntryPoint> createDummyForDeserialize(
        Linkage* linkage,
        Name* name,
        Profile profile,
        String mangledName);

    /// Get the number of existential type parameters for the entry point.
    SLANG_NO_THROW Index SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE;

    /// Get the existential type parameter at `index`.
    SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE;

    Index getRequirementCount() SLANG_OVERRIDE;
    RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

    SpecializationParams const& getExistentialSpecializationParams()
    {
        return m_existentialSpecializationParams;
    }

    Index getGenericSpecializationParamCount() { return m_genericSpecializationParams.getCount(); }
    Index getExistentialSpecializationParamCount()
    {
        return m_existentialSpecializationParams.getCount();
    }

    /// Get an array of all entry-point shader parameters.
    List<ShaderParamInfo> const& getShaderParams() { return m_shaderParams; }

    Index getEntryPointCount() SLANG_OVERRIDE { return 1; };
    RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return this;
    }
    String getEntryPointMangledName(Index index) SLANG_OVERRIDE;
    String getEntryPointNameOverride(Index index) SLANG_OVERRIDE;

    Index getShaderParamCount() SLANG_OVERRIDE { return 0; }
    ShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return ShaderParamInfo();
    }

    class EntryPointSpecializationInfo : public SpecializationInfo
    {
    public:
        DeclRef<FuncDecl> specializedFuncDeclRef;
        List<ExpandedSpecializationArg> existentialSpecializationArgs;
    };

    SLANG_NO_THROW slang::FunctionReflection* SLANG_MCALL getFunctionReflection() SLANG_OVERRIDE
    {
        return (slang::FunctionReflection*)m_funcDeclRef.declRefBase;
    }

protected:
    void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
        SLANG_OVERRIDE;

    RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink) SLANG_OVERRIDE;

private:
    EntryPoint(Linkage* linkage, Name* name, Profile profile, DeclRef<FuncDecl> funcDeclRef);

    void _collectGenericSpecializationParamsRec(Decl* decl);
    void _collectShaderParams();

    // The name of the entry point function (e.g., `main`)
    //
    Name* m_name = nullptr;

    // The declaration of the entry-point function itself.
    //
    DeclRef<FuncDecl> m_funcDeclRef;

    /// The mangled name of the entry point function
    String m_mangledName;

    SpecializationParams m_genericSpecializationParams;
    SpecializationParams m_existentialSpecializationParams;

    /// Information about entry-point parameters
    List<ShaderParamInfo> m_shaderParams;

    // The profile that the entry point will be compiled for
    // (this is a combination of the target stage, and also
    // a feature level that sets capabilities)
    //
    // Note: the profile-version part of this should probably
    // be moving towards deprecation, in favor of the version
    // information (e.g., "Shader Model 5.1") always coming
    // from the target, while the stage part is all that is
    // intrinsic to the entry point.
    //
    Profile m_profile;
};

} // namespace Slang
