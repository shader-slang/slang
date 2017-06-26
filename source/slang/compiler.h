#ifndef RASTER_SHADER_COMPILER_H
#define RASTER_SHADER_COMPILER_H

#include "../core/basic.h"

#include "compiled-program.h"
#include "diagnostics.h"
#include "profile.h"
#include "syntax.h"
#include "type-layout.h"

#include "../../slang.h"

namespace Slang
{
    struct IncludeHandler;
    class CompileRequest;

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
        GLSL                = SLANG_GLSL,
        GLSL_Vulkan         = SLANG_GLSL_VULKAN,
        GLSL_Vulkan_OneDesc = SLANG_GLSL_VULKAN_ONE_DESC,
        HLSL                = SLANG_HLSL,
        SPIRV               = SLANG_SPIRV,
        SPIRVAssembly       = SLANG_SPIRV_ASM,
        DXBytecode          = SLANG_DXBC,
        DXBytecodeAssembly  = SLANG_DXBC_ASM,
        ReflectionJSON      = SLANG_REFLECTION_JSON,
    };

    class CompileRequest;
    class TranslationUnitRequest;

    // Result of compiling an entry point
    struct EntryPointResult
    {
        String outputSource;
    };

    // Describes an entry point that we've been requested to compile
    class EntryPointRequest : public RefObject
    {
    public:
        // The parent compile request
        CompileRequest* compileRequest = nullptr;

        // The name of the entry point function (e.g., `main`)
        String name;

        // The profile that the entry point will be compiled for
        // (this is a combination of the target state, and also
        // a feature level that sets capabilities)
        Profile profile;

        // The index of the translation unit (within the parent
        // compile request) that the entry point function is
        // supposed to be defined in.
        int translationUnitIndex;

        // The resulting output for the enry point
        //
        // TODO: low-level code generation should be a distinct step
        EntryPointResult result;

        // The translation unit that this entry point came from
        TranslationUnitRequest* getTranslationUnit();
    };

    enum class PassThroughMode : SlangPassThrough
    {
        None = SLANG_PASS_THROUGH_NONE,	// don't pass through: use Slang compiler
        HLSL = SLANG_PASS_THROUGH_FXC,	// pass through HLSL to `D3DCompile` API
//			GLSL,	// pass through GLSL to `glslang` library
    };

    // Represents a single source file (either an on-disk file, or a
    // "virtual" file passed in as a string)
    class SourceFile : public RefObject
    {
    public:
        // The file path for a real file, or the nominal path for a virtual file
        String path;

        // The actual contents of the file
        String content;
    };

    // Result of compiling a translation unit
    struct TranslationUnitResult
    {
        String outputSource;
    };

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
        RefPtr<ProgramSyntaxNode>   SyntaxNode;

        // The resulting output for the translation unit
        //
        // TODO: low-level code generation should be a distinct step
        TranslationUnitResult result;
    };

    // A directory to be searched when looking for files (e.g., `#include`)
    struct SearchDirectory
    {
        enum Kind
        {
            Default,
            AutoImport,
        };

        SearchDirectory() = default;
        SearchDirectory(SearchDirectory const& other) = default;
        SearchDirectory(String const& path, Kind kind)
            : path(path)
            , kind(kind)
        {}

        String  path;
        Kind    kind;
    };

    class Session;

    class CompileRequest : public RefObject
    {
    public:
        // Pointer to parent session
        Session* mSession;

        // What target language are we compiling to?
        CodeGenTarget Target = CodeGenTarget::Unknown;

        // Directories to search for `#include` files or `import`ed modules
        List<SearchDirectory> searchDirectories;

        // Definitions to provide during preprocessing
        Dictionary<String, String> preprocessorDefinitions;

        // Translation units we are being asked to compile
        List<RefPtr<TranslationUnitRequest> > translationUnits;

        // Entry points we've been asked to compile (each
        // assocaited with a translation unit).
        List<RefPtr<EntryPointRequest> > entryPoints;

        // The code generation profile we've been asked to use.
        Profile profile;

        // Should we just pass the input to another compiler?
        PassThroughMode passThrough = PassThroughMode::None;

        // Compile flags to be shared by all translation units
        SlangCompileFlags compileFlags = 0;

        // Output stuff
        DiagnosticSink mSink;
        String mDiagnosticOutput;

        // Files that compilation depended on
        List<String> mDependencyFilePaths;

        // The resulting reflection layout information
        RefPtr<ProgramLayout> layout;

        // Modules that have been dynamically loaded via `import`
        Dictionary<String, RefPtr<ProgramSyntaxNode>> loadedModulesMap;
        List<RefPtr<ProgramSyntaxNode> > loadedModulesList;


        CompileRequest(Session* session)
            : mSession(session)
        {}

        ~CompileRequest()
        {}

        void parseTranslationUnit(
            TranslationUnitRequest* translationUnit);

        void CompileRequest::checkTranslationUnit(
            TranslationUnitRequest* translationUnit);

        void checkAllTranslationUnits();

        int executeActionsInner();
        int executeActions();

        int addTranslationUnit(SourceLanguage language, String const& name);

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
            Profile                 profile);

        RefPtr<ProgramSyntaxNode> loadModule(
            String const&       name,
            String const&       path,
            String const&       source,
            CodePosition const& loc);

        String autoImportModule(
            String const&       path,
            String const&       source,
            CodePosition const& loc);

        RefPtr<ProgramSyntaxNode> findOrImportModule(
            String const&       name,
            CodePosition const& loc);
    };

    void generateOutput(
        CompileRequest* compileRequest);
}

#endif