#ifndef SLANG_COMPILER_H_INCLUDED
#define SLANG_COMPILER_H_INCLUDED

#include "../core/basic.h"
#include "../core/slang-shared-library.h"

#include "../../slang-com-ptr.h"

#include "diagnostics.h"
#include "name.h"
#include "profile.h"
#include "syntax.h"

#include "../../slang.h"

namespace Slang
{
    struct PathInfo;
    struct IncludeHandler;
    class ProgramLayout;
    class PtrType;
    class TargetProgram;
    class TargetRequest;
    class TypeLayout;

    enum class CompilerMode
    {
        ProduceLibrary,
        ProduceShader,
        GenerateChoice
    };

    enum class StageTarget
    {
        Unknown,
        VertexShader,
        HullShader,
        DomainShader,
        GeometryShader,
        FragmentShader,
        ComputeShader,
    };

    enum class CodeGenTarget
    {
        Unknown             = SLANG_TARGET_UNKNOWN,
        None                = SLANG_TARGET_NONE,
        GLSL                = SLANG_GLSL,
        GLSL_Vulkan         = SLANG_GLSL_VULKAN,
        GLSL_Vulkan_OneDesc = SLANG_GLSL_VULKAN_ONE_DESC,
        HLSL                = SLANG_HLSL,
        SPIRV               = SLANG_SPIRV,
        SPIRVAssembly       = SLANG_SPIRV_ASM,
        DXBytecode          = SLANG_DXBC,
        DXBytecodeAssembly  = SLANG_DXBC_ASM,
        DXIL                = SLANG_DXIL,
        DXILAssembly        = SLANG_DXIL_ASM,
    };

    enum class ContainerFormat
    {
        None            = SLANG_CONTAINER_FORMAT_NONE,
        SlangModule     = SLANG_CONTAINER_FORMAT_SLANG_MODULE,
    };

    enum class LineDirectiveMode : SlangLineDirectiveMode
    {
        Default     = SLANG_LINE_DIRECTIVE_MODE_DEFAULT,
        None        = SLANG_LINE_DIRECTIVE_MODE_NONE,
        Standard    = SLANG_LINE_DIRECTIVE_MODE_STANDARD,
        GLSL        = SLANG_LINE_DIRECTIVE_MODE_GLSL,
    };

    enum class ResultFormat
    {
        None,
        Text,
        Binary
    };

    // When storing the layout for a matrix-type
    // value, we need to know whether it has been
    // laid out with row-major or column-major
    // storage.
    //
    enum MatrixLayoutMode
    {
        kMatrixLayoutMode_RowMajor      = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
        kMatrixLayoutMode_ColumnMajor   = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
    };

    enum class DebugInfoLevel : SlangDebugInfoLevel
    {
        None        = SLANG_DEBUG_INFO_LEVEL_NONE,
        Minimal     = SLANG_DEBUG_INFO_LEVEL_MINIMAL,
        Standard    = SLANG_DEBUG_INFO_LEVEL_STANDARD,
        Maximal     = SLANG_DEBUG_INFO_LEVEL_MAXIMAL,
    };

    enum class OptimizationLevel : SlangOptimizationLevel
    {
        None    = SLANG_OPTIMIZATION_LEVEL_NONE,
        Default = SLANG_OPTIMIZATION_LEVEL_DEFAULT,
        High    = SLANG_OPTIMIZATION_LEVEL_HIGH,
        Maximal = SLANG_OPTIMIZATION_LEVEL_MAXIMAL,
    };

    class Linkage;
    class Module;
    class Program;
    class FrontEndCompileRequest;
    class BackEndCompileRequest;
    class EndToEndCompileRequest;
    class TranslationUnitRequest;

    // Result of compiling an entry point.
    // Should only ever be string OR binary.
    class CompileResult
    {
    public:
        CompileResult() = default;
        CompileResult(String const& str) : format(ResultFormat::Text), outputString(str) {}
        CompileResult(List<uint8_t> const& buffer) : format(ResultFormat::Binary), outputBinary(buffer) {}

        void append(CompileResult const& result);

        ComPtr<ISlangBlob> getBlob();

        ResultFormat format = ResultFormat::None;
        String outputString;
        List<uint8_t> outputBinary;

        ComPtr<ISlangBlob> blob;
    };

        /// Collects information about existential type parameters and their arguments.
    struct ExistentialTypeSlots
    {
            /// For each type parameter, holds the interface/existential type that constrains it.
        List<RefPtr<Type>> paramTypes;

            /// An argument for an existential type parameter.
            ///
            /// Comprises a concrete type and a witness for its conformance to the desired
            /// interface/existential type for the corresponding parameter.
            ///
        struct Arg
        {
            RefPtr<Type>    type;
            RefPtr<Val>     witness;
        };

            /// Any arguments provided for the existential type parameters.
            ///
            /// It is possible for `args` to be empty even if `paramTypes` is non-empty;
            /// that situation represents an unspecialized program or entry point.
            ///
        List<Arg> args;
    };

        /// Information collected about global or entry-point shader parameters
    struct ShaderParamInfo
    {
        DeclRef<VarDeclBase>    paramDeclRef;
        UInt                    firstExistentialTypeSlot = 0;
        UInt                    existentialTypeSlotCount = 0;
    };

        /// Extended information specific to global shader parameters
    struct GlobalShaderParamInfo : ShaderParamInfo
    {
        // Additional global-scope declarations that are conceptually
        // declaring the "same" parameter as the `paramDeclRef`.
        List<DeclRef<VarDeclBase>> additionalParamDeclRefs;
    };

        /// A request for the front-end to find and validate an entry-point function
    struct FrontEndEntryPointRequest : RefObject
    {
    public:
            /// Create a request for an entry point.
        FrontEndEntryPointRequest(
            FrontEndCompileRequest* compileRequest,
            int                     translationUnitIndex,
            Name*                   name,
            Profile                 profile);

