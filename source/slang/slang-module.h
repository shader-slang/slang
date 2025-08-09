// slang-module.h
#pragma once

//
// This file provides the `Module` class, which is
// central to many parts of the Slang compiler codebase.
//

#include "../core/slang-string-util.h"
#include "slang-ast-builder.h"
#include "slang-entry-point.h"
#include "slang-linkable.h"

namespace Slang
{

/// A module of code that has been compiled through the front-end
///
/// A module comprises all the code from one translation unit (which
/// may span multiple Slang source files), and provides access
/// to both the AST and IR representations of that code.
///
/// This class serves multiple important roles in the Slang compiler:
///
/// * this class implements the `slang::IModule` interface from
///   the public Slang API.
///
/// * this class is the primary output of front-end compilation,
///   and its data is what gets stored/loaded using the `.slang-module`
///   file format.
///
/// * The checked AST in a `Module` provides all of the information
///   that the front-end uses when checking code that `import`s
///   that module (e.g., the names and signatures of functions defined
///   in the module).
///
/// * The checked AST is also used to service queries through the
///   Slang reflection API.
///
/// * the `Module` class is a subclass of `ComponentType` and thus
///   is a unit of linkable code. One or more modules (and other
///   linkable objects) can be combined to form a linked program.
///
/// * The Slang IR in a `Module` provides all of the information that
///   the back-end uses when generating code for a program/binary
///   that links this module (or any of its entry points).
///
class Module : public ComponentType, public slang::IModule
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

