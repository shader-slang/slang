// slang-linkable.h
#pragma once

//
// This file defines the `ComponentType` class, which
// provides the root of the hierarchy for classes
// that represent units of linkable code.
//
// The most obvious case of linkable code is a single
// `Module` produced by invoking the Slang front-end
// on source code (or by loading a previously compiled
// `.slang-module` file).
//

#include "../compiler-core/slang-artifact.h"
#include "slang-ast-base.h"
#include "slang-compiler-fwd.h"
#include "slang-compiler-options.h"

#include <slang-com-helper.h>
#include <slang.h>

namespace Slang
{
class Linkage;

class EntryPoint;

class ComponentType;
class ComponentTypeVisitor;

/// Information collected about global or entry-point shader parameters
struct ShaderParamInfo
{
    DeclRef<VarDeclBase> paramDeclRef;
    Int firstSpecializationParamIndex = 0;
    Int specializationParamCount = 0;
};

/// Tracks an ordered list of modules that something depends on.
/// TODO: Shader caching currently relies on this being in well defined order.
struct ModuleDependencyList
{
public:
    /// Get the list of modules that are depended on.
    List<Module*> const& getModuleList() { return m_moduleList; }

    /// Add a module and everything it depends on to the list.
    void addDependency(Module* module);

    /// Add a module to the list, but not the modules it depends on.
    void addLeafDependency(Module* module);

private:
    void _addDependency(Module* module);

    List<Module*> m_moduleList;
    HashSet<Module*> m_moduleSet;
};

/// Tracks an unordered list of source files that something depends on
/// TODO: Shader caching currently relies on this being in well defined order.
struct FileDependencyList
{
public:
    /// Get the list of files that are depended on.
    List<SourceFile*> const& getFileList() { return m_fileList; }

    /// Add a file to the list, if it is not already present
    void addDependency(SourceFile* sourceFile);

    /// Add all of the paths that `module` depends on to the list
    void addDependency(Module* module);

    void clear()
    {
        m_fileList.clear();
        m_fileSet.clear();
    }

private:
    // TODO: We are using a `HashSet` here to deduplicate
    // the paths so that we don't return the same path
    // multiple times from `getFilePathList`, but because
    // order isn't important, we could potentially do better
    // in terms of memory (at some cost in performance) by
    // just sorting the `m_fileList` every once in
    // a while and then deduplicating.

    List<SourceFile*> m_fileList;
    HashSet<SourceFile*> m_fileSet;
};

/// Base class for "component types" that represent the pieces a final
/// shader program gets linked together from.
///
class ComponentType : public RefObject,
                      public slang::IComponentType,
                      public slang::IComponentType2,
                      public slang::IModulePrecompileService_Experimental
{
public:
    //
    // ISlangUnknown interface
    //

    SLANG_REF_OBJECT_IUNKNOWN_ALL;
    ISlangUnknown* getInterface(Guid const& guid);

    //
    // slang::IComponentType interface
    //

    SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() SLANG_OVERRIDE;
    SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, slang::IBlob** outDiagnostics) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE;