            /// Get the parent front-end compile request.
        FrontEndCompileRequest* getCompileRequest() { return m_compileRequest; }

            /// Get the translation unit that contains the entry point.
        TranslationUnitRequest* getTranslationUnit();

            /// Get the name of the entry point to find.
        Name* getName() { return m_name; }

            /// Get the stage that the entry point is to be compiled for
        Stage getStage() { return m_profile.GetStage(); }

            /// Get the profile that the entry point is to be compiled for
        Profile getProfile() { return m_profile; }

    private:
        // The parent compile request
        FrontEndCompileRequest* m_compileRequest;

        // The index of the translation unit that will hold the entry point
        int                     m_translationUnitIndex;

        // The name of the entry point function to look for
        Name*                   m_name;

        // The profile to compile for (including stage)
        Profile                 m_profile;
    };

        /// Tracks an ordered list of modules that something depends on.
    struct ModuleDependencyList
    {
    public:
            /// Get the list of modules that are depended on.
        List<RefPtr<Module>> const& getModuleList() { return m_moduleList; }

            /// Add a module and everything it depends on to the list.
        void addDependency(Module* module);

            /// Add a module to the list, but not the modules it depends on.
        void addLeafDependency(Module* module);

    private:
        void _addDependency(Module* module);

        List<RefPtr<Module>>    m_moduleList;
        HashSet<Module*>        m_moduleSet;
    };

        /// Tracks an unordered list of filesystem paths that something depends on
    struct FilePathDependencyList
    {
    public:
            /// Get the list of paths that are depended on.
        List<String> const& getFilePathList() { return m_filePathList; }

            /// Add a path to the list, if it is not already present
        void addDependency(String const& path);

            /// Add all of the paths that `module` depends on to the list
        void addDependency(Module* module);

    private:

        // TODO: We are using a `HashSet` here to deduplicate
        // the paths so that we don't return the same path
        // multiple times from `getFilePathList`, but because
        // order isn't important, we could potentially do better
        // in terms of memory (at some cost in performance) by
        // just sorting the `m_filePathList` every once in
        // a while and then deduplicating.

        List<String>    m_filePathList;
        HashSet<String> m_filePathSet;
    };

        /// Describes an entry point for the purposes of layout and code generation.
        ///
        /// This class also tracks any generic arguments to the entry point,
        /// in the case that it is a specialization of a generic entry point.
        ///
        /// There is also a provision for creating a "dummy" entry point for
        /// the purposes of pass-through compilation modes. Only the
        /// `getName()` and `getProfile()` methods should be expected to
        /// return useful data on pass-through entry points.
        ///
    class EntryPoint : public RefObject
    {
    public:
            /// Create an entry point that refers to the given function.
        static RefPtr<EntryPoint> create(
            DeclRef<FuncDecl>   funcDeclRef,
            Profile             profile);

            /// Get the function decl-ref, including any generic arguments.
        DeclRef<FuncDecl> getFuncDeclRef() { return m_funcDeclRef; }

            /// Get the function declaration (without generic arguments).
        RefPtr<FuncDecl> getFuncDecl() { return m_funcDeclRef.getDecl(); }

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
        Stage getStage() { return m_profile.GetStage(); }

            /// Get the module that contains the entry point.
        Module* getModule();

            /// Get the linkage that contains the module for this entry point.
        Linkage* getLinkage();

            /// Get a list of modules that this entry point depends on.
            ///
            /// This will include the module that defines the entry point (see `getModule()`),
            /// but may also include modules that are required by its generic type arguments.
            ///
        List<RefPtr<Module>> getModuleDependencies() { return m_dependencyList.getModuleList(); }

            /// Get a list of tagged-union types referenced by the entry point's generic parameters.
        List<RefPtr<TaggedUnionType>> const& getTaggedUnionTypes() { return m_taggedUnionTypes; }

            /// Create a dummy `EntryPoint` that is only usable for pass-through compilation.
        static RefPtr<EntryPoint> createDummyForPassThrough(
            Name*       name,
            Profile     profile);

            /// Get the number of existential type parameters for the entry point.
        UInt getExistentialTypeParamCount() { return m_existentialSlots.paramTypes.Count(); }

            /// Get the existential type parameter at `index`.
        Type* getExistentialTypeParam(UInt index) { return m_existentialSlots.paramTypes[index]; }

            /// Get the number of arguments supplied for existential type parameters.
            ///
            /// Note that the number of arguments may not match the number of parameters.
            /// In particular, an unspecialized entry point may have many parameters, but zero arguments.
        UInt getExistentialTypeArgCount() { return m_existentialSlots.args.Count(); }

            /// Get the existential type argument (type and witness table) at `index`.
        ExistentialTypeSlots::Arg getExistentialTypeArg(UInt index) { return m_existentialSlots.args[index]; }

            /// Get an array of all existential type arguments.
        ExistentialTypeSlots::Arg const* getExistentialTypeArgs() { return m_existentialSlots.args.Buffer(); }

            /// Get an array of all entry-point shader parameters.
        List<ShaderParamInfo> const& getShaderParams() { return m_shaderParams; }

        void _specializeExistentialTypeParams(
            List<RefPtr<Expr>> const&   args,
            DiagnosticSink*             sink);

    private:
        EntryPoint(
            Name*               name,
            Profile             profile,
            DeclRef<FuncDecl>   funcDeclRef);

        void _collectShaderParams();

        // The name of the entry point function (e.g., `main`)
        //
        Name* m_name = nullptr;

        // The declaration of the entry-point function itself.
        //
        DeclRef<FuncDecl> m_funcDeclRef;

            /// The existential/interface slots associated with the entry point parameter scope.
        ExistentialTypeSlots m_existentialSlots;

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

        // Any tagged union types that were referenced by the generic arguments of the entry point.
        List<RefPtr<TaggedUnionType>> m_taggedUnionTypes;

