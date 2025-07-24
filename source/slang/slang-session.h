// slang-session.h
#pragma once

//
// This file declares the `Linkage` class, which implements
// the `slang::ISession` interface from the public API.
//
// TODO: there is an unfortunate and confusing situation
// where the public Slang API `ISession` type is implemented
// by the internal `Linkage` class, while the internal
// `Session` class implements the `IGlobalSession` interface
// from the public API.
//

#include "../compiler-core/slang-artifact.h"
#include "../compiler-core/slang-command-line-args.h"
#include "../compiler-core/slang-include-system.h"
#include "../compiler-core/slang-name.h"
#include "../core/slang-riff.h"
#include "../core/slang-smart-pointer.h"
#include "slang-ast-base.h"
#include "slang-compiler-fwd.h"
#include "slang-compiler-options.h"
#include "slang-content-assist-info.h"
#include "slang-global-session.h"

#include <slang.h>

namespace Slang
{

/// A dictionary of modules to be considered when resolving `import`s,
/// beyond those that would normally be found through a `Linkage`.
///
/// Checking of an `import` declaration will bottleneck through
/// `Linkage::findOrImportModule`, which would usually just check for
/// any module that had been previously loaded into the same `Linkage`
/// (e.g., by a call to `Linkage::loadModule()`).
///
/// In the case where compilation is being done through an
/// explicit `FrontEndCompileRequest` or `EndToEndCompileRequest`,
/// the modules being compiled by that request do not get added to
/// the surrounding `Linkage`.
///
/// There is a corner case when an explicit compile request has
/// multiple `TranslationUnitRequest`s, because the user (reasonably)
/// expects that if they compile `A.slang` and `B.slang` as two
/// distinct translation units in the same compile request, then
/// an `import B` inside of `A.slang` should resolve to reference
/// the code of `B.slang`. But because neither `A` nor `B` gets
/// added to the `Linkage`, and the `Linkage` is what usually
/// determines what is or isn't loaded, that intuition will
/// be wrong, without a bit of help.
///
/// The `LoadedModuleDictionary` is thus filled in by a
/// `FrontEndCompileRequest` to collect the modules it is compiling,
/// so that they can cross-reference one another (albeit with
/// a current implementation restriction that modules in the
/// request can only `import` those earlier in the request...).
///
/// The dictionary then gets passed around between nearly all of
/// the operations that deal with loading modules, to make sure
/// that they can detect a previously loaded module.
///
typedef Dictionary<Name*, Module*> LoadedModuleDictionary;

enum class ModuleBlobType
{
    Source,
    IR
};

struct ContainerTypeKey
{
    slang::TypeReflection* elementType;
    slang::ContainerType containerType;
    bool operator==(ContainerTypeKey other) const
    {
        return elementType == other.elementType && containerType == other.containerType;
    }
    Slang::HashCode getHashCode() const
    {
        return Slang::combineHash(
            Slang::getHashCode(elementType),
            Slang::getHashCode(containerType));
    }
};

/// A context for loading and re-using code modules.
class Linkage : public RefObject, public slang::ISession
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    CompilerOptionSet m_optionSet;

    ISlangUnknown* getInterface(const Guid& guid);