    IArtifact* getTargetArtifact(SlangInt targetIndex, slang::IBlob** outDiagnostics);

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics = nullptr) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointMetadata(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        slang::IBlob** outDiagnostics = nullptr) SLANG_OVERRIDE;

    SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) SLANG_OVERRIDE;

    SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL link(
        slang::IComponentType** outLinkedComponentType,
        ISlangBlob** outDiagnostics) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE;

    /// ComponentType is the only class inheriting from IComponentType that provides a
    /// meaningful implementation for this function. All others should forward these and
    /// implement `buildHash`.
    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outHash) SLANG_OVERRIDE;

    SLANG_NO_THROW SlangResult SLANG_MCALL linkWithOptions(
        slang::IComponentType** outLinkedComponentType,
        uint32_t count,
        slang::CompilerOptionEntry* entries,
        ISlangBlob** outDiagnostics) override;

    //
    // slang::IComponentType2 interface
    //

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCompileResult(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCompileResult(
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics = nullptr) SLANG_OVERRIDE;

    //
    // slang::IModulePrecompileService interface
    //
    SLANG_NO_THROW SlangResult SLANG_MCALL
    precompileForTarget(SlangCompileTarget target, slang::IBlob** outDiagnostics) SLANG_OVERRIDE;

    SLANG_NO_THROW SlangResult SLANG_MCALL getPrecompiledTargetCode(
        SlangCompileTarget target,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics = nullptr) SLANG_OVERRIDE;

    SLANG_NO_THROW SlangInt SLANG_MCALL getModuleDependencyCount() SLANG_OVERRIDE;

    SLANG_NO_THROW SlangResult SLANG_MCALL getModuleDependency(
        SlangInt dependencyIndex,
        slang::IModule** outModule,
        slang::IBlob** outDiagnostics = nullptr) SLANG_OVERRIDE;

    CompilerOptionSet& getOptionSet() { return m_optionSet; }

    /// Get the linkage (aka "session" in the public API) for this component type.
    Linkage* getLinkage() { return m_linkage; }

    /// Get the target-specific version of this program for the given `target`.
    ///
    /// The `target` must be a target on the `Linkage` that was used to create this program.
    TargetProgram* getTargetProgram(TargetRequest* target);

    /// Update the hash builder with the dependencies for this component type.
    virtual void buildHash(DigestBuilder<SHA1>& builder) = 0;

    /// Get the number of entry points linked into this component type.
    virtual Index getEntryPointCount() = 0;

    /// Get one of the entry points linked into this component type.
    virtual RefPtr<EntryPoint> getEntryPoint(Index index) = 0;

    /// Get the mangled name of one of the entry points linked into this component type.
    virtual String getEntryPointMangledName(Index index) = 0;

    /// Get the name override of one of the entry points linked into this component type.
    virtual String getEntryPointNameOverride(Index index) = 0;

    /// Get the number of global shader parameters linked into this component type.
    virtual Index getShaderParamCount() = 0;

    /// Get one of the global shader parametesr linked into this component type.
    virtual ShaderParamInfo getShaderParam(Index index) = 0;

    /// Get the specialization parameter at `index`.
    virtual SpecializationParam const& getSpecializationParam(Index index) = 0;

    /// Get the number of "requirements" that this component type has.
    ///
    /// A requirement represents another component type that this component
    /// needs in order to function correctly. For example, the dependency
    /// of one module on another module that it `import`s is represented
    /// as a requirement, as is the dependency of an entry point on the
    /// module that defines it.
    ///
    virtual Index getRequirementCount() = 0;

    /// Get the requirement at `index`.
    virtual RefPtr<ComponentType> getRequirement(Index index) = 0;

    /// Parse a type from a string, in the context of this component type.
    ///
    /// Any names in the string will be resolved using the modules
    /// referenced by the program.
    ///
    /// On an error, returns null and reports diagnostic messages
    /// to the provided `sink`.
    ///
    /// TODO: This function shouldn't be on the base class, since
    /// it only really makes sense on `Module`.
    ///
    Type* getTypeFromString(String const& typeStr, DiagnosticSink* sink);
    Expr* parseExprFromString(String expr, DiagnosticSink* sink);
    Expr* findDeclFromString(String const& name, DiagnosticSink* sink);
    Expr* tryResolveOverloadedExpr(Expr* exprIn);

    Expr* findDeclFromStringInType(
        Type* type,
        String const& name,
        LookupMask mask,
        DiagnosticSink* sink);

    bool isSubType(Type* subType, Type* superType);

    Dictionary<String, IntVal*>& getMangledNameToIntValMap();
    ConstantIntVal* tryFoldIntVal(IntVal* intVal);

    /// Get a list of modules that this component type depends on.
    ///
    virtual List<Module*> const& getModuleDependencies() = 0;

    /// Get the full list of source files this component type depends on.
    ///
    virtual List<SourceFile*> const& getFileDependencies() = 0;

    /// Callback for use with `enumerateIRModules`
    typedef void (*EnumerateIRModulesCallback)(IRModule* irModule, void* userData);

    /// Invoke `callback` on all the IR modules that are (transitively) linked into this component
    /// type.
    void enumerateIRModules(EnumerateIRModulesCallback callback, void* userData);

    /// Invoke `callback` on all the IR modules that are (transitively) linked into this component
    /// type.
    template<typename F>
    void enumerateIRModules(F const& callback)
    {
        struct Helper
        {
            static void helper(IRModule* irModule, void* userData) { (*(F*)userData)(irModule); }
        };
        enumerateIRModules(&Helper::helper, (void*)&callback);
    }

    /// Callback for use with `enumerateModules`
    typedef void (*EnumerateModulesCallback)(Module* module, void* userData);

    /// Invoke `callback` on all the modules that are (transitively) linked into this component
    /// type.
    void enumerateModules(EnumerateModulesCallback callback, void* userData);

    /// Invoke `callback` on all the modules that are (transitively) linked into this component
    /// type.
    template<typename F>
    void enumerateModules(F const& callback)
    {
        struct Helper
        {
            static void helper(Module* module, void* userData) { (*(F*)userData)(module); }
        };
        enumerateModules(&Helper::helper, (void*)&callback);
    }

    /// Side-band information generated when specializing this component type.
    ///
    /// Difference subclasses of `ComponentType` are expected to create their
    /// own subclass of `SpecializationInfo` as the output of `_validateSpecializationArgs`.
    /// Later, whenever we want to use a specialized component type we will
    /// also have the `SpecializationInfo` available and will expect it to
    /// have the correct (subclass-specific) type.
    ///
    class SpecializationInfo : public RefObject
    {
    };

    /// Validate the given specialization `args` and compute any side-band specialization info.
    ///
    /// Any errors will be reported to `sink`, which can thus be used to test
    /// if the operation was successful.
    ///
    /// A null return value is allowed, since not all subclasses require
    /// custom side-band specialization information.
    ///
    /// This function is an implementation detail of `specialize()`.
    ///
    virtual RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink) = 0;

    /// Validate the given specialization `args` and compute any side-band specialization info.
    ///
    /// Any errors will be reported to `sink`, which can thus be used to test
    /// if the operation was successful.
    ///
    /// A null return value is allowed, since not all subclasses require
    /// custom side-band specialization information.
    ///
    /// This function is an implementation detail of `specialize()`.
    ///
    RefPtr<SpecializationInfo> _validateSpecializationArgs(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink)
    {
        if (argCount == 0)
            return nullptr;
        return _validateSpecializationArgsImpl(args, argCount, outConsumedArgCount, sink);
    }

    /// Specialize this component type given `specializationArgs`
    ///
    /// Any diagnostics will be reported to `sink`, which can be used
    /// to determine if the operation was successful. It is allowed
    /// for this operation to have a non-null return even when an
    /// error is ecnountered.
    ///
    RefPtr<ComponentType> specialize(
        SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        DiagnosticSink* sink);

    /// Invoke `visitor` on this component type, using the appropriate dynamic type.
    ///
    /// This function implements the "visitor pattern" for `ComponentType`.
    ///
    /// If the `specializationInfo` argument is non-null, it must be specialization
    /// information generated for this specific component type by `_validateSpecializationArgs`.
    /// In that case, appropriately-typed specialization information will be passed
    /// when invoking the `visitor`.
    ///
    virtual void acceptVisitor(
        ComponentTypeVisitor* visitor,
        SpecializationInfo* specializationInfo) = 0;

    /// Create a scope suitable for looking up names or parsing specialization arguments.
    ///
    /// This facility is only needed to support legacy APIs for string-based lookup
    /// and parsing via Slang reflection, and is not recommended for future APIs to use.
    ///
    Scope* _getOrCreateScopeForLegacyLookup(ASTBuilder* astBuilder);

protected:
    ComponentType(Linkage* linkage);

protected:
    Linkage* m_linkage;

    CompilerOptionSet m_optionSet;

    // Cache of target-specific programs for each target.
    Dictionary<TargetRequest*, RefPtr<TargetProgram>> m_targetPrograms;

    // Any types looked up dynamically using `getTypeFromString`
    //
    // TODO: Remove this. Type lookup should only be supported on `Module`s.
    //
    Dictionary<String, Type*> m_types;

    // Any decls looked up dynamically using `findDeclFromString`.
    Dictionary<String, Expr*> m_decls;

    Scope* m_lookupScope = nullptr;
    std::unique_ptr<Dictionary<String, IntVal*>> m_mapMangledNameToIntVal;

    Dictionary<Int, ComPtr<IArtifact>> m_targetArtifacts;
};

} // namespace Slang
