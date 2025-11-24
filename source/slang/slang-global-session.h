// slang-global-session.h
#pragma once

//
// This file provides the `Session` type, and the implementation
// of the `slang::IGlobalSession` interface from the public API.
//
// TODO: there is an unfortunate and confusing situation
// where the public Slang API `ISession` type is implemented
// by the internal `Linkage` class, while the internal
// `Session` class implements the `IGlobalSession` interface
// from the public API.
//

#include "../compiler-core/slang-downstream-compiler-set.h"
#include "../compiler-core/slang-downstream-compiler-util.h"
#include "../compiler-core/slang-downstream-compiler.h"
#include "../compiler-core/slang-spirv-core-grammar.h"
#include "../core/slang-command-options.h"
#include "slang-pass-through.h"
#include "slang-target.h"

namespace Slang
{


class CodeGenTransitionMap
{
public:
    struct Pair
    {
        typedef Pair ThisType;
        SLANG_FORCE_INLINE bool operator==(const ThisType& rhs) const
        {
            return source == rhs.source && target == rhs.target;
        }
        SLANG_FORCE_INLINE bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        SLANG_FORCE_INLINE HashCode getHashCode() const
        {
            return combineHash(HashCode(source), HashCode(target));
        }

        CodeGenTarget source;
        CodeGenTarget target;
    };

    void removeTransition(CodeGenTarget source, CodeGenTarget target)
    {
        m_map.remove(Pair{source, target});
    }
    void addTransition(CodeGenTarget source, CodeGenTarget target, PassThroughMode compiler)
    {
        SLANG_ASSERT(source != target);
        m_map.set(Pair{source, target}, compiler);
    }
    bool hasTransition(CodeGenTarget source, CodeGenTarget target) const
    {
        return m_map.containsKey(Pair{source, target});
    }
    PassThroughMode getTransition(CodeGenTarget source, CodeGenTarget target) const
    {
        const Pair pair{source, target};
        auto value = m_map.tryGetValue(pair);
        return value ? *value : PassThroughMode::None;
    }

protected:
    Dictionary<Pair, PassThroughMode> m_map;
};

/// A global session for interaction with the Slang compiler.
///
/// A `Session` provides a context for ongoing interaction
/// between a client application and the Slang API. Creating
/// a `Session` has an up-front cost that then makes creation
/// of one or more `Linkage`s cheaper.
///
/// The main services provided by a `Session` are:
///
/// * This class implements the `slang::IGobalSession` interface
///   from the public Slang API.
///
/// * The global session provides a scope for a loading built-in
///   modules (including the core module), such that the costs
///   associated with those modules can be amortized across
///   multiple `Linkage`s using the same global session.
///
/// * The global session provides a scope for interactions with
///   the surrounding OS, and application-specific customizations
///   related to it. This includes locating other tools such as
///   downstream compilers, which may be implemented either as
///   executables or shared libraries.
///
/// * The global session provides a scope for application-specific
///   injection of custom source code to be treated like a builtin
///   module, or text to be used as the "prelude" for particular
///   downstream languages/compilers. This functionality should be
///   considered as a legacy and/or deprecated feature.
///
/// * The global session provides various one-off services that
///   are specific to the `slangc` tool or to the build process
///   for Slang itself. Some of these have been exposed through the
///   `slang::IGlobalSession` interface, but should be *not* be
///   used by user applications.
///
class Session : public RefObject, public slang::IGlobalSession
{
public:
    SLANG_COM_INTERFACE(
        0xd6b767eb,
        0xd786,
        0x4343,
        {0x2a, 0x8c, 0x6d, 0xa0, 0x3d, 0x5a, 0xb4, 0x4a})

    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject)
        SLANG_OVERRIDE;
    SLANG_REF_OBJECT_IUNKNOWN_ADD_REF
    SLANG_REF_OBJECT_IUNKNOWN_RELEASE