    SLANG_NO_THROW slang::IGlobalSession* SLANG_MCALL getGlobalSession() override;
    SLANG_NO_THROW slang::IModule* SLANG_MCALL
    loadModule(const char* moduleName, slang::IBlob** outDiagnostics = nullptr) override;
    slang::IModule* loadModuleFromBlob(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        ModuleBlobType blobType,
        slang::IBlob** outDiagnostics = nullptr);
    SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromIRBlob(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL loadModuleInfoFromIRBlob(
        slang::IBlob* source,
        SlangInt& outModuleVersion,
        const char*& outModuleCompilerVersion,
        const char*& outModuleName) override;
    SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromSource(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromSourceString(
        const char* moduleName,
        const char* path,
        const char* string,
        slang::IBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL createCompositeComponentType(
        slang::IComponentType* const* componentTypes,
        SlangInt componentTypeCount,
        slang::IComponentType** outCompositeComponentType,
        ISlangBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL specializeType(
        slang::TypeReflection* type,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ISlangBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getTypeLayout(
        slang::TypeReflection* type,
        SlangInt targetIndex = 0,
        slang::LayoutRules rules = slang::LayoutRules::Default,
        ISlangBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics = nullptr) override;
    SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL getDynamicType() override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getTypeRTTIMangledName(slang::TypeReflection* type, ISlangBlob** outNameBlob) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t* outId) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL getDynamicObjectRTTIBytes(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t* outBytes,
        uint32_t bufferSize) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL createTypeConformanceComponentType(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        slang::ITypeConformance** outConformance,
        SlangInt conformanceIdOverride,
        ISlangBlob** outDiagnostics) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompileRequest(SlangCompileRequest** outCompileRequest) override;
    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getLoadedModuleCount() override;
    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL getLoadedModule(SlangInt index) override;
    virtual SLANG_NO_THROW bool SLANG_MCALL
    isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob) override;

    // Updates the supplied builder with linkage-related information, which includes preprocessor
    // defines, the compiler version, and other compiler options. This is then merged with the hash
    // produced for the program to produce a key that can be used with the shader cache.
    void buildHash(DigestBuilder<SHA1>& builder, SlangInt targetIndex = -1);

    void addTarget(slang::TargetDesc const& desc);
    SlangResult addSearchPath(char const* path);
    SlangResult addPreprocessorDefine(char const* name, char const* value);
    SlangResult setMatrixLayoutMode(SlangMatrixLayoutMode mode);
    /// Create an initially-empty linkage
    Linkage(Session* session, ASTBuilder* astBuilder, Linkage* builtinLinkage);

    /// Dtor
    ~Linkage();

    bool isInLanguageServer()
    {
        return contentAssistInfo.checkingMode != ContentAssistCheckingMode::None;
    }

    /// Get the parent session for this linkage
    Session* getSessionImpl() { return m_session; }

    // Information on the targets we are being asked to
    // generate code for.
    List<RefPtr<TargetRequest>> targets;

    // Directories to search for `#include` files or `import`ed modules
    SearchDirectoryList& getSearchDirectories();

    // Source manager to help track files loaded
    SourceManager m_defaultSourceManager;
    SourceManager* m_sourceManager = nullptr;
    RefPtr<CommandLineContext> m_cmdLineContext;

    // Used to store strings returned by the api as const char*
    StringSlicePool m_stringSlicePool;

    // Name pool for looking up names
    NamePool* namePool = nullptr;

    NamePool* getNamePool() { return namePool; }

    ASTBuilder* getASTBuilder() { return m_astBuilder; }

    RefPtr<ASTBuilder> m_astBuilder;

    // Cache for container types.
    Dictionary<ContainerTypeKey, Type*> m_containerTypes;

    // cache used by type checking, implemented in check.cpp
    TypeCheckingCache* getTypeCheckingCache();
    void destroyTypeCheckingCache();

    RefPtr<RefObject> m_typeCheckingCache = nullptr;

    // Modules that have been dynamically loaded via `import`
    //
    // This is a list of unique modules loaded, in the order they were encountered.
    List<RefPtr<LoadedModule>> loadedModulesList;

    // Map from the path (or uniqueIdentity if available) of a module file to its definition
    Dictionary<String, RefPtr<LoadedModule>> mapPathToLoadedModule;

    // Map from the logical name of a module to its definition
    Dictionary<Name*, RefPtr<LoadedModule>> mapNameToLoadedModules;

    // Map from the mangled name of RTTI objects to sequential IDs
    // used by `switch`-based dynamic dispatch.
    Dictionary<String, uint32_t> mapMangledNameToRTTIObjectIndex;

    // Counters for allocating sequential IDs to witness tables conforming to each interface type.
    Dictionary<String, uint32_t> mapInterfaceMangledNameToSequentialIDCounters;

    SearchDirectoryList searchDirectoryCache;

    // The resulting specialized IR module for each entry point request
    List<RefPtr<IRModule>> compiledModules;

    ContentAssistInfo contentAssistInfo;

    /// File system implementation to use when loading files from disk.
    ///
    /// If this member is `null`, a default implementation that tries
    /// to use the native OS filesystem will be used instead.
    ///
    ComPtr<ISlangFileSystem> m_fileSystem;

    /// The extended file system implementation. Will be set to a default implementation
    /// if fileSystem is nullptr. Otherwise it will either be fileSystem's interface,
    /// or a wrapped impl that makes fileSystem operate as fileSystemExt
    ComPtr<ISlangFileSystemExt> m_fileSystemExt;

    /// Get the currenly set file system
    ISlangFileSystemExt* getFileSystemExt() { return m_fileSystemExt; }

    /// Load a file into memory using the configured file system.
    ///
    /// @param path The path to attempt to load from
    /// @param outBlob A destination pointer to receive the loaded blob
    /// @returns A `SlangResult` to indicate success or failure.
    ///
    SlangResult loadFile(String const& path, PathInfo& outPathInfo, ISlangBlob** outBlob);

    Expr* parseTermString(String str, Scope* scope);

    Type* specializeType(
        Type* unspecializedType,
        Int argCount,
        Type* const* args,
        DiagnosticSink* sink);

    /// Add a new target and return its index.
    UInt addTarget(CodeGenTarget target);

    /// "Bottleneck" routine for loading a module.
    ///
    /// All attempts to load a module, whether through
    /// Slang API calls, `import` operations, or other
    /// means, should bottleneck through `loadModuleImpl`,
    /// or one of the specialized cases `loadSourceModuleImpl`
    /// and `loadBinaryModuleImpl`.
    ///
    RefPtr<Module> loadModuleImpl(
        Name* name,
        const PathInfo& filePathInfo,
        ISlangBlob* fileContentsBlob,
        SourceLoc const& loc,
        DiagnosticSink* sink,
        const LoadedModuleDictionary* additionalLoadedModules,
        ModuleBlobType blobType);

    RefPtr<Module> loadSourceModuleImpl(
        Name* name,
        const PathInfo& filePathInfo,
        ISlangBlob* fileContentsBlob,
        SourceLoc const& loc,
        DiagnosticSink* sink,
        const LoadedModuleDictionary* additionalLoadedModules);

    RefPtr<Module> loadBinaryModuleImpl(
        Name* name,
        const PathInfo& filePathInfo,
        ISlangBlob* fileContentsBlob,
        SourceLoc const& loc,
        DiagnosticSink* sink);

    /// Either finds a previously-loaded module matching what
    /// was serialized into `moduleChunk`, or else attempts
    /// to load the serialized module.
    ///
    /// If a previously-loaded module is found that matches the
    /// name or path information in `moduleChunk`, then that
    /// previously-loaded module is returned.
    ///
    /// Othwerise, attempts to load a module from `moduleChunk`
    /// and, if successful, returns the freshly loaded module.
    ///
    /// Otherwise, return null.
    ///
    RefPtr<Module> findOrLoadSerializedModuleForModuleLibrary(
        ISlangBlob* blobHoldingSerializedData,
        ModuleChunk const* moduleChunk,
        RIFF::ListChunk const* libraryChunk,
        DiagnosticSink* sink);

    RefPtr<Module> loadSerializedModule(
        Name* moduleName,
        const PathInfo& moduleFilePathInfo,
        ISlangBlob* blobHoldingSerializedData,
        ModuleChunk const* moduleChunk,
        RIFF::ListChunk const* containerChunk, //< The outer container, if there is one.
        SourceLoc const& requestingLoc,
        DiagnosticSink* sink);

    SlangResult loadSerializedModuleContents(
        Module* module,
        const PathInfo& moduleFilePathInfo,
        ISlangBlob* blobHoldingSerializedData,
        ModuleChunk const* moduleChunk,
        RIFF::ListChunk const* containerChunk, //< The outer container, if there is one.
        DiagnosticSink* sink);

    SourceFile* loadSourceFile(String pathFrom, String path);

    void loadParsedModule(
        RefPtr<FrontEndCompileRequest> compileRequest,
        RefPtr<TranslationUnitRequest> translationUnit,
        Name* name,
        PathInfo const& pathInfo);

    bool isBinaryModuleUpToDate(String fromPath, RIFF::ListChunk const* baseChunk);

    RefPtr<Module> findOrImportModule(
        Name* name,
        SourceLoc const& loc,
        DiagnosticSink* sink,
        const LoadedModuleDictionary* loadedModules = nullptr);

    SourceFile* findFile(Name* name, SourceLoc loc, IncludeSystem& outIncludeSystem);
    struct IncludeResult
    {
        FileDecl* fileDecl;
        bool isNew;
    };
    IncludeResult findAndIncludeFile(
        Module* module,
        TranslationUnitRequest* translationUnit,
        Name* name,
        SourceLoc const& loc,
        DiagnosticSink* sink);

    SourceManager* getSourceManager() { return m_sourceManager; }

    /// Override the source manager for the linkage.
    ///
    /// This is only used to install a temporary override when
    /// parsing stuff from strings (where we don't want to retain
    /// full source files for the parsed result).
    ///
    /// TODO: We should remove the need for this hack.
    ///
    void setSourceManager(SourceManager* sourceManager) { m_sourceManager = sourceManager; }

    void setRequireCacheFileSystem(bool requireCacheFileSystem);

    void setFileSystem(ISlangFileSystem* fileSystem);

    DeclRef<Decl> specializeGeneric(
        DeclRef<Decl> declRef,
        List<Expr*> argExprs,
        DiagnosticSink* sink);

    DeclRef<Decl> specializeWithArgTypes(
        Expr* funcExpr,
        List<Type*> argTypes,
        DiagnosticSink* sink);

    bool isSpecialized(DeclRef<Decl> declRef);

    DiagnosticSink::Flags diagnosticSinkFlags = 0;

    bool m_requireCacheFileSystem = false;

    // Modules that have been read in with the -r option
    List<ComPtr<IArtifact>> m_libModules;

    void _stopRetainingParentSession() { m_retainedSession = nullptr; }

    // Get shared semantics information for reflection purposes.
    SharedSemanticsContext* getSemanticsForReflection();

private:
    /// The global Slang library session that this linkage is a child of
    Session* m_session = nullptr;

    RefPtr<Session> m_retainedSession;

    /// Tracks state of modules currently being loaded.
    ///
    /// This information is used to diagnose cases where
    /// a user tries to recursively import the same module
    /// (possibly along a transitive chain of `import`s).
    ///
    struct ModuleBeingImportedRAII
    {
    public:
        ModuleBeingImportedRAII(
            Linkage* linkage,
            Module* module,
            Name* name,
            SourceLoc const& importLoc)
            : linkage(linkage), module(module), name(name), importLoc(importLoc)
        {
            next = linkage->m_modulesBeingImported;
            linkage->m_modulesBeingImported = this;
        }

        ~ModuleBeingImportedRAII() { linkage->m_modulesBeingImported = next; }

        Linkage* linkage;
        Module* module;
        Name* name;
        SourceLoc importLoc;
        ModuleBeingImportedRAII* next;
    };

    // Any modules currently being imported will be listed here
    ModuleBeingImportedRAII* m_modulesBeingImported = nullptr;

    /// Is the given module in the middle of being imported?
    bool isBeingImported(Module* module);

    /// Diagnose that an error occured in the process of importing a module
    void _diagnoseErrorInImportedModule(DiagnosticSink* sink);

    List<Type*> m_specializedTypes;

    RefPtr<SharedSemanticsContext> m_semanticsForReflection;
};
} // namespace Slang