        // Modules the entry point depends on.
        ModuleDependencyList m_dependencyList;
    };

    enum class PassThroughMode : SlangPassThrough
    {
        None = SLANG_PASS_THROUGH_NONE,	// don't pass through: use Slang compiler
        fxc = SLANG_PASS_THROUGH_FXC,	// pass through HLSL to `D3DCompile` API
        dxc = SLANG_PASS_THROUGH_DXC,	// pass through HLSL to `IDxcCompiler` API
        glslang = SLANG_PASS_THROUGH_GLSLANG,	// pass through GLSL to `glslang` library
    };

    class SourceFile;

        /// A module of code that has been compiled through the front-end
        ///
        /// A module comprises all the code from one translation unit (which
        /// may span multiple Slang source files), and provides access
        /// to both the AST and IR representations of that code.
        ///
    class Module : public RefObject
    {
    public:
            /// Create a module (initially empty).
        Module(Linkage* linkage);

            /// Get the parent linkage of this module.
        Linkage* getLinkage() { return m_linkage; }

            /// Get the AST for the module (if it has been parsed)
        ModuleDecl* getModuleDecl() { return m_moduleDecl; }

            /// The the IR for the module (if it has been generated)
        IRModule* getIRModule() { return m_irModule; }

            /// Get the list of other modules this module depends on
        List<RefPtr<Module>> const& getModuleDependencyList() { return m_moduleDependencyList.getModuleList(); }

            /// Get the list of filesystem paths this module depends on
        List<String> const& getFilePathDependencyList() { return m_filePathDependencyList.getFilePathList(); }

            /// Register a module that this module depends on
        void addModuleDependency(Module* module);

            /// Register a filesystem path that this module depends on
        void addFilePathDependency(String const& path);

            /// Set the AST for this module.
            ///
            /// This should only be called once, during creation of the module.
            ///
        void setModuleDecl(ModuleDecl* moduleDecl) { m_moduleDecl = moduleDecl; }

            /// Set the IR for this module.
            ///
            /// This should only be called once, during creation of the module.
            ///
        void setIRModule(IRModule* irModule) { m_irModule = irModule; }

    private:
        // The parent linkage
        Linkage* m_linkage = nullptr;

        // The AST for the module
        RefPtr<ModuleDecl>  m_moduleDecl;

        // The IR for the module
        RefPtr<IRModule> m_irModule = nullptr;

        // List of modules this module depends on
        ModuleDependencyList m_moduleDependencyList;

        // List of filesystem paths this module depends on
        FilePathDependencyList m_filePathDependencyList;
    };
    typedef Module LoadedModule;

        /// A request for the front-end to compile a translation unit.
    class TranslationUnitRequest : public RefObject
    {
    public:
        TranslationUnitRequest(
            FrontEndCompileRequest* compileRequest);

        // The parent compile request
        FrontEndCompileRequest* compileRequest = nullptr;

        // The language in which the source file(s)
        // are assumed to be written
        SourceLanguage sourceLanguage = SourceLanguage::Unknown;

        // The source file(s) that will be compiled to form this translation unit
        //
        // Usually, for HLSL or GLSL there will be only one file.
        List<SourceFile*> m_sourceFiles;

        List<SourceFile*> const& getSourceFiles() { return m_sourceFiles; }
        void addSourceFile(SourceFile* sourceFile);

        // The entry points associated with this translation unit
        List<RefPtr<EntryPoint>> entryPoints;

        // Preprocessor definitions to use for this translation unit only
        // (whereas the ones on `compileRequest` will be shared)
        Dictionary<String, String> preprocessorDefinitions;

            /// The name that will be used for the module this translation unit produces.
        Name* moduleName = nullptr;

            /// Result of compiling this translation unit (a module)
        RefPtr<Module> module;

        Module* getModule() { return module; }
        RefPtr<ModuleDecl> getModuleDecl() { return module->getModuleDecl(); }

        Session* getSession();
        NamePool* getNamePool();
        SourceManager* getSourceManager();
    };

    enum class FloatingPointMode : SlangFloatingPointMode
    {
        Default = SLANG_FLOATING_POINT_MODE_DEFAULT,
        Fast = SLANG_FLOATING_POINT_MODE_FAST,
        Precise = SLANG_FLOATING_POINT_MODE_PRECISE,
    };

    enum class WriterChannel : SlangWriterChannel
    {
        Diagnostic = SLANG_WRITER_CHANNEL_DIAGNOSTIC,
        StdOutput = SLANG_WRITER_CHANNEL_STD_OUTPUT,
        StdError = SLANG_WRITER_CHANNEL_STD_ERROR,
        CountOf = SLANG_WRITER_CHANNEL_COUNT_OF,
    };

    enum class WriterMode : SlangWriterMode
    {
        Text = SLANG_WRITER_MODE_TEXT,
        Binary = SLANG_WRITER_MODE_BINARY,
    };

        /// A request to generate output in some target format.
    class TargetRequest : public RefObject
    {
    public:
        Linkage*            linkage;
        CodeGenTarget       target;
        SlangTargetFlags    targetFlags = 0;
        Slang::Profile      targetProfile = Slang::Profile();
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;

        Linkage* getLinkage() { return linkage; }
        CodeGenTarget getTarget() { return target; }
        Profile getTargetProfile() { return targetProfile; }
        FloatingPointMode getFloatingPointMode() { return floatingPointMode; }

        Session* getSession();
        MatrixLayoutMode getDefaultMatrixLayoutMode();

        // TypeLayouts created on the fly by reflection API
        Dictionary<Type*, RefPtr<TypeLayout>> typeLayouts;

        Dictionary<Type*, RefPtr<TypeLayout>>& getTypeLayouts() { return typeLayouts; }
    };

        /// Are we generating code for a D3D API?
    bool isD3DTarget(TargetRequest* targetReq);