    SLANG_NO_THROW SlangResult SLANG_MCALL
    findEntryPointByName(char const* name, slang::IEntryPoint** outEntryPoint) SLANG_OVERRIDE
    {
        if (outEntryPoint == nullptr)
        {
            return SLANG_E_INVALID_ARG;
        }
        SLANG_AST_BUILDER_RAII(m_astBuilder);
        ComPtr<slang::IEntryPoint> entryPoint(findEntryPointByName(UnownedStringSlice(name)));
        if ((!entryPoint))
            return SLANG_FAIL;

        *outEntryPoint = entryPoint.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL findAndCheckEntryPoint(
        char const* name,
        SlangStage stage,
        slang::IEntryPoint** outEntryPoint,
        ISlangBlob** outDiagnostics) override
    {
        if (outEntryPoint == nullptr)
        {
            return SLANG_E_INVALID_ARG;
        }
        ComPtr<slang::IEntryPoint> entryPoint(
            findAndCheckEntryPoint(UnownedStringSlice(name), stage, outDiagnostics));
        if ((!entryPoint))
            return SLANG_FAIL;

        *outEntryPoint = entryPoint.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW SlangInt32 SLANG_MCALL getDefinedEntryPointCount() override
    {
        return (SlangInt32)m_entryPoints.getCount();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getDefinedEntryPoint(SlangInt32 index, slang::IEntryPoint** outEntryPoint) override
    {
        if (index < 0 || index >= m_entryPoints.getCount())
            return SLANG_E_INVALID_ARG;

        if (outEntryPoint == nullptr)
        {
            return SLANG_E_INVALID_ARG;
        }

        ComPtr<slang::IEntryPoint> entryPoint(m_entryPoints[index].Ptr());
        *outEntryPoint = entryPoint.detach();
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL linkWithOptions(
        slang::IComponentType** outLinkedComponentType,
        uint32_t count,
        slang::CompilerOptionEntry* entries,
        ISlangBlob** outDiagnostics) override
    {
        return Super::linkWithOptions(outLinkedComponentType, count, entries, outDiagnostics);
    }
    //

    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outHash) SLANG_OVERRIDE
    {
        return Super::getEntryPointHash(entryPointIndex, targetIndex, outHash);
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

    /// Get a serialized representation of the checked module.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    serialize(ISlangBlob** outSerializedBlob) override;

    /// Write the serialized representation of this module to a file.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL writeToFile(char const* fileName) override;

    /// Get the name of the module.
    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() override;

    /// Get the path of the module.
    virtual SLANG_NO_THROW const char* SLANG_MCALL getFilePath() override;

    /// Get the unique identity of the module.
    virtual SLANG_NO_THROW const char* SLANG_MCALL getUniqueIdentity() override;

    /// Get the number of dependency files that this module depends on.
    /// This includes both the explicit source files, as well as any
    /// additional files that were transitively referenced (e.g., via
    /// a `#include` directive).
    virtual SLANG_NO_THROW SlangInt32 SLANG_MCALL getDependencyFileCount() override;

    /// Get the path to a file this module depends on.
    virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(SlangInt32 index) override;


    // IModulePrecompileService_Experimental
    /// Precompile TU to target language
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    precompileForTarget(SlangCompileTarget target, slang::IBlob** outDiagnostics) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPrecompiledTargetCode(
        SlangCompileTarget target,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics = nullptr) override;

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getModuleDependencyCount() SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getModuleDependency(
        SlangInt dependencyIndex,
        slang::IModule** outModule,
        slang::IBlob** outDiagnostics = nullptr) SLANG_OVERRIDE;

    virtual void buildHash(DigestBuilder<SHA1>& builder) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW slang::DeclReflection* SLANG_MCALL getModuleReflection() SLANG_OVERRIDE;

    void setDigest(SHA1::Digest const& digest) { m_digest = digest; }
    SHA1::Digest computeDigest();

    /// Create a module (initially empty).
    Module(Linkage* linkage, ASTBuilder* astBuilder = nullptr);

    /// Get the AST for the module (if it has been parsed)
    ModuleDecl* getModuleDecl() { return m_moduleDecl; }

    /// The the IR for the module (if it has been generated)
    IRModule* getIRModule() { return m_irModule; }

    /// Get the list of other modules this module depends on
    List<Module*> const& getModuleDependencyList()
    {
        return m_moduleDependencyList.getModuleList();
    }

    /// Get the list of files this module depends on
    List<SourceFile*> const& getFileDependencyList() { return m_fileDependencyList.getFileList(); }

    /// Register a module that this module depends on
    void addModuleDependency(Module* module);

    /// Register a source file that this module depends on
    void addFileDependency(SourceFile* sourceFile);

    void clearFileDependency() { m_fileDependencyList.clear(); }
    /// Set the AST for this module.
    ///
    /// This should only be called once, during creation of the module.
    ///
    void setModuleDecl(ModuleDecl* moduleDecl); // { m_moduleDecl = moduleDecl; }

    void setName(String name);
    void setName(Name* name) { m_name = name; }
    Name* getNameObj() { return m_name; }

    void setPathInfo(PathInfo pathInfo) { m_pathInfo = pathInfo; }

    /// Set the IR for this module.
    ///
    /// This should only be called once, during creation of the module.
    ///
    void setIRModule(IRModule* irModule) { m_irModule = irModule; }

    Index getEntryPointCount() SLANG_OVERRIDE { return 0; }
    RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return nullptr;
    }
    String getEntryPointMangledName(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return String();
    }
    String getEntryPointNameOverride(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return String();
    }

    Index getShaderParamCount() SLANG_OVERRIDE { return m_shaderParams.getCount(); }
    ShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE { return m_shaderParams[index]; }

    SLANG_NO_THROW Index SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE
    {
        return m_specializationParams.getCount();
    }
    SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE
    {
        return m_specializationParams[index];
    }

    Index getRequirementCount() SLANG_OVERRIDE;
    RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

    List<Module*> const& getModuleDependencies() SLANG_OVERRIDE
    {
        return m_moduleDependencyList.getModuleList();
    }
    List<SourceFile*> const& getFileDependencies() SLANG_OVERRIDE
    {
        return m_fileDependencyList.getFileList();
    }

    /// Given a mangled name finds the exported NodeBase associated with this module.
    /// If not found returns nullptr.
    Decl* findExportedDeclByMangledName(const UnownedStringSlice& mangledName);

    /// Ensure that the any accelerator(s) used for `findExportedDeclByMangledName`
    /// have already been built.
    ///
    void ensureExportLookupAcceleratorBuilt();

    Count getExportedDeclCount();
    Decl* getExportedDecl(Index index);
    UnownedStringSlice getExportedDeclMangledName(Index index);

    /// Get the ASTBuilder
    ASTBuilder* getASTBuilder() { return m_astBuilder; }

    /// Collect information on the shader parameters of the module.
    ///
    /// This method should only be called once, after the core
    /// structured of the module (its AST and IR) have been created,
    /// and before any of the `ComponentType` APIs are used.
    ///
    /// TODO: We might eventually consider a non-stateful approach
    /// to constructing a `Module`.
    ///
    void _collectShaderParams();

    void _discoverEntryPoints(DiagnosticSink* sink, const List<RefPtr<TargetRequest>>& targets);
    void _discoverEntryPointsImpl(
        ContainerDecl* containerDecl,
        DiagnosticSink* sink,
        const List<RefPtr<TargetRequest>>& targets);


    class ModuleSpecializationInfo : public SpecializationInfo
    {
    public:
        struct GenericArgInfo
        {
            Decl* paramDecl = nullptr;
            Val* argVal = nullptr;
        };

        List<GenericArgInfo> genericArgs;
        List<ExpandedSpecializationArg> existentialArgs;
    };

    RefPtr<EntryPoint> findEntryPointByName(UnownedStringSlice const& name);
    RefPtr<EntryPoint> findAndCheckEntryPoint(
        UnownedStringSlice const& name,
        SlangStage stage,
        ISlangBlob** outDiagnostics);

    List<RefPtr<EntryPoint>>& getEntryPoints() { return m_entryPoints; }
    void _addEntryPoint(EntryPoint* entryPoint);
    void _processFindDeclsExportSymbolsRec(Decl* decl);

    // Gets the files that has been included into the module.
    Dictionary<SourceFile*, FileDecl*>& getIncludedSourceFileMap()
    {
        return m_mapSourceFileToFileDecl;
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
    Name* m_name = nullptr;
    PathInfo m_pathInfo;

    // The AST for the module
    ModuleDecl* m_moduleDecl = nullptr;

    // The IR for the module
    RefPtr<IRModule> m_irModule = nullptr;

    List<ShaderParamInfo> m_shaderParams;
    SpecializationParams m_specializationParams;

    List<Module*> m_requirements;

    // A digest that uniquely identifies the contents of the module.
    SHA1::Digest m_digest;

    // List of modules this module depends on
    ModuleDependencyList m_moduleDependencyList;

    // List of source files this module depends on
    FileDependencyList m_fileDependencyList;

    // Entry points that were defined in this module
    //
    // Note: the entry point defined in the module are *not*
    // part of the memory image/layout of the module when
    // it is considered as an IComponentType. This can be
    // a bit confusing, but if all the entry points in the
    // module were automatically linked into the component
    // type, we'd need a way to access just the global
    // scope of the module without the entry points, in
    // case we wanted to link a single entry point against
    // the global scope. The `Module` type provides exactly
    // that "module without its entry points" unit of
    // granularity for linking.
    //
    // This list only exists for lookup purposes, so that
    // the user can find an existing entry-point function
    // that was defined as part of the module.
    //
    List<RefPtr<EntryPoint>> m_entryPoints;

    // The builder that owns all of the AST nodes from parsing the source of
    // this module.
    RefPtr<ASTBuilder> m_astBuilder;

    // Holds map of exported mangled names to symbols. m_mangledExportPool maps names to indices,
    // and m_mangledExportSymbols holds the NodeBase* values for each index.
    StringSlicePool m_mangledExportPool;
    List<Decl*> m_mangledExportSymbols;

    // Source files that have been pulled into the module with `__include`.
    Dictionary<SourceFile*, FileDecl*> m_mapSourceFileToFileDecl;

public:
    SLANG_NO_THROW SlangResult SLANG_MCALL disassemble(slang::IBlob** outDisassembledBlob) override
    {
        if (!outDisassembledBlob)
            return SLANG_E_INVALID_ARG;
        String disassembly;
        this->getIRModule()->getModuleInst()->dump(disassembly);
        auto blob = StringUtil::createStringBlob(disassembly);
        *outDisassembledBlob = blob.detach();
        return SLANG_OK;
    }
};

} // namespace Slang