    // slang::IGlobalSession
    SLANG_NO_THROW SlangResult SLANG_MCALL
    createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override;
    SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const* name) override;
    SLANG_NO_THROW void SLANG_MCALL
    setDownstreamCompilerPath(SlangPassThrough passThrough, char const* path) override;
    SLANG_NO_THROW void SLANG_MCALL
    setDownstreamCompilerPrelude(SlangPassThrough inPassThrough, char const* prelude) override;
    SLANG_NO_THROW void SLANG_MCALL
    getDownstreamCompilerPrelude(SlangPassThrough inPassThrough, ISlangBlob** outPrelude) override;
    SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() override;
    SLANG_NO_THROW SlangResult SLANG_MCALL setDefaultDownstreamCompiler(
        SlangSourceLanguage sourceLanguage,
        SlangPassThrough defaultCompiler) override;
    SLANG_NO_THROW SlangPassThrough SLANG_MCALL
    getDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage) override;

    SLANG_NO_THROW void SLANG_MCALL
    setLanguagePrelude(SlangSourceLanguage inSourceLanguage, char const* prelude) override;
    SLANG_NO_THROW void SLANG_MCALL
    getLanguagePrelude(SlangSourceLanguage inSourceLanguage, ISlangBlob** outPrelude) override;

    SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompileRequest(slang::ICompileRequest** outCompileRequest) override;

    SLANG_NO_THROW void SLANG_MCALL
    addBuiltins(char const* sourcePath, char const* sourceString) override;
    SLANG_NO_THROW void SLANG_MCALL
    setSharedLibraryLoader(ISlangSharedLibraryLoader* loader) override;
    SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL getSharedLibraryLoader() override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    checkCompileTargetSupport(SlangCompileTarget target) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    checkPassThroughSupport(SlangPassThrough passThrough) override;

    void writeCoreModuleDoc(String config);
    SLANG_NO_THROW SlangResult SLANG_MCALL
    compileCoreModule(slang::CompileCoreModuleFlags flags) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    loadCoreModule(const void* coreModule, size_t coreModuleSizeInBytes) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL
    saveCoreModule(SlangArchiveType archiveType, ISlangBlob** outBlob) override;

    SLANG_NO_THROW SlangResult SLANG_MCALL compileBuiltinModule(
        slang::BuiltinModuleName moduleName,
        slang::CompileCoreModuleFlags flags) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL loadBuiltinModule(
        slang::BuiltinModuleName moduleName,
        const void* coreModule,
        size_t coreModuleSizeInBytes) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL saveBuiltinModule(
        slang::BuiltinModuleName moduleName,
        SlangArchiveType archiveType,
        ISlangBlob** outBlob) override;

    SLANG_NO_THROW SlangCapabilityID SLANG_MCALL findCapability(char const* name) override;

    SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerForTransition(
        SlangCompileTarget source,
        SlangCompileTarget target,
        SlangPassThrough compiler) override;
    SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDownstreamCompilerForTransition(
        SlangCompileTarget source,
        SlangCompileTarget target) override;
    SLANG_NO_THROW void SLANG_MCALL
    getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime) override
    {
        *outDownstreamTime = m_downstreamCompileTime;
        *outTotalTime = m_totalCompileTime;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL setSPIRVCoreGrammar(char const* jsonPath) override;

    SLANG_NO_THROW SlangResult SLANG_MCALL parseCommandLineArguments(
        int argc,
        const char* const* argv,
        slang::SessionDesc* outSessionDesc,
        ISlangUnknown** outAllocation) override;

    SLANG_NO_THROW SlangResult SLANG_MCALL
    getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob) override;

    /// Get the downstream compiler for a transition
    IDownstreamCompiler* getDownstreamCompiler(CodeGenTarget source, CodeGenTarget target);

    // This needs to be atomic not because of contention between threads as `Session` is
    // *not* multithreaded, but can be used exclusively on one thread at a time.
    // The need for atomic is purely for visibility. If the session is used on a different
    // thread we need to be sure any changes to m_epochId are visible to this thread.
    std::atomic<Index> m_epochId = 1;

    Scope* baseLanguageScope = nullptr;
    Scope* coreLanguageScope = nullptr;
    Scope* hlslLanguageScope = nullptr;
    Scope* slangLanguageScope = nullptr;
    Scope* glslLanguageScope = nullptr;
    Name* glslModuleName = nullptr;
    Name* neuralModuleName = nullptr;

    ModuleDecl* baseModuleDecl = nullptr;
    List<RefPtr<Module>> coreModules;

    SourceManager builtinSourceManager;

    SourceManager* getBuiltinSourceManager() { return &builtinSourceManager; }

    // Name pool stuff for unique-ing identifiers

    NamePool namePool;

    NamePool* getNamePool() { return &namePool; }
    Name* getNameObj(String name) { return namePool.getName(name); }
    Name* tryGetNameObj(String name) { return namePool.tryGetName(name); }
    //

    /// Get the AST builder associated with this global session.
    ///
    /// This is the AST builder used for builtin modules.
    ///
    RefPtr<ASTBuilder> getASTBuilder() { return m_rootASTBuilder; }

    /// Get the shared AST builder state associated with this global session.
    ///
    /// Equivalent to `this->getASTBuilder()->getSharedASTBuilder()`.
    ///
    SharedASTBuilder* getSharedASTBuilder();

    /// This AST Builder should only be used for creating AST nodes that are global across requests
    /// not doing so could lead to memory being consumed but not used.
    ASTBuilder* getGlobalASTBuilder() { return m_rootASTBuilder; }

    // Generated code for core module, etc.
    String coreModulePath;

    ComPtr<ISlangBlob> coreLibraryCode;
    // ComPtr<ISlangBlob> slangLibraryCode;
    ComPtr<ISlangBlob> hlslLibraryCode;
    ComPtr<ISlangBlob> glslLibraryCode;
    ComPtr<ISlangBlob> autodiffLibraryCode;

    String getCoreModulePath();

    ComPtr<ISlangBlob> getCoreLibraryCode();
    ComPtr<ISlangBlob> getHLSLLibraryCode();
    ComPtr<ISlangBlob> getAutodiffLibraryCode();
    ComPtr<ISlangBlob> getGLSLLibraryCode();

    void getBuiltinModuleSource(StringBuilder& sb, slang::BuiltinModuleName moduleName);

    SPIRVCoreGrammarInfo& getSPIRVCoreGrammarInfo()
    {
        if (!spirvCoreGrammarInfo)
            setSPIRVCoreGrammar(nullptr);
        SLANG_ASSERT(spirvCoreGrammarInfo);
        return *spirvCoreGrammarInfo;
    }
    RefPtr<SPIRVCoreGrammarInfo> spirvCoreGrammarInfo;

    //

    void _setSharedLibraryLoader(ISlangSharedLibraryLoader* loader);

    /// Will try to load the library by specified name (using the set loader), if not one already
    /// available.
    IDownstreamCompiler* getOrLoadDownstreamCompiler(PassThroughMode type, DiagnosticSink* sink);
    /// Will unload the specified shared library if it's currently loaded
    void resetDownstreamCompiler(PassThroughMode type);

    /// Get the prelude associated with the language
    const String& getPreludeForLanguage(SourceLanguage language)
    {
        return m_languagePreludes[int(language)];
    }

    /// Get the built in linkage -> handy to get the core module from
    Linkage* getBuiltinLinkage() const { return m_builtinLinkage; }

    Module* getBuiltinModule(slang::BuiltinModuleName builtinModuleName);

    Name* getCompletionRequestTokenName() const { return m_completionTokenName; }

    void init();

    void addBuiltinSource(
        Scope* scope,
        String const& path,
        ISlangBlob* sourceBlob,
        Module*& outModule);
    ~Session();

    void addDownstreamCompileTime(double time) { m_downstreamCompileTime += time; }
    void addTotalCompileTime(double time) { m_totalCompileTime += time; }

    ComPtr<ISlangSharedLibraryLoader>
        m_sharedLibraryLoader; ///< The shared library loader (never null)

    int m_downstreamCompilerInitialized = 0;

    RefPtr<DownstreamCompilerSet>
        m_downstreamCompilerSet; ///< Information about all available downstream compilers.
    ComPtr<IDownstreamCompiler> m_downstreamCompilers[int(
        PassThroughMode::CountOf)]; ///< A downstream compiler for a pass through
    DownstreamCompilerLocatorFunc m_downstreamCompilerLocators[int(PassThroughMode::CountOf)];
    Name* m_completionTokenName = nullptr; ///< The name of a completion request token.

    /// For parsing command line options
    CommandOptions m_commandOptions;

    int m_typeDictionarySize = 0;

    RefPtr<RefObject> m_typeCheckingCache;
    TypeCheckingCache* getTypeCheckingCache();
    std::mutex m_typeCheckingCacheMutex;