        /// Are we generating code for a Khronos API (OpenGL or Vulkan)?
    bool isKhronosTarget(TargetRequest* targetReq);

    // Compute the "effective" profile to use when outputting the given entry point
    // for the chosen code-generation target.
    //
    // The stage of the effective profile will always come from the entry point, while
    // the profile version (aka "shader model") will be computed as follows:
    //
    // - If the entry point and target belong to the same profile family, then take
    //   the latest version between the two (e.g., if the entry point specified `ps_5_1`
    //   and the target specifies `sm_5_0` then use `sm_5_1` as the version).
    //
    // - If the entry point and target disagree on the profile family, always use the
    //   profile family and version from the target.
    //
    Profile getEffectiveProfile(EntryPoint* entryPoint, TargetRequest* target);


    // A directory to be searched when looking for files (e.g., `#include`)
    struct SearchDirectory
    {
        SearchDirectory() = default;
        SearchDirectory(SearchDirectory const& other) = default;
        SearchDirectory(String const& path)
            : path(path)
        {}

        String  path;
    };

        /// A list of directories to search for files (e.g., `#include`)
    struct SearchDirectoryList
    {
        // A parent list that should also be searched
        SearchDirectoryList*    parent = nullptr;

        // Directories to be searched
        List<SearchDirectory>   searchDirectories;
    };

    /// Create a blob that will retain (a copy of) raw data.
    ///
    ComPtr<ISlangBlob> createRawBlob(void const* data, size_t size);

        /// A context for loading and re-using code modules.
    class Linkage : public RefObject
    {
    public:
            /// Create an initially-empty linkage
        Linkage(Session* session);

            /// Get the parent session for this linkage
        Session* getSession() { return m_session; }

        // Information on the targets we are being asked to
        // generate code for.
        List<RefPtr<TargetRequest>> targets;

        // Directories to search for `#include` files or `import`ed modules
        SearchDirectoryList searchDirectories;

        SearchDirectoryList const& getSearchDirectories() { return searchDirectories; }

        // Definitions to provide during preprocessing
        Dictionary<String, String> preprocessorDefinitions;

        // Source manager to help track files loaded
        SourceManager m_defaultSourceManager;
        SourceManager* m_sourceManager = nullptr;

        // Name pool for looking up names
        NamePool namePool;

        NamePool* getNamePool() { return &namePool; }

        // Modules that have been dynamically loaded via `import`
        //
        // This is a list of unique modules loaded, in the order they were encountered.
        List<RefPtr<LoadedModule> > loadedModulesList;

        // Map from the path (or uniqueIdentity if available) of a module file to its definition
        Dictionary<String, RefPtr<LoadedModule>> mapPathToLoadedModule;

        // Map from the logical name of a module to its definition
        Dictionary<Name*, RefPtr<LoadedModule>> mapNameToLoadedModules;

        // The resulting specialized IR module for each entry point request
        List<RefPtr<IRModule>> compiledModules;

        /// File system implementation to use when loading files from disk.
        ///
        /// If this member is `null`, a default implementation that tries
        /// to use the native OS filesystem will be used instead.
        ///
        ComPtr<ISlangFileSystem> fileSystem;

        /// The extended file system implementation. Will be set to a default implementation
        /// if fileSystem is nullptr. Otherwise it will either be fileSystem's interface, 
        /// or a wrapped impl that makes fileSystem operate as fileSystemExt
        ComPtr<ISlangFileSystemExt> fileSystemExt;

        ISlangFileSystemExt* getFileSystemExt() { return fileSystemExt; }

        /// Load a file into memory using the configured file system.
        ///
        /// @param path The path to attempt to load from
        /// @param outBlob A destination pointer to receive the loaded blob
        /// @returns A `SlangResult` to indicate success or failure.
        ///
        SlangResult loadFile(String const& path, ISlangBlob** outBlob);


        RefPtr<Expr> parseTypeString(String typeStr, RefPtr<Scope> scope);

            /// Add a mew target amd return its index.
        UInt addTarget(
            CodeGenTarget   target);

        RefPtr<Module> loadModule(
            Name*               name,
            const PathInfo&     filePathInfo,
            ISlangBlob*         fileContentsBlob,
            SourceLoc const&    loc,
            DiagnosticSink*     sink);

        void loadParsedModule(
            RefPtr<TranslationUnitRequest>  translationUnit,
            Name*                           name,
            PathInfo const&                 pathInfo);

            /// Load a module of the given name.
        Module* loadModule(String const& name);

        RefPtr<Module> findOrImportModule(
            Name*               name,
            SourceLoc const&    loc,
            DiagnosticSink*     sink);

        SourceManager* getSourceManager()
        {
            return m_sourceManager;
        }

            /// Override the source manager for the linakge.
            ///
            /// This is only used to install a temporary override when
            /// parsing stuff from strings (where we don't want to retain
            /// full source files for the parsed result).
            ///
            /// TODO: We should remove the need for this hack.
            ///
        void setSourceManager(SourceManager* sourceManager)
        {
            m_sourceManager = sourceManager;
        }

        void setFileSystem(ISlangFileSystem* fileSystem);

        /// The layout to use for matrices by default (row/column major)
        MatrixLayoutMode defaultMatrixLayoutMode = kMatrixLayoutMode_ColumnMajor;
        MatrixLayoutMode getDefaultMatrixLayoutMode() { return defaultMatrixLayoutMode; }

        DebugInfoLevel debugInfoLevel = DebugInfoLevel::None;

        OptimizationLevel optimizationLevel = OptimizationLevel::Default;

    private:
        Session* m_session = nullptr;

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
                Linkage*    linkage,
                Module*     module)
                : linkage(linkage)
                , module(module)
            {
                next = linkage->m_modulesBeingImported;
                linkage->m_modulesBeingImported = this;
            }

            ~ModuleBeingImportedRAII()
            {
                linkage->m_modulesBeingImported = next;
            }

