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
    class CompileRequest;
    class ProgramLayout;
    class PtrType;
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


    class CompileRequest;
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

    // Describes an entry point that we've been requested to compile
    class EntryPointRequest : public RefObject
    {
    public:
        // The parent compile request
        CompileRequest* compileRequest = nullptr;

        // The name of the entry point function (e.g., `main`)
        Name* name;

        // The type names we want to substitute into the
        // global generic type parameters
        List<String> genericParameterTypeNames;

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
        Profile profile;

        // Get the stage that the entry point is being compiled for.
        Stage getStage() { return profile.GetStage(); }

        // The index of the translation unit (within the parent
        // compile request) that the entry point function is
        // supposed to be defined in.
        int translationUnitIndex;

        // The translation unit that this entry point came from
        TranslationUnitRequest* getTranslationUnit();

        // The declaration of the entry-point function itself.
        // This will be filled in as part of semantic analysis;
        // it should not be assumed to be available in cases
        // where any errors were diagnosed.
        RefPtr<FuncDecl> decl;

        RefPtr<Substitutions> globalGenericSubst;
    };

    enum class PassThroughMode : SlangPassThrough
    {
        None = SLANG_PASS_THROUGH_NONE,	// don't pass through: use Slang compiler
        fxc = SLANG_PASS_THROUGH_FXC,	// pass through HLSL to `D3DCompile` API
        dxc = SLANG_PASS_THROUGH_DXC,	// pass through HLSL to `IDxcCompiler` API
        glslang = SLANG_PASS_THROUGH_GLSLANG,	// pass through GLSL to `glslang` library
    };

    class SourceFile;

    // A single translation unit requested to be compiled.
    //
    class TranslationUnitRequest : public RefObject
    {
    public:
        // The parent compile request
        CompileRequest* compileRequest = nullptr;

        // The language in which the source file(s)
        // are assumed to be written
        SourceLanguage sourceLanguage = SourceLanguage::Unknown;

        // The source file(s) that will be compiled to form this translation unit
        //
        // Usually, for HLSL or GLSL there will be only one file.
        List<RefPtr<SourceFile> > sourceFiles;

        // The entry points associated with this translation unit
        List<RefPtr<EntryPointRequest> > entryPoints;

        // Preprocessor definitions to use for this translation unit only
        // (whereas the ones on `CompileOptions` will be shared)
        Dictionary<String, String> preprocessorDefinitions;

        // Compile flags for this translation unit
        SlangCompileFlags compileFlags = 0;

        // The parsed syntax for the translation unit
        RefPtr<ModuleDecl>   SyntaxNode;

        // The IR-level code for this translation unit.
        // This will only be valid/non-null after semantic
        // checking and IR generation are complete, so it
        // is not safe to use this field without testing for NULL.
        RefPtr<IRModule> irModule;
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

    // A request to generate output in some target format
    class TargetRequest : public RefObject
    {
    public:
        CompileRequest*     compileRequest;
        CodeGenTarget       target;
        SlangTargetFlags    targetFlags = 0;
        Slang::Profile      targetProfile = Slang::Profile();
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;

        // Requested output paths for each entry point.
        // An empty string indices no output desired for
        // the given entry point.
        List<String> entryPointOutputPaths;

        // The resulting reflection layout information
        RefPtr<ProgramLayout> layout;

        // Generated compile results for each entry point
        // in the parent compile request (indexing matches
        // the order they are given in the compile request)
        List<CompileResult> entryPointResults;

        // TypeLayouts created on the fly by reflection API
        Dictionary<Type*, RefPtr<TypeLayout>> typeLayouts;

        MatrixLayoutMode getDefaultMatrixLayoutMode();
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
    Profile getEffectiveProfile(EntryPointRequest* entryPoint, TargetRequest* target);

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

    // Represents a module that has been loaded through the front-end
    // (up through IR generation).
    //
    class LoadedModule : public RefObject
    {
    public:
        // The AST for the module
        RefPtr<ModuleDecl>  moduleDecl;

        // The IR for the module
        RefPtr<IRModule> irModule = nullptr;
    };

    class Session;


    /// Create a blob that will retain (a copy of) raw data.
    ///
    ComPtr<ISlangBlob> createRawBlob(void const* data, size_t size);

    class CompileRequest : public RefObject
    {
    public:
        // Pointer to parent session
        Session* mSession;

        // Information on the targets we are being asked to
        // generate code for.
        List<RefPtr<TargetRequest>> targets;

        // What container format are we being asked to generate?
        ContainerFormat containerFormat = ContainerFormat::None;

        // Path to output container to
        String containerOutputPath;

        // Directories to search for `#include` files or `import`ed modules
        List<SearchDirectory> searchDirectories;

        // Definitions to provide during preprocessing
        Dictionary<String, String> preprocessorDefinitions;

        // Translation units we are being asked to compile
        List<RefPtr<TranslationUnitRequest> > translationUnits;

        // Entry points we've been asked to compile (each
        // associated with a translation unit).
        List<RefPtr<EntryPointRequest> > entryPoints;

        // Types constructed by reflection API
        Dictionary<String, RefPtr<Type>> types;

        /// The layout to use for matrices by default (row/column major)
        MatrixLayoutMode defaultMatrixLayoutMode = kMatrixLayoutMode_ColumnMajor;
        MatrixLayoutMode getDefaultMatrixLayoutMode() { return defaultMatrixLayoutMode; }

        // Should we just pass the input to another compiler?
        PassThroughMode passThrough = PassThroughMode::None;

        // Compile flags to be shared by all translation units
        SlangCompileFlags compileFlags = 0;

        // Should we dump intermediate results along the way, for debugging?
        bool shouldDumpIntermediates = false;

        bool shouldDumpIR = false;
        bool shouldValidateIR = false;
        bool shouldSkipCodegen = false;

        // If true then generateIR will serialize out IR, and serialize back in again. Making 
        // serialization a bottleneck or firewall between the front end and the backend
        bool useSerialIRBottleneck = false; 

        // How should `#line` directives be emitted (if at all)?
        LineDirectiveMode lineDirectiveMode = LineDirectiveMode::Default;

        // Are we being driven by the command-line `slangc`, and should act accordingly?
        bool isCommandLineCompile = false;

        // Source manager to help track files loaded
        SourceManager sourceManagerStorage;
        SourceManager* sourceManager;

        // Name pool for looking up names
        NamePool namePool;

        NamePool* getNamePool() { return &namePool; }

        // Output stuff
        DiagnosticSink mSink;
        String mDiagnosticOutput;

        /// A blob holding the diagnostic output
        ComPtr<ISlangBlob> diagnosticOutputBlob;

        // Files that compilation depended on
        List<String> mDependencyFilePaths;

        // Generated bytecode representation of all the code
        List<uint8_t> generatedBytecode;

        // Modules that have been dynamically loaded via `import`
        //
        // This is a list of unique modules loaded, in the order they were encountered.
        List<RefPtr<LoadedModule> > loadedModulesList;

        // Map from the path of a module file to its definition
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

        // For output
        ComPtr<ISlangWriter> m_writers[SLANG_WRITER_CHANNEL_COUNT_OF];

        void setWriter(WriterChannel chan, ISlangWriter* writer);
        ISlangWriter* getWriter(WriterChannel chan) const { return m_writers[int(chan)]; }

        /// Load a file into memory using the configured file system.
        ///
        /// @param path The path to attempt to load from
        /// @param outBlob A destination pointer to receive the loaded blob
        /// @returns A `SlangResult` to indicate success or failure.
        ///
        SlangResult loadFile(String const& path, ISlangBlob** outBlob);

        CompileRequest(Session* session);

        RefPtr<Expr> parseTypeString(TranslationUnitRequest * translationUnit, String typeStr, RefPtr<Scope> scope);

        Type* getTypeFromString(String typeStr);

        void parseTranslationUnit(
            TranslationUnitRequest* translationUnit);

        // Perform primary semantic checking on all
        // of the translation units in the program
        void checkAllTranslationUnits();

        void generateIR();

        SlangResult executeActionsInner();
        SlangResult executeActions();

        int addTranslationUnit(SourceLanguage language, String const& name);

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

        int addEntryPoint(
            int                     translationUnitIndex,
            String const&           name,
            Profile                 profile,
            List<String> const &    genericTypeNames);

        UInt addTarget(
            CodeGenTarget   target);

        RefPtr<ModuleDecl> loadModule(
            Name*               name,
            const PathInfo&     filePathInfo,
            ISlangBlob*         fileContentsBlob,
            SourceLoc const& loc);

        void loadParsedModule(
            RefPtr<TranslationUnitRequest> const&   translationUnit,
            Name*                                   name,
            PathInfo const&                         pathInfo);

        RefPtr<ModuleDecl> findOrImportModule(
            Name*               name,
            SourceLoc const&    loc);

        Decl* lookupGlobalDecl(Name* name);

        SourceManager* getSourceManager()
        {
            return sourceManager;
        }

        void setSourceManager(SourceManager* sm)
        {
            sourceManager = sm;
            mSink.sourceManager = sm;
        }

            /// During propagation of an exception for an internal
            /// error, note that this source location was involved
        void noteInternalErrorLoc(SourceLoc const& loc);

        int internalErrorLocsNoted = 0;
    };

    void generateOutput(
        CompileRequest* compileRequest);

    // Helper to dump intermediate output when debugging
    void maybeDumpIntermediate(
        CompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        CodeGenTarget   target);
    void maybeDumpIntermediate(
        CompileRequest* compileRequest,
        char const*     text,
        CodeGenTarget   target);

    /* Returns true if a codeGen target is available. */
    SlangResult checkCompileTargetSupport(Session* session, CodeGenTarget target);

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
    };

}

#endif