private:
    struct BuiltinModuleInfo
    {
        const char* name;
        Scope* languageScope;
    };

    BuiltinModuleInfo getBuiltinModuleInfo(slang::BuiltinModuleName name);

    void _initCodeGenTransitionMap();

    SlangResult _readBuiltinModule(
        ISlangFileSystem* fileSystem,
        Scope* scope,
        String moduleName,
        Module*& outModule);

    SlangResult _loadRequest(EndToEndCompileRequest* request, const void* data, size_t size);

    /// Linkage used for all built-in (core module) code.
    RefPtr<Linkage> m_builtinLinkage;

    String
        m_downstreamCompilerPaths[int(PassThroughMode::CountOf)]; ///< Paths for each pass through
    String m_languagePreludes[int(SourceLanguage::CountOf)]; ///< Prelude for each source language
    PassThroughMode m_defaultDownstreamCompilers[int(SourceLanguage::CountOf)];

    // Describes a conversion from one code gen target (source) to another (target)
    CodeGenTransitionMap m_codeGenTransitionMap;

    double m_downstreamCompileTime = 0.0;
    double m_totalCompileTime = 0.0;

    /// The AST builder that will be used for builtin modules.
    ///
    RefPtr<ASTBuilder> m_rootASTBuilder;
};

/* Returns SLANG_OK if pass through support is available */
SlangResult checkExternalCompilerSupport(Session* session, PassThroughMode passThrough);

const char* getBuiltinModuleNameStr(slang::BuiltinModuleName name);

} // namespace Slang