            Linkage* linkage;
            Module* module;
            ModuleBeingImportedRAII* next;
        };

        // Any modules currently being imported will be listed here
        ModuleBeingImportedRAII* m_modulesBeingImported = nullptr;

            /// Is the given module in the middle of being imported?
        bool isBeingImported(Module* module);
    };

        /// Shared functionality between front- and back-end compile requests.
        ///
        /// This is the base class for both `FrontEndCompileRequest` and
        /// `BackEndCompileRequest`, and allows a small number of parts of
        /// the compiler to be easily invocable from either front-end or
        /// back-end work.
        ///
    class CompileRequestBase : public RefObject
    {
        // TODO: We really shouldn't need this type in the long run.
        // The few places that rely on it should be refactored to just
        // depend on the underlying information (a linkage and a diagnostic
        // sink) directly.
        //
        // The flags to control dumping and validation of IR should be
        // moved to some kind of shared settings/options `struct` that
        // both front-end and back-end requests can store.

    public:
        Session* getSession();
        Linkage* getLinkage() { return m_linkage; }
        DiagnosticSink* getSink() { return m_sink; }
        SourceManager* getSourceManager() { return getLinkage()->getSourceManager(); }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }
        ISlangFileSystemExt* getFileSystemExt() { return getLinkage()->getFileSystemExt(); }
        SlangResult loadFile(String const& path, ISlangBlob** outBlob) { return getLinkage()->loadFile(path, outBlob); }

        bool shouldDumpIR = false;
        bool shouldValidateIR = false;

    protected:
        CompileRequestBase(
            Linkage*        linkage,
            DiagnosticSink* sink);

    private:
        Linkage* m_linkage = nullptr;
        DiagnosticSink* m_sink = nullptr;
    };

        /// A request to compile source code to an AST + IR.
    class FrontEndCompileRequest : public CompileRequestBase
    {
    public:
        FrontEndCompileRequest(
            Linkage*        linkage,
            DiagnosticSink* sink);

        int addEntryPoint(
            int                     translationUnitIndex,
            String const&           name,
            Profile                 entryPointProfile);

        // Translation units we are being asked to compile
        List<RefPtr<TranslationUnitRequest> > translationUnits;

        RefPtr<TranslationUnitRequest> getTranslationUnit(UInt index) { return translationUnits[index]; }

        // Compile flags to be shared by all translation units
        SlangCompileFlags compileFlags = 0;

        // If true then generateIR will serialize out IR, and serialize back in again. Making 
        // serialization a bottleneck or firewall between the front end and the backend
        bool useSerialIRBottleneck = false; 

        // If true will serialize and de-serialize with debug information
        bool verifyDebugSerialization = false;

        List<RefPtr<FrontEndEntryPointRequest>> m_entryPointReqs;

        List<RefPtr<FrontEndEntryPointRequest>> const& getEntryPointReqs() { return m_entryPointReqs; }
        UInt getEntryPointReqCount() { return m_entryPointReqs.Count(); }
        FrontEndEntryPointRequest* getEntryPointReq(UInt index) { return m_entryPointReqs[index]; }

        // Directories to search for `#include` files or `import`ed modules
        // NOTE! That for now these search directories are not settable via the API
        // so the search directories on Linkage is used for #include as well as for modules.
        SearchDirectoryList searchDirectories;

        SearchDirectoryList const& getSearchDirectories() { return searchDirectories; }

        // Definitions to provide during preprocessing
        Dictionary<String, String> preprocessorDefinitions;

        void parseTranslationUnit(
            TranslationUnitRequest* translationUnit);

        // Perform primary semantic checking on all
        // of the translation units in the program
        void checkAllTranslationUnits();

        void generateIR();

        SlangResult executeActionsInner();

            /// Add a translation unit to be compiled.
            ///
            /// @param language The source language that the translation unit will use (e.g., `SourceLanguage::Slang`
            /// @param moduleName The name that will be used for the module compile from the translation unit.
            /// @return The zero-based index of the translation unit in this compile request.
        int addTranslationUnit(SourceLanguage language, Name* moduleName);

            /// Add a translation unit to be compiled.
            ///
            /// @param language The source language that the translation unit will use (e.g., `SourceLanguage::Slang`
            /// @return The zero-based index of the translation unit in this compile request.
            ///
            /// The module name for the translation unit will be automatically generated.
            /// If all translation units in a compile request use automatically generated
            /// module names, then they are guaranteed not to conflict with one another.
            ///
        int addTranslationUnit(SourceLanguage language);

        void addTranslationUnitSourceFile(
            int             translationUnitIndex,
            SourceFile*     sourceFile);

        void addTranslationUnitSourceBlob(
            int             translationUnitIndex,
            String const&   path,
            ISlangBlob*     sourceBlob);

        void addTranslationUnitSourceString(
            int             translationUnitIndex,
            String const&   path,
            String const&   source);

        void addTranslationUnitSourceFile(
            int             translationUnitIndex,
            String const&   path);

        Program* getProgram() { return m_program; }

    private:
        RefPtr<Program> m_program;
    };

        /// A collection of code modules and entry points that are intended to be used together.
        ///
        /// A `Program` establishes that certain pieces of code are intended
        /// to be used togehter so that, e.g., layout can make sure to allocate
        /// space for the global shader parameters in all referenced modules.
        ///
    class Program : public RefObject
    {
    public:
            /// Create a new program, initially empty.
            ///
            /// All code loaded into the program must come
            /// from the given `linkage`.
        Program(
            Linkage* linkage);

            /// Get the linkage that this program uses.
        Linkage* getLinkage() { return m_linkage; }

            /// Get the number of entry points added to the program
        UInt getEntryPointCount() { return m_entryPoints.Count(); }

            /// Get the entry point at the given `index`.
        RefPtr<EntryPoint> getEntryPoint(UInt index) { return m_entryPoints[index]; }

            /// Get the full ist of entry points on the program.
        List<RefPtr<EntryPoint>> const& getEntryPoints() { return m_entryPoints; }

            /// Get the substitution (if any) that represents how global generics are specialized.
        RefPtr<Substitutions> getGlobalGenericSubstitution() { return m_globalGenericSubst; }

            /// Get the full list of modules this program depends on
        List<RefPtr<Module>> getModuleDependencies() { return m_moduleDependencyList.getModuleList(); }

            /// Get the full list of filesystem paths this program depends on
        List<String> getFilePathDependencies() { return m_filePathDependencyList.getFilePathList(); }

            /// Get the target-specific version of this program for the given `target`.
            ///
            /// The `target` must be a target on the `Linkage` that was used to create this program.
        TargetProgram* getTargetProgram(TargetRequest* target);
            
            /// Add a module (and everything it depends on) to the list of references
        void addReferencedModule(Module* module);

            /// Add a module (but not the things it depends on) to the list of references
            ///
            /// This is a compatiblity hack for legacy compiler behavior.
        void addReferencedLeafModule(Module* module);


            /// Add an entry point to the program
            ///
            /// This also adds everything the entry point depends on to the list of references.
            ///
        void addEntryPoint(EntryPoint* entryPoint);

            /// Set the global generic argument substitution to use.
        void setGlobalGenericSubsitution(RefPtr<Substitutions> subst)
        {
            m_globalGenericSubst = subst;
        }

            /// Parse a type from a string, in the context of this program.
            ///
            /// Any names in the string will be resolved using the modules
            /// referenced by the program.
            ///
            /// On an error, returns null and reports diagnostic messages
            /// to the provided `sink`.
            ///
        Type* getTypeFromString(String typeStr, DiagnosticSink* sink);

            /// Get the IR module that represents this program and its entry points.
            ///
            /// The IR module for a program tries to be minimal, and in the
            /// common case will only include symbols with `[import]` declarations
            /// for the entry point(s) of the program, and any types they
            /// depend on.
            ///
            /// This IR module is intended to be linked against the IR modules
            /// for all of the dependencies (see `getModuleDependencies()`) to
            /// provide complete code.
            ///
        RefPtr<IRModule> getOrCreateIRModule(DiagnosticSink* sink);

            /// Get the number of existential type parameters for the program.
        UInt getExistentialTypeParamCount() { return m_globalExistentialSlots.paramTypes.Count(); }

            /// Get the existential type parameter at `index`.
        Type* getExistentialTypeParam(UInt index) { return m_globalExistentialSlots.paramTypes[index]; }

            /// Get the number of arguments supplied for existential type parameters.
            ///
            /// Note that the number of arguments may not match the number of parameters.
            /// In particular, an unspecialized program may have many parameters, but zero arguments.
        UInt getExistentialTypeArgCount() { return m_globalExistentialSlots.args.Count(); }

            /// Get the existential type argument (type and witness table) at `index`.
        ExistentialTypeSlots::Arg getExistentialTypeArg(UInt index) { return m_globalExistentialSlots.args[index]; }

            /// Get an array of all existential type arguments.
        ExistentialTypeSlots::Arg const* getExistentialTypeArgs() { return m_globalExistentialSlots.args.Buffer(); }

            /// Get an array of all global shader parameters.
        List<GlobalShaderParamInfo> const& getShaderParams() { return m_shaderParams; }

        void _collectShaderParams(DiagnosticSink* sink);
        void _specializeExistentialTypeParams(
            List<RefPtr<Expr>> const&   args,
            DiagnosticSink*             sink);

    private:

        // The linakge this program is associated with.
        //
        // Note that a `Program` keeps its associated linkage alive,
        // and not vice versa.
        //
        RefPtr<Linkage> m_linkage;

        // Tracking data for the list of modules dependend on
        ModuleDependencyList m_moduleDependencyList;

        // Tracking data for the list of filesystem paths dependend on
        FilePathDependencyList m_filePathDependencyList;

        // Entry points that are part of the program.
        List<RefPtr<EntryPoint> > m_entryPoints;

        // Specializations for global generic parameters (if any)
        RefPtr<Substitutions> m_globalGenericSubst;

        // The existential/interface slots associated with the global scope.
        ExistentialTypeSlots m_globalExistentialSlots;

            /// Information about global shader parameters
        List<GlobalShaderParamInfo> m_shaderParams;

        // Generated IR for this program.
        RefPtr<IRModule> m_irModule;

        // Cache of target-specific programs for each target.
        Dictionary<TargetRequest*, RefPtr<TargetProgram>> m_targetPrograms;

        // Any types looked up dynamically using `getTypeFromString`
        Dictionary<String, RefPtr<Type>> m_types;
    };

        /// A `Program` specialized for a particular `TargetRequest`
    class TargetProgram : public RefObject
    {
    public:
        TargetProgram(
            Program*        program,
            TargetRequest*  targetReq);

            /// Get the underlying program
        Program* getProgram() { return m_program; }

            /// Get the underlying target
        TargetRequest* getTargetReq() { return m_targetReq; }

            /// Get the layout for the program on the target.
            ///
            /// If this is the first time the layout has been
            /// requested, report any errors that arise during
            /// layout to the given `sink`.
            ///
        ProgramLayout* getOrCreateLayout(DiagnosticSink* sink);

            /// Get the layout for the program on the taarget.
            ///
            /// This routine assumes that `getOrCreateLayout`
            /// has already been called previously.
            ///
        ProgramLayout* getExistingLayout()
        {
            SLANG_ASSERT(m_layout);
            return m_layout;
        }

            /// Get the compiled code for an entry point on the target.
            ///
            /// This routine assumes code generation has already been
            /// performed and called `setEntryPointResult`.
            ///
        CompileResult& getExistingEntryPointResult(Int entryPointIndex)
        {
            return m_entryPointResults[entryPointIndex];
        }

        // TODO: Need a lazy `getOrCreateEntryPointResult`

            /// Set the compiled code for an entry point.
            ///
            /// Should only be called by code generation.
        void setEntryPointResult(Int entryPointIndex, CompileResult const& result)
        {
            m_entryPointResults[entryPointIndex] = result;
        }

    private:
        // The program being compiled or laid out
        Program* m_program;

        // The target that code/layout will be generated for
        TargetRequest* m_targetReq;

        // The computed layout, if it has been generated yet
        RefPtr<ProgramLayout> m_layout;

        // Generated compile results for each entry point
        // in the parent `Program` (indexing matches
        // the order they are given in the `Program`)
        List<CompileResult> m_entryPointResults;
    };

        /// A request to generate code for a program
    class BackEndCompileRequest : public CompileRequestBase
    {
    public:
        BackEndCompileRequest(
            Linkage*        linkage,
            DiagnosticSink* sink,
            Program*        program = nullptr);

        // Should we dump intermediate results along the way, for debugging?
        bool shouldDumpIntermediates = false;

        // How should `#line` directives be emitted (if at all)?
        LineDirectiveMode lineDirectiveMode = LineDirectiveMode::Default;

        LineDirectiveMode getLineDirectiveMode() { return lineDirectiveMode; }

        Program* getProgram() { return m_program; }
        void setProgram(Program* program) { m_program = program; }

    private:
        RefPtr<Program> m_program;
    };

        /// A compile request that spans the front and back ends of the compiler
        ///
        /// This is what the command-line `slangc` uses, as well as the legacy
        /// C API. It ties together the functionality of `Linkage`,
        /// `FrontEndCompileRequest`, and `BackEndCompileRequest`, plus a small
        /// number of additional features that primarily make sense for
        /// command-line usage.
        ///
    class EndToEndCompileRequest : public RefObject
    {
    public:
        EndToEndCompileRequest(
            Session* session);

        // What container format are we being asked to generate?
        //
        // Note: This field is unused except by the options-parsing
        // logic; it exists to support wriiting out binary modules
        // once that feature is ready.
        //
        ContainerFormat containerFormat = ContainerFormat::None;

        // Path to output container to
        //
        // Note: This field exists to support wriiting out binary modules
        // once that feature is ready.
        //
        String containerOutputPath;

        // Should we just pass the input to another compiler?
        PassThroughMode passThrough = PassThroughMode::None;

            /// Source code for the generic arguments to use for the global generic parameters of the program.
        List<String> globalGenericArgStrings;

            /// Types to use to fill global existential "slots"
        List<String> globalExistentialSlotArgStrings;

        bool shouldSkipCodegen = false;

        // Are we being driven by the command-line `slangc`, and should act accordingly?
        bool isCommandLineCompile = false;

        String mDiagnosticOutput;

            /// A blob holding the diagnostic output
        ComPtr<ISlangBlob> diagnosticOutputBlob;

            /// Per-entry-point information not tracked by other compile requests
        class EntryPointInfo : public RefObject
        {
        public:
            /// Source code for the generic arguments to use for the generic parameters of the entry point.
            List<String> genericArgStrings;

            /// Source code for the type arguments to plug into the existential type "slots" of the entry point
            List<String> existentialArgStrings;
        };
        List<EntryPointInfo> entryPoints;

            /// Per-target information only needed for command-line compiles
        class TargetInfo : public RefObject
        {
        public:
            // Requested output paths for each entry point.
            // An empty string indices no output desired for
            // the given entry point.
            Dictionary<Int, String> entryPointOutputPaths;
        };
        Dictionary<TargetRequest*, RefPtr<TargetInfo>> targetInfos;

        Linkage* getLinkage() { return m_linkage; }

        int addEntryPoint(
            int                     translationUnitIndex,
            String const&           name,
            Profile                 profile,
            List<String> const &    genericTypeNames);

        void setWriter(WriterChannel chan, ISlangWriter* writer);
        ISlangWriter* getWriter(WriterChannel chan) const { return m_writers[int(chan)]; }

        SlangResult executeActionsInner();
        SlangResult executeActions();

        Session* getSession() { return m_session; }
        DiagnosticSink* getSink() { return &m_sink; }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }

        FrontEndCompileRequest* getFrontEndReq() { return m_frontEndReq; }
        BackEndCompileRequest* getBackEndReq() { return m_backEndReq; }
        Program* getUnspecializedProgram() { return getFrontEndReq()->getProgram(); }
        Program* getSpecializedProgram() { return m_specializedProgram; }

    private:
        Session*                        m_session = nullptr;
        RefPtr<Linkage>                 m_linkage;
        DiagnosticSink                  m_sink;
        RefPtr<FrontEndCompileRequest>  m_frontEndReq;
        RefPtr<Program>                 m_unspecializedProgram;
        RefPtr<Program>                 m_specializedProgram;
        RefPtr<BackEndCompileRequest>   m_backEndReq;

        // For output
        ComPtr<ISlangWriter> m_writers[SLANG_WRITER_CHANNEL_COUNT_OF];
    };

    void generateOutput(
        BackEndCompileRequest* compileRequest);

    void generateOutput(
        EndToEndCompileRequest* compileRequest);

    // Helper to dump intermediate output when debugging
    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        CodeGenTarget   target);
    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        char const*     text,
        CodeGenTarget   target);

    /* Returns SLANG_OK if a codeGen target is available. */
    SlangResult checkCompileTargetSupport(Session* session, CodeGenTarget target);
    /* Returns SLANG_OK if pass through support is available */
    SlangResult checkExternalCompilerSupport(Session* session, PassThroughMode passThrough);

    /* Report an error appearing from external compiler to the diagnostic sink error to the diagnostic sink.
    @param compilerName The name of the compiler the error came for (or nullptr if not known)
    @param res Result associated with the error. The error code will be reported. (Can take HRESULT - and will expand to string if known)
    @param diagnostic The diagnostic string associated with the compile failure
    @param sink The diagnostic sink to report to */
    void reportExternalCompileError(const char* compilerName, SlangResult res, const UnownedStringSlice& diagnostic, DiagnosticSink* sink);

    /* Determines a suitable filename to identify the input for a given entry point being compiled.
    If the end-to-end compile is a pass-through case, will attempt to find the (unique) source file
    pathname for the translation unit containing the entry point at `entryPointIndex.
    If the compilation is not in a pass-through case, then always returns `"slang-generated"`.
    @param endToEndReq The end-to-end compile request which might be using pass-through copmilation
    @param entryPointIndex The index of the entry point to compute a filename for.
    @return the appropriate source filename */
    String calcSourcePathForEntryPoint(EndToEndCompileRequest* endToEndReq, UInt entryPointIndex);

    struct TypeCheckingCache;
    //

    class Session
    {
    public:
        enum class SharedLibraryFuncType
        {
            Glslang_Compile,
            Fxc_D3DCompile,
            Fxc_D3DDisassemble,
            Dxc_DxcCreateInstance,
            CountOf,
        };

        //

        RefPtr<Scope>   baseLanguageScope;
        RefPtr<Scope>   coreLanguageScope;
        RefPtr<Scope>   hlslLanguageScope;
        RefPtr<Scope>   slangLanguageScope;

        List<RefPtr<ModuleDecl>> loadedModuleCode;

        SourceManager   builtinSourceManager;

        SourceManager* getBuiltinSourceManager() { return &builtinSourceManager; }

        // Name pool stuff for unique-ing identifiers

        RootNamePool rootNamePool;
        NamePool namePool;

        RootNamePool* getRootNamePool() { return &rootNamePool; }
        NamePool* getNamePool() { return &namePool; }
        Name* getNameObj(String name) { return namePool.getName(name); }
        Name* tryGetNameObj(String name) { return namePool.tryGetName(name); }
        //

        // Generated code for stdlib, etc.
        String stdlibPath;
        String coreLibraryCode;
        String slangLibraryCode;
        String hlslLibraryCode;
        String glslLibraryCode;

        String getStdlibPath();
        String getCoreLibraryCode();
        String getHLSLLibraryCode();

        // Basic types that we don't want to re-create all the time
        RefPtr<Type> errorType;
        RefPtr<Type> initializerListType;
        RefPtr<Type> overloadedType;
        RefPtr<Type> constExprRate;
        RefPtr<Type> irBasicBlockType;

        RefPtr<Type> stringType;
        RefPtr<Type> enumTypeType;

        ComPtr<ISlangSharedLibraryLoader> sharedLibraryLoader;                          ///< The shared library loader (never null)
        ComPtr<ISlangSharedLibrary> sharedLibraries[int(SharedLibraryType::CountOf)];   ///< The loaded shared libraries
        SlangFuncPtr sharedLibraryFunctions[int(SharedLibraryFuncType::CountOf)];

        Dictionary<int, RefPtr<Type>> builtinTypes;
        Dictionary<String, Decl*> magicDecls;

        void initializeTypes();

        Type* getBoolType();
        Type* getHalfType();
        Type* getFloatType();
        Type* getDoubleType();
        Type* getIntType();
        Type* getInt64Type();
        Type* getUIntType();
        Type* getUInt64Type();
        Type* getVoidType();
        Type* getBuiltinType(BaseType flavor);

        Type* getInitializerListType();
        Type* getOverloadedType();
        Type* getErrorType();
        Type* getStringType();

        Type* getEnumTypeType();

        // Construct the type `Ptr<valueType>`, where `Ptr`
        // is looked up as a builtin type.
        RefPtr<PtrType> getPtrType(RefPtr<Type> valueType);

        // Construct the type `Out<valueType>`
        RefPtr<OutType> getOutType(RefPtr<Type> valueType);

        // Construct the type `InOut<valueType>`
        RefPtr<InOutType> getInOutType(RefPtr<Type> valueType);

        // Construct the type `Ref<valueType>`
        RefPtr<RefType> getRefType(RefPtr<Type> valueType);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the actual type name for the pointer type is given by `ptrTypeName`
        RefPtr<PtrTypeBase> getPtrType(RefPtr<Type> valueType, char const* ptrTypeName);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the generic declaration for the pointer type is `genericDecl`
        RefPtr<PtrTypeBase> getPtrType(RefPtr<Type> valueType, GenericDecl* genericDecl);

        RefPtr<ArrayExpressionType> getArrayType(
            Type*   elementType,
            IntVal* elementCount);

        RefPtr<VectorExpressionType> getVectorType(
            RefPtr<Type>    elementType,
            RefPtr<IntVal>  elementCount);

        SyntaxClass<RefObject> findSyntaxClass(Name* name);

        Dictionary<Name*, SyntaxClass<RefObject> > mapNameToSyntaxClass;

        // cache used by type checking, implemented in check.cpp
        TypeCheckingCache* typeCheckingCache = nullptr;
        TypeCheckingCache* getTypeCheckingCache();
        void destroyTypeCheckingCache();
        //

            /// Will try to load the library by specified name (using the set loader), if not one already available.
        ISlangSharedLibrary* getOrLoadSharedLibrary(SharedLibraryType type, DiagnosticSink* sink);

            /// Gets a shared library by type, or null if not loaded
        ISlangSharedLibrary* getSharedLibrary(SharedLibraryType type) const { return sharedLibraries[int(type)]; }

        SlangFuncPtr getSharedLibraryFunc(SharedLibraryFuncType type, DiagnosticSink* sink);

        Session();

        void addBuiltinSource(
            RefPtr<Scope> const&    scope,
            String const&           path,
            String const&           source);
        ~Session();

    private:
            /// Linkage used for all built-in (stdlib) code.
        RefPtr<Linkage> m_builtinLinkage;
    };

}

#endif
