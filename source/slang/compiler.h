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
    namespace Compiler
    {
        class ILConstOperand;
        struct IncludeHandler;

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

        // Describes an entry point that we've been requested to compile
        struct EntryPointOption
        {
            String name;
            Profile profile;
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

        // Options for a single translation unit being requested by the user
        class TranslationUnitOptions
        {
        public:
            SourceLanguage sourceLanguage = SourceLanguage::Unknown;

            // All entry points we've been asked to compile for this translation unit
            List<EntryPointOption> entryPoints;

            // The source file(s) that will be compiled to form this translation unit
            List<RefPtr<SourceFile> > sourceFiles;
        };

        class CompileOptions
        {
        public:
            CompilerMode Mode = CompilerMode::ProduceShader;
            CodeGenTarget Target = CodeGenTarget::Unknown;
            StageTarget stage = StageTarget::Unknown;
            EnumerableDictionary<String, String> BackendArguments;

            String SymbolToCompile;
            String outputName;
            List<String> TemplateShaderArguments;
            List<String> SearchDirectories;
            Dictionary<String, String> PreprocessorDefinitions;

            List<TranslationUnitOptions> translationUnits;

            // the code generation profile we've been asked to use
            Profile profile;

            // should we just pass the input to another compiler?
            PassThroughMode passThrough = PassThroughMode::None;

            // Flags supplied through the API
            SlangCompileFlags flags = 0;
        };

        // This is the representation of a given translation unit
        class CompileUnit
        {
        public:
            TranslationUnitOptions      options;
            RefPtr<ProgramSyntaxNode>   SyntaxNode;
        };

        // TODO: pick an appropriate name for this...
        class CollectionOfTranslationUnits : public RefObject
        {
        public:
            List<CompileUnit> translationUnits;

            // TODO: this is more output-oriented, but maybe okay to have here...
            RefPtr<ProgramLayout> layout;
        };

        class ShaderCompiler : public CoreLib::Basic::Object
        {
        public:
            virtual void Compile(
                CompileResult&                  result,
                CollectionOfTranslationUnits*   collectionOfTranslationUnits,
                const CompileOptions&           options) = 0;

            virtual TranslationUnitResult PassThrough(
                String const&			sourceText,
                String const&			sourcePath,
                const CompileOptions &	options,
                TranslationUnitOptions const& translationUnitOptions) = 0;

        };

        ShaderCompiler * CreateShaderCompiler();
    }
}

#endif