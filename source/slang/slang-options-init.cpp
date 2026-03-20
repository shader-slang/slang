// slang-options-init.cpp
//
// Lightweight implementation of initCommandOptions(), extracted from
// slang-options.cpp so it can be compiled without heavy compiler
// dependencies (e.g. slang-compiler.h, slang-serialize-ir.h).

#include "slang-options.h"

#include "../compiler-core/slang-source-embed-util.h"
#include "../core/slang-command-options-writer.h"
#include "../core/slang-name-value.h"
#include "../core/slang-type-text-util.h"
#include "slang-capability.h"
#include "slang-compiler-options.h"
#include "slang-hlsl-to-vulkan-layout-options.h"
#include "slang-profile.h"

namespace Slang
{

namespace
{ // anonymous – avoids ODR conflict with the same types in slang-options.cpp

typedef CompilerOptionName OptionKind;

struct Option
{
    OptionKind optionKind;
    const char* name;
    const char* usage = nullptr;
    const char* description = nullptr;
    const char* displayName = nullptr;
    const CommandOptions::InputLink* links = nullptr;
    Count linkCount = 0;
};

enum class ValueCategory
{
    Compiler,
    Target,
    Language,
    FloatingPointMode,
    FloatingPointDenormalMode,
    ArchiveType,
    Stage,
    LineDirectiveMode,
    DebugInfoFormat,
    HelpStyle,
    OptimizationLevel,
    DebugLevel,
    FileSystemType,
    VulkanShift,
    SourceEmbedStyle,
    LanguageVersion,

    CountOf,
};

template<typename T>
struct GetValueCategory;

#define SLANG_GET_VALUE_CATEGORY(cat, type)   \
    template<>                                \
    struct GetValueCategory<type>             \
    {                                         \
        enum                                  \
        {                                     \
            Value = Index(ValueCategory::cat) \
        };                                    \
    };

SLANG_GET_VALUE_CATEGORY(Compiler, SlangPassThrough)
SLANG_GET_VALUE_CATEGORY(ArchiveType, SlangArchiveType)
SLANG_GET_VALUE_CATEGORY(LineDirectiveMode, SlangLineDirectiveMode)
SLANG_GET_VALUE_CATEGORY(FloatingPointMode, FloatingPointMode)
SLANG_GET_VALUE_CATEGORY(FloatingPointDenormalMode, FloatingPointDenormalMode)
SLANG_GET_VALUE_CATEGORY(FileSystemType, TypeTextUtil::FileSystemType)
SLANG_GET_VALUE_CATEGORY(HelpStyle, CommandOptionsWriter::Style)
SLANG_GET_VALUE_CATEGORY(OptimizationLevel, SlangOptimizationLevel)
SLANG_GET_VALUE_CATEGORY(VulkanShift, HLSLToVulkanLayoutOptions::Kind)
SLANG_GET_VALUE_CATEGORY(SourceEmbedStyle, SourceEmbedUtil::Style)
SLANG_GET_VALUE_CATEGORY(Language, SourceLanguage)

static void _addOptions(const ConstArrayView<Option>& options, CommandOptions& cmdOptions)
{
    for (auto& opt : options)
    {
        cmdOptions.add(
            opt.name,
            opt.usage,
            opt.description,
            CommandOptions::UserValue(opt.optionKind),
            opt.displayName,
            opt.links,
            opt.linkCount);
    }
}

} // anonymous namespace

void initCommandOptions(CommandOptions& options)
{
    typedef CommandOptions::Flag::Enum Flag;
    typedef CommandOptions::CategoryKind CategoryKind;
    typedef CommandOptions::UserValue UserValue;

    // Add all the option categories

    options.addCategory(CategoryKind::Option, "General", "General options");
    options.addCategory(CategoryKind::Option, "Target", "Target code generation options");
    options.addCategory(CategoryKind::Option, "Downstream", "Downstream compiler options");
    options.addCategory(
        CategoryKind::Option,
        "Debugging",
        "Compiler debugging/instrumentation options");
    options.addCategory(CategoryKind::Option, "Repro", "Slang repro system related");
    options.addCategory(
        CategoryKind::Option,
        "Experimental",
        "Experimental options (use at your own risk)");
    options.addCategory(
        CategoryKind::Option,
        "Internal",
        "Internal-use options (use at your own risk)");
    options.addCategory(
        CategoryKind::Option,
        "Deprecated",
        "Deprecated options (allowed but ignored; may be removed in future)");

    // Do the easy ones
    {
        options.addCategory(
            CategoryKind::Value,
            "compiler",
            "Downstream Compilers (aka Pass through)",
            UserValue(ValueCategory::Compiler));
        options.addValues(TypeTextUtil::getCompilerInfos());

        options.addCategory(
            CategoryKind::Value,
            "language",
            "Language",
            UserValue(ValueCategory::Language));
        options.addValues(TypeTextUtil::getLanguageInfos());

        options.addCategory(
            CategoryKind::Value,
            "language-version",
            "Language Version",
            UserValue(ValueCategory::LanguageVersion));
        options.addValues(TypeTextUtil::getLanguageVersionInfos());


        options.addCategory(
            CategoryKind::Value,
            "archive-type",
            "Archive Type",
            UserValue(ValueCategory::ArchiveType));
        options.addValues(TypeTextUtil::getArchiveTypeInfos());

        options.addCategory(
            CategoryKind::Value,
            "line-directive-mode",
            "Line Directive Mode",
            UserValue(ValueCategory::LineDirectiveMode));
        options.addValues(TypeTextUtil::getLineDirectiveInfos());

        options.addCategory(
            CategoryKind::Value,
            "debug-info-format",
            "Debug Info Format",
            UserValue(ValueCategory::DebugInfoFormat));
        options.addValues(TypeTextUtil::getDebugInfoFormatInfos());

        options.addCategory(
            CategoryKind::Value,
            "fp-mode",
            "Floating Point Mode",
            UserValue(ValueCategory::FloatingPointMode));
        options.addValues(TypeTextUtil::getFloatingPointModeInfos());

        options.addCategory(
            CategoryKind::Value,
            "fp-denormal-mode",
            "Floating Point Denormal Handling Mode",
            UserValue(ValueCategory::FloatingPointDenormalMode));
        options.addValues(TypeTextUtil::getFpDenormalModeInfos());

        options.addCategory(
            CategoryKind::Value,
            "help-style",
            "Help Style",
            UserValue(ValueCategory::HelpStyle));
        options.addValues(CommandOptionsWriter::getStyleInfos());

        options.addCategory(
            CategoryKind::Value,
            "optimization-level",
            "Optimization Level",
            UserValue(ValueCategory::OptimizationLevel));
        options.addValues(TypeTextUtil::getOptimizationLevelInfos());

        options.addCategory(
            CategoryKind::Value,
            "debug-level",
            "Debug Level",
            UserValue(ValueCategory::DebugLevel));
        options.addValues(TypeTextUtil::getDebugLevelInfos());

        options.addCategory(
            CategoryKind::Value,
            "file-system-type",
            "File System Type",
            UserValue(ValueCategory::FileSystemType));
        options.addValues(TypeTextUtil::getFileSystemTypeInfos());

        options.addCategory(
            CategoryKind::Value,
            "source-embed-style",
            "Source Embed Style",
            UserValue(ValueCategory::SourceEmbedStyle));
        options.addValues(SourceEmbedUtil::getStyleInfos());
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! target !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options
            .addCategory(CategoryKind::Value, "target", "Target", UserValue(ValueCategory::Target));
        for (auto opt : TypeTextUtil::getCompileTargetInfos())
        {
            options.addValue(opt.names, opt.description, UserValue(opt.target));
        }
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! stage !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(CategoryKind::Value, "stage", "Stage", UserValue(ValueCategory::Stage));

        struct StageInfoData
        {
            const char* name;
            Stage stage;
        };
        static const StageInfoData kStages[] = {
#define PROFILE_STAGE(ID, NAME, ...) {#NAME, Stage::ID},
#define PROFILE_STAGE_ALIAS(ID, NAME, VAL) {#NAME, Stage::ID},
#include "slang-profile-defs.h"
        };

        List<NameValue> opts;
        for (auto& info : makeConstArrayView(kStages))
            opts.add({ValueInt(info.stage), info.name});
        options.addValuesWithAliases(opts.getArrayView());
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! vulkan-shift !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(
            CategoryKind::Value,
            "vulkan-shift",
            "Vulkan Shift",
            UserValue(ValueCategory::VulkanShift));
        options.addValues(HLSLToVulkanLayoutOptions::getKindInfos());
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! capabilities !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(
            CategoryKind::Value,
            "capability",
            "A capability describes an optional feature that a target may or "
            "may not support. When a -capability is specified, the compiler "
            "may assume that the target supports that capability, and generate "
            "code accordingly.");

        List<UnownedStringSlice> names;
        getCapabilityNames(names);

        // We'll just add to keep the list more simple...
        options.addValue("spirv_1_{ 0,1,2,3,4,5 }", "minimum supported SPIR - V version");

        for (auto name : names)
        {
            if (name.startsWith("__") || name.startsWith("spirv_1_") || name.startsWith("_") ||
                name == "Invalid")
            {
                continue;
            }
            else if (name.startsWith("GL_") || name.startsWith("SPV_") || name.startsWith("GLSL_"))
            {
                // We'll assume it is an extension..
                StringBuilder buf;
                buf << "enables the " << name << " extension";
                options.addValue(name, buf.getUnownedSlice());
            }
            else
            {
                options.addValue(name);
            }
        }
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! extension !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(
            CategoryKind::Value,
            "file-extension",
            "A <language>, <format>, and/or <stage> may be inferred from the "
            "extension of an input or -o path");

        // TODO(JS): It's concevable that these are enumerated via some other system
        // rather than just being listed here

        const CommandOptions::ValuePair pairs[] = {
            {"hlsl,fx", "hlsl"},
            {"dxbc", nullptr},
            {"dxbc-asm", "dxbc-assembly"},
            {"dxil", nullptr},
            {"dxil-asm", "dxil-assembly"},
            {"glsl", nullptr},
            {"vert", "glsl (vertex)"},
            {"frag", "glsl (fragment)"},
            {"geom", "glsl (geoemtry)"},
            {"tesc", "glsl (hull)"},
            {"tese", "glsl (domain)"},
            {"comp", "glsl (compute)"},
            {"mesh", "glsl (mesh)"},
            {"task", "glsl (amplification)"},
            {"slang", nullptr},
            {"spv", "SPIR-V"},
            {"spv-asm", "SPIR-V assembly"},
            {"c", nullptr},
            {"cpp,c++,cxx", "C++"},
            {"hpp", "C++ Header"},
            {"exe", "executable"},
            {"dll,so", "sharedlibrary/dll"},
            {"cu", "CUDA"},
            {"cuh", "CUDA Header"},
            {"ptx", "PTX"},
            {"obj,o", "object-code"},
            {"zip", "container"},
            {"slang-module,slang-library", "Slang Module/Library"},
            {"dir", "Container as a directory"},
        };
        options.addValues(pairs, SLANG_COUNT_OF(pairs));
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! help-category !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(
            CategoryKind::Value,
            "help-category",
            "Available help categories for the -h option");

        // Add all existing categories as valid help category values
        const auto& categories = options.getCategories();
        for (Index categoryIndex = 0; categoryIndex < categories.getCount(); ++categoryIndex)
        {
            const auto& category = categories[categoryIndex];
            options.addValue(category.name, category.description);
        }
    }


    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! General !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("General");

    const Option generalOpts[] = {
        {OptionKind::MacroDefine,
         "-D?...",
         "-D<name>[=<value>], -D <name>[=<value>]",
         "Insert a preprocessor macro.\n"
         "The space between - D and <name> is optional. If no <value> is specified, Slang will "
         "define the macro with an empty value."},
        {OptionKind::DepFile,
         "-depfile",
         "-depfile <path>",
         "Save the source file dependency list in a file."},
        {OptionKind::EntryPointName,
         "-entry",
         "-entry <name>",
         "Specify the name of an entry-point function.\n"
         "When compiling from a single file, this defaults to main if you specify a stage using "
         "-stage.\n"
         "Multiple -entry options may be used in a single invocation. "
         "When they do, the file associated with the entry point will be the first one found when "
         "searching to the left in the command line.\n"
         "If no -entry options are given, compiler will use [shader(...)] "
         "attributes to detect entry points."},
        {OptionKind::Specialize,
         "-specialize",
         "-specialize <typename>",
         "Specialize the last entrypoint with <typename>.\n"},
        {OptionKind::EmitIr,
         "-emit-ir",
         nullptr,
         "Emit IR typically as a '.slang-module' when outputting to a container."},
        {OptionKind::Help,
         "-h,-help,--help",
         "-h or -h <help-category>",
         "Print this message, or help in specified category."},
        {OptionKind::HelpStyle, "-help-style", "-help-style <help-style>", "Help formatting style"},
        {OptionKind::Include,
         "-I?...",
         "-I<path>, -I <path>",
         "Add a path to be used in resolving '#include' "
         "and 'import' operations."},
        {OptionKind::Language,
         "-lang",
         "-lang <language>",
         "Set the language for the following input files."},
        {OptionKind::MatrixLayoutColumn,
         "-matrix-layout-column-major",
         nullptr,
         "Set the default matrix layout to column-major."},
        {OptionKind::MatrixLayoutRow,
         "-matrix-layout-row-major",
         nullptr,
         "Set the default matrix layout to row-major."},
        {OptionKind::RestrictiveCapabilityCheck,
         "-restrictive-capability-check",
         nullptr,
         "Many capability warnings will become an error."},
        {OptionKind::IgnoreCapabilities,
         "-ignore-capabilities",
         nullptr,
         "Do not warn or error if capabilities are violated"},
        {OptionKind::MinimumSlangOptimization,
         "-minimum-slang-optimization",
         nullptr,
         "Perform minimum code optimization in Slang to favor compilation time."},
        {OptionKind::DisableNonEssentialValidations,
         "-disable-non-essential-validations",
         nullptr,
         "Disable non-essential IR validations such as use of uninitialized variables."},
        {OptionKind::DisableSourceMap,
         "-disable-source-map",
         nullptr,
         "Disable source mapping in the Obfuscation."},
        {OptionKind::ModuleName,
         "-module-name",
         "-module-name <name>",
         "Set the module name to use when compiling multiple .slang source files into a single "
         "module."},
        {OptionKind::Output,
         "-o",
         "-o <path>",
         "Specify a path where generated output should be written.\n"
         "If no -target or -stage is specified, one may be inferred "
         "from file extension (see <file-extension>). "
         "If multiple -target options and a single -entry are present, each -o "
         "associates with the first -target to its left. "
         "Otherwise, if multiple -entry options are present, each -o associates "
         "with the first -entry to its left, and with the -target that matches "
         "the one inferred from <path>."},
        {OptionKind::Profile,
         "-profile",
         "-profile <profile>[+<capability>...]",
         "Specify the shader profile for code generation.\n"
         "Accepted profiles are:\n"
         "* sm_{4_0,4_1,5_0,5_1,6_0,6_1,6_2,6_3,6_4,6_5,6_6}\n"
         "* glsl_{110,120,130,140,150,330,400,410,420,430,440,450,460}\n"
         "Additional profiles that include -stage information:\n"
         "* {vs,hs,ds,gs,ps}_<version>\n"
         "See -capability for information on <capability>\n"
         "When multiple -target options are present, each -profile associates "
         "with the first -target to its left."},
        {OptionKind::Stage,
         "-stage",
         "-stage <stage>",
         "Specify the stage of an entry-point function.\n"
         "When multiple -entry options are present, each -stage associated with "
         "the first -entry to its left.\n"
         "May be omitted if entry-point function has a [shader(...)] attribute; "
         "otherwise required for each -entry option."},
        {OptionKind::Target,
         "-target",
         "-target <target>",
         "Specifies the format in which code should be generated."},
        {OptionKind::Version,
         "-v,-version",
         nullptr,
         "Display the build version. This is the contents of git describe --tags.\n"
         "It is typically only set from automated builds(such as distros available on github).A "
         "user build will by default be 'unknown'."},
        {OptionKind::LanguageVersion,
         "-std",
         "-std <language-version>",
         "Specifies the language standard that should be used."},
        {OptionKind::WarningsAsErrors,
         "-warnings-as-errors",
         "-warnings-as-errors all or -warnings-as-errors <id>[,<id>...]",
         "all - Treat all warnings as errors.\n"
         "<id>[,<id>...]: Treat specific warning ids as errors.\n"},
        {OptionKind::DisableWarnings,
         "-warnings-disable",
         "-warnings-disable <id>[,<id>...]",
         "Disable specific warning ids."},
        {OptionKind::EnableWarning, "-W...", "-W<id>", "Enable a warning with the specified id."},
        {OptionKind::DisableWarning, "-Wno-...", "-Wno-<id>", "Disable warning with <id>"},
        {OptionKind::DumpWarningDiagnostics,
         "-dump-warning-diagnostics",
         nullptr,
         "Dump to output list of warning diagnostic numeric and name ids."},
        {OptionKind::InputFilesRemain,
         "--",
         nullptr,
         "Treat the rest of the command line as input files."},
        {OptionKind::ReportDownstreamTime,
         "-report-downstream-time",
         nullptr,
         "Reports the time spent in the downstream compiler."},
        {OptionKind::ReportPerfBenchmark,
         "-report-perf-benchmark",
         nullptr,
         "Reports compiler performance benchmark results."},
        {OptionKind::ReportDetailedPerfBenchmark,
         "-report-detailed-perf-benchmark",
         nullptr,
         "Reports compiler performance benchmark results for each intermediate pass (implies "
         "-report-perf-benchmark)."},
        {OptionKind::ReportCheckpointIntermediates,
         "-report-checkpoint-intermediates",
         nullptr,
         "Reports information about checkpoint contexts used for reverse-mode automatic "
         "differentiation."},
        {OptionKind::ReportDynamicDispatchSites,
         "-report-dynamic-dispatch-sites",
         nullptr,
         "Reports information about dynamic dispatch sites for interface calls."},
        {OptionKind::SkipSPIRVValidation,
         "-skip-spirv-validation",
         nullptr,
         "Skips spirv validation."},
        {OptionKind::SourceEmbedStyle,
         "-source-embed-style",
         "-source-embed-style <source-embed-style>",
         "If source embedding is enabled, defines the style used. When enabled (with any style "
         "other than `none`), "
         "will write compile results into embeddable source for the target language. "
         "If no output file is specified, the output is written to stdout. If an output file is "
         "specified "
         "it is written either to that file directly (if it is appropriate for the target "
         "language), "
         "or it will be output to the filename with an appropriate extension.\n\n"
         "Note for C/C++ with u16/u32/u64 types it is necessary to have \"#include <stdint.h>\" "
         "before the generated file.\n"},
        {OptionKind::SourceEmbedName,
         "-source-embed-name",
         "-source-embed-name <name>",
         "The name used as the basis for variables output for source embedding."},
        {OptionKind::SourceEmbedLanguage,
         "-source-embed-language",
         "-source-embed-language <language>",
         "The language to be used for source embedding. Defaults to C/C++. Currently only C/C++ "
         "are supported"},
        {OptionKind::DisableShortCircuit,
         "-disable-short-circuit",
         nullptr,
         "Disable short-circuiting for \"&&\" and \"||\" operations"},
        {OptionKind::UnscopedEnum,
         "-unscoped-enum",
         nullptr,
         "Treat enums types as unscoped by default."},
        {OptionKind::PreserveParameters,
         "-preserve-params",
         nullptr,
         "Preserve all resource parameters in the output code, even if they are not used by the "
         "shader."},
        {OptionKind::TypeConformance,
         "-conformance",
         "-conformance <typeName>:<interfaceName>[=<sequentialID>]",
         "Include additional type conformance during linking for dynamic dispatch."},
        {OptionKind::EmitReflectionJSON,
         "-reflection-json",
         "-reflection-json <path>",
         "Emit reflection data in JSON format to a file."},
        {OptionKind::UseMSVCStyleBitfieldPacking,
         "-msvc-style-bitfield-packing",
         nullptr,
         "Pack bitfields according to MSVC rules (msb first, new field when underlying type size "
         "changes) rather than gcc-style (lsb first)"}};

    _addOptions(makeConstArrayView(generalOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Target !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Target");

    StringBuilder vkShiftNames;
    {
        for (auto nameSlice : NameValueUtil::getNames(
                 NameValueUtil::NameKind::All,
                 HLSLToVulkanLayoutOptions::getKindInfos()))
        {
            // -fvk-{b|s|t|u}-shift
            vkShiftNames << "-fvk-" << nameSlice << "-shift,";
        }
        // remove last ,
        vkShiftNames.reduceLength(vkShiftNames.getLength() - 1);
    }

    static const CommandOptions::InputLink kVulkanBindShiftLinks[] = {
        {"DXC description",
         "https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/"
         "SPIR-V.rst#implicit-binding-number-assignment"},
        {"GLSL wiki",
         "https://github.com/KhronosGroup/glslang/wiki/"
         "HLSL-FAQ#auto-mapped-binding-numbers"},
    };

    static const CommandOptions::InputLink kVulkanBindGlobalsLinks[] = {
        {"DXC description",
         "https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/"
         "SPIR-V.rst#implicit-binding-number-assignment"},
    };

    const Option targetOpts[] = {
        {OptionKind::Capability,
         "-capability",
         "-capability <capability>[+<capability>...]",
         "Add optional capabilities to a code generation target. See Capabilities below."},
        {OptionKind::DefaultImageFormatUnknown,
         "-default-image-format-unknown",
         nullptr,
         "Set the format of R/W images with unspecified format to 'unknown'. Otherwise try to "
         "guess the format."},
        {OptionKind::DisableDynamicDispatch,
         "-disable-dynamic-dispatch",
         nullptr,
         "Disables generating dynamic dispatch code."},
        {OptionKind::DisableSpecialization,
         "-disable-specialization",
         nullptr,
         "Disables generics and specialization pass."},
        {OptionKind::FloatingPointMode,
         "-fp-mode,-floating-point-mode",
         "-fp-mode <fp-mode>, -floating-point-mode <fp-mode>",
         "Control floating point optimizations"},
        {OptionKind::DenormalModeFp16,
         "-denorm-mode-fp16",
         "-denorm-mode-fp16 <fp-denormal-mode>",
         "Control handling of 16-bit denormal floating point values in SPIR-V (any, preserve, "
         "ftz)"},
        {OptionKind::DenormalModeFp32,
         "-denorm-mode-fp32",
         "-denorm-mode-fp32 <fp-denormal-mode>",
         "Control handling of 32-bit denormal floating point values in SPIR-V and DXIL (any, "
         "preserve, ftz)"},
        {OptionKind::DenormalModeFp64,
         "-denorm-mode-fp64",
         "-denorm-mode-fp64 <fp-denormal-mode>",
         "Control handling of 64-bit denormal floating point values in SPIR-V (any, preserve, "
         "ftz)"},
        {OptionKind::DebugInformation,
         "-g...",
         "-g, -g<debug-info-format>, -g<debug-level>",
         "Include debug information in the generated code, where possible.\n"
         "<debug-level> is the amount of information, 0..3, unspecified means 2\n"
         "<debug-info-format> specifies a debugging info format\n"
         "It is valid to have multiple -g options, such as a <debug-level> and a "
         "<debug-info-format>"},
        {OptionKind::LineDirectiveMode,
         "-line-directive-mode",
         "-line-directive-mode <line-directive-mode>",
         "Sets how the `#line` directives should be produced. Available options are:\n"
         "If not specified, default behavior is to use C-style `#line` directives "
         "for HLSL and C/C++ output, and traditional GLSL-style `#line` directives "
         "for GLSL output."},
        {OptionKind::Optimization,
         "-O...",
         "-O<optimization-level>",
         "Set the optimization level."},
        {OptionKind::Obfuscate,
         "-obfuscate",
         nullptr,
         "Remove all source file information from outputs."},
        {OptionKind::GLSLForceScalarLayout,
         "-force-glsl-scalar-layout,-fvk-use-scalar-layout",
         nullptr,
         "Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, "
         "ByteAddressBuffer and general pointers follow the 'scalar' layout when targeting GLSL or "
         "SPIRV."},
        {OptionKind::ForceDXLayout,
         "-fvk-use-dx-layout",
         nullptr,
         "Pack members using FXCs member packing rules when targeting GLSL or SPIRV."},
        {OptionKind::ForceCLayout,
         "-fvk-use-c-layout",
         nullptr,
         "Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, "
         "ByteAddressBuffer and general pointers follow the C/C++ structure layout rules "
         "when targeting SPIRV."},
        {OptionKind::VulkanBindShift,
         vkShiftNames.getBuffer(),
         "-fvk-<vulkan-shift>-shift <N> <space>",
         "For example '-fvk-b-shift <N> <space>' shifts by N the inferred binding numbers for all "
         "resources in 'b' registers of space <space>. "
         "For a resource attached with :register(bX, <space>) but not [vk::binding(...)], "
         "sets its Vulkan descriptor set to <space> and binding number to X + N. If you need to "
         "shift the "
         "inferred binding numbers for more than one space, provide more than one such option. "
         "If more than one such option is provided for the same space, the last one takes effect. "
         "If you need to shift the inferred binding numbers for all sets, use 'all' as <space>.",
         "-fvk-<vulkan-shift>-shift",
         kVulkanBindShiftLinks,
         SLANG_COUNT_OF(kVulkanBindShiftLinks)},
        {OptionKind::VulkanBindGlobals,
         "-fvk-bind-globals",
         "-fvk-bind-globals <N> <descriptor-set>",
         "Places the $Globals cbuffer at descriptor set <descriptor-set> and binding <N>.\n"
         "It lets you specify the descriptor for the source at a certain register.",
         nullptr,
         kVulkanBindGlobalsLinks,
         SLANG_COUNT_OF(kVulkanBindGlobalsLinks)},
        {OptionKind::VulkanInvertY,
         "-fvk-invert-y",
         nullptr,
         "Negates (additively inverts) SV_Position.y before writing to stage output."},
        {OptionKind::VulkanUseDxPositionW,
         "-fvk-use-dx-position-w",
         nullptr,
         "Reciprocates (multiplicatively inverts) SV_Position.w after reading from stage input. "
         "For use in fragment shaders only."},
        {OptionKind::VulkanUseEntryPointName,
         "-fvk-use-entrypoint-name",
         nullptr,
         "Uses the entrypoint name from the source instead of 'main' in the spirv output."},
        {OptionKind::VulkanUseGLLayout,
         "-fvk-use-gl-layout",
         nullptr,
         "Use std430 layout instead of D3D buffer layout for raw buffer load/stores."},
        {OptionKind::VulkanEmitReflection,
         "-fspv-reflect",
         nullptr,
         "Include reflection decorations in the resulting SPIRV for shader parameters."},
        {OptionKind::EnableEffectAnnotations,
         "-enable-effect-annotations",
         nullptr,
         "Enables support for legacy effect annotation syntax."},
        {OptionKind::EmitSpirvViaGLSL,
         "-emit-spirv-via-glsl",
         nullptr,
         "Generate SPIR-V output by compiling generated GLSL with glslang"},
        {OptionKind::EmitSpirvDirectly,
         "-emit-spirv-directly",
         nullptr,
         "Generate SPIR-V output directly (default)"},
        {OptionKind::SPIRVCoreGrammarJSON,
         "-spirv-core-grammar",
         nullptr,
         "A path to a specific spirv.core.grammar.json to use when generating SPIR-V output"},
        {OptionKind::IncompleteLibrary,
         "-incomplete-library",
         nullptr,
         "Allow generating code from incomplete libraries with unresolved external functions"},
        {OptionKind::BindlessSpaceIndex,
         "-bindless-space-index",
         "-bindless-space-index <index>",
         "Specify the space index for the system defined global bindless resource array."},
        {OptionKind::SPIRVResourceHeapStride,
         "-spirv-resource-heap-stride",
         "-spirv-resource-heap-stride <stride>",
         "Specify the byte stride for the resource descriptor heap when generating SPIRV with "
         "spvDescriptorHeapEXT. Defaults to 0, which will use OpConstantSizeOfEXT(ResourceType)."},
        {OptionKind::SPIRVSamplerHeapStride,
         "-spirv-sampler-heap-stride",
         "-spirv-sampler-heap-stride <stride>",
         "Specify the byte stride for the sampler descriptor heap when generating SPIRV with "
         "spvDescriptorHeapEXT. Defaults to 0, which will use OpConstantSizeOfEXT(OpTypeSampler)."},
        {OptionKind::EmitSeparateDebug,
         "-separate-debug-info",
         nullptr,
         "Emit debug data to a separate file, and strip it from the main output file."},
        {OptionKind::EmitCPUViaCPP,
         "-emit-cpu-via-cpp",
         nullptr,
         "Generate CPU targets using C++ (default)"},
        {OptionKind::EmitCPUViaLLVM,
         "-emit-cpu-via-llvm",
         nullptr,
         "Generate CPU targets using LLVM"},
        {OptionKind::LLVMTargetTriple,
         "-llvm-target-triple",
         "-llvm-target-triple <target triple>",
         "Sets the target triple for the LLVM target, enabling cross "
         "compilation. The default value is the host platform."},
        {OptionKind::LLVMCPU,
         "-llvm-cpu",
         "-llvm-cpu <cpu name>",
         "Sets the target CPU for the LLVM target, enabling the extensions and "
         "features of that CPU. The default value is \"generic\"."},
        {OptionKind::LLVMFeatures,
         "-llvm-features",
         "-llvm-features <a1,+enable,-disable,...>",
         "Sets a comma-separates list of architecture-specific features for the LLVM targets."},
    };

    _addOptions(makeConstArrayView(targetOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Downstream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Downstream");

    {
        auto namesList = NameValueUtil::getNames(
            NameValueUtil::NameKind::First,
            TypeTextUtil::getCompilerInfos());
        StringBuilder names;
        for (auto name : namesList)
        {
            names << "-" << name << "-path,";
        }
        // remove last ,
        names.reduceLength(names.getLength() - 1);

        options.add(
            names.getBuffer(),
            "-<compiler>-path <path>",
            "Specify path to a downstream <compiler> "
            "executable or library.\n",
            UserValue(OptionKind::CompilerPath),
            "-<compiler>-path");
    }

    const Option downstreamOpts[] = {
        {OptionKind::DefaultDownstreamCompiler,
         "-default-downstream-compiler",
         "-default-downstream-compiler <language> <compiler>",
         "Set a default compiler for the given language. See -lang for the list of languages."},
        {OptionKind::DownstreamArgs,
         "-X...",
         "-X<compiler> <option> -X<compiler>... <options> -X.",
         "Pass arguments to downstream <compiler>. Just -X<compiler> passes just the next argument "
         "to the downstream compiler. -X<compiler>... options -X. will pass *all* of the options "
         "inbetween the opening -X and -X. to the downstream compiler."},
        {OptionKind::PassThrough,
         "-pass-through",
         "-pass-through <compiler>",
         "Pass the input through mostly unmodified to the "
         "existing compiler <compiler>.\n"
         "These are intended for debugging/testing purposes, when you want to be able to see what "
         "these existing compilers do with the \"same\" input and options"},
    };

    _addOptions(makeConstArrayView(downstreamOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Repro !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Repro");

    const Option reproOpts[] = {
        {OptionKind::DumpReproOnError,
         "-dump-repro-on-error",
         nullptr,
         "Dump `.slang-repro` file on any compilation error."},
        {OptionKind::ExtractRepro,
         "-extract-repro",
         "-extract-repro <name>",
         "Extract the repro files into a folder."},
        {OptionKind::LoadReproDirectory,
         "-load-repro-directory",
         "-load-repro-directory <path>",
         "Use repro along specified path"},
        {OptionKind::LoadRepro, "-load-repro", "-load-repro <name>", "Load repro"},
        {OptionKind::ReproFileSystem,
         "-repro-file-system",
         "-repro-file-system <name>",
         "Use a repro as a file system"},
        {OptionKind::DumpRepro,
         "-dump-repro",
         nullptr,
         "Dump a `.slang-repro` file that can be used to reproduce "
         "a compilation on another machine.\n"},
        {OptionKind::ReproFallbackDirectory,
         "-repro-fallback-directory <path>",
         "Specify a directory to use if a file isn't found in a repro. Should be specified "
         "*before* any repro usage such as `load-repro`. \n"
         "There are two *special* directories: \n\n"
         " * 'none:' indicates no fallback, so if the file isn't found in the repro compliation "
         "will fail\n"
         " * 'default:' is the default (which is the OS file system)"}};

    _addOptions(makeConstArrayView(reproOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Debugging !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Debugging");

    const Option debuggingOpts[] = {
        {OptionKind::DumpAst,
         "-dump-ast",
         nullptr,
         "Dump the AST to a .slang-ast file next to the input."},
        {OptionKind::DumpIntermediatePrefix,
         "-dump-intermediate-prefix",
         "-dump-intermediate-prefix <prefix>",
         "File name prefix for -dump-intermediates outputs, default is no prefix"},
        {OptionKind::DumpIntermediates,
         "-dump-intermediates",
         nullptr,
         "Dump intermediate outputs for debugging."},
        {OptionKind::DumpIr, "-dump-ir", nullptr, "Dump the IR after every pass for debugging."},
        {OptionKind::DumpIrIds,
         "-dump-ir-ids",
         nullptr,
         "Dump the IDs with -dump-ir (debug builds only)"},
        {OptionKind::PreprocessorOutput,
         "-E,-output-preprocessor",
         nullptr,
         "Output the preprocessing result and exit."},
        {OptionKind::NoCodeGen,
         "-no-codegen",
         nullptr,
         "Skip the code generation step, just check the code and generate layout."},
        {OptionKind::OutputIncludes,
         "-output-includes",
         nullptr,
         "Print the hierarchy of the processed source files."},
        {OptionKind::REMOVED_SerialIR,
         "-serial-ir",
         nullptr,
         "[REMOVED] Serialize the IR between front-end and back-end."},
        {OptionKind::SkipCodeGen, "-skip-codegen", nullptr, "Skip the code generation phase."},
        {OptionKind::ValidateIr,
         "-validate-ir",
         nullptr,
         "Validate the IR after select intermediate passes."},
        {OptionKind::ValidateIRDetailed,
         "-validate-ir-detailed",
         nullptr,
         "Perform debug validation on IR after each intermediate pass."},
        {OptionKind::DumpIRBefore,
         "-dump-ir-before",
         "-dump-ir-before <pass-names>",
         "Dump IR before specified pass, may be specified more than once"},
        {OptionKind::DumpIRAfter,
         "-dump-ir-after",
         "-dump-ir-after <pass-names>",
         "Dump IR after specified pass, may be specified more than once"},
        {OptionKind::VerbosePaths,
         "-verbose-paths",
         nullptr,
         "When displaying diagnostic output aim to display more detailed path information. "
         "In practice this is typically the complete 'canonical' path to the source file used."},
        {OptionKind::VerifyDebugSerialIr,
         "-verify-debug-serial-ir",
         nullptr,
         "Verify IR in the front-end."},
        {OptionKind::DumpModule, "-dump-module", nullptr, "Disassemble and print the module IR."},
        {OptionKind::GetModuleInfo,
         "-get-module-info",
         nullptr,
         "Print the name and version of a serialized IR Module"},
        {OptionKind::GetSupportedModuleVersions,
         "-get-supported-module-versions",
         nullptr,
         "Print the minimum and maximum module versions this compiler supports"}};
    _addOptions(makeConstArrayView(debuggingOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Experimental !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Experimental");

    const Option experimentalOpts[] = {
        {OptionKind::FileSystem,
         "-file-system",
         "-file-system <file-system-type>",
         "Set the filesystem hook to use for a compile request."},
        {OptionKind::Heterogeneous,
         "-heterogeneous",
         nullptr,
         "Output heterogeneity-related code."},
        {OptionKind::NoMangle,
         "-no-mangle",
         nullptr,
         "Do as little mangling of names as possible."},
        {OptionKind::NoHLSLBinding,
         "-no-hlsl-binding",
         nullptr,
         "Do not include explicit parameter binding semantics in the output HLSL code,"
         "except for parameters that has explicit bindings in the input source."},
        {OptionKind::NoHLSLPackConstantBufferElements,
         "-no-hlsl-pack-constant-buffer-elements",
         nullptr,
         "Do not pack elements of constant buffers into structs in the output HLSL code."},
        {OptionKind::ValidateUniformity,
         "-validate-uniformity",
         nullptr,
         "Perform uniformity validation analysis."},
        {OptionKind::AllowGLSL, "-allow-glsl", nullptr, "Enable GLSL as an input language."},
        {OptionKind::EnableExperimentalPasses,
         "-enable-experimental-passes",
         nullptr,
         "Enable experimental compiler passes"},
        {OptionKind::EnableExperimentalDynamicDispatch,
         "-enable-experimental-dynamic-dispatch",
         nullptr,
         "Enable experimental dynamic dispatch features"},
        {OptionKind::EmbedDownstreamIR,
         "-embed-downstream-ir",
         nullptr,
         "Embed downstream IR into emitted slang IR"},
        {OptionKind::ExperimentalFeature,
         "-experimental-feature",
         nullptr,
         "Enable experimental features (loading builtin neural module)"},
        {OptionKind::EnableRichDiagnostics,
         "-enable-experimental-rich-diagnostics",
         nullptr,
         "Enable experimental rich diagnostics with enhanced formatting and details"},
        {OptionKind::EnableMachineReadableDiagnostics,
         "-enable-machine-readable-diagnostics",
         nullptr,
         "Enable machine-readable diagnostic output in tab-separated format"},
        {OptionKind::DiagnosticColor,
         "-diagnostic-color",
         "-diagnostic-color <always|never|auto>",
         "Control colored diagnostic output (auto uses color if stderr is a tty)"},
    };
    _addOptions(makeConstArrayView(experimentalOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Internal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Internal");

    const Option internalOpts[] = {
        {OptionKind::ArchiveType,
         "-archive-type",
         "-archive-type <archive-type>",
         "Set the archive type for -save-core-module. Default is zip."},
        {OptionKind::CompileCoreModule,
         "-compile-core-module",
         nullptr,
         "Compile the core module from embedded sources. "
         "Will return a failure if there is already a core module available."},
        {OptionKind::Doc, "-doc", nullptr, "Write documentation for -compile-core-module"},
        {OptionKind::IrCompression,
         "-ir-compression",
         "-ir-compression <type>",
         "Set compression for IR and AST outputs.\n"
         "Accepted compression types: none, lite"},
        {OptionKind::LoadCoreModule,
         "-load-core-module",
         "-load-core-module <filename>",
         "Load the core module from file."},
        {OptionKind::ReferenceModule, "-r", "-r <name>", "reference module <name>"},
        {OptionKind::SaveCoreModule,
         "-save-core-module",
         "-save-core-module <filename>",
         "Save the core module to an archive file."},
        {OptionKind::SaveCoreModuleBinSource,
         "-save-core-module-bin-source",
         "-save-core-module-bin-source <filename>",
         "Same as -save-core-module but output "
         "the data as a C array.\n"},
        {OptionKind::SaveGLSLModuleBinSource,
         "-save-glsl-module-bin-source",
         "-save-glsl-module-bin-source <filename>",
         "Save the serialized glsl module "
         "as a C array.\n"},
        {OptionKind::TrackLiveness,
         "-track-liveness",
         nullptr,
         "Enable liveness tracking. Places SLANG_LIVE_START, and SLANG_LIVE_END in output source "
         "to indicate value liveness."},
        {OptionKind::LoopInversion,
         "-loop-inversion",
         nullptr,
         "Enable loop inversion in the code-gen optimization. Default is off"},
        {OptionKind::GenerateWholeProgram,
         "-whole-program",
         nullptr,
         "Generate code for all entry points in a single output (library mode)."},
    };
    _addOptions(makeConstArrayView(internalOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Deprecated !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Deprecated");

    const Option deprecatedOpts[] = {
        {OptionKind::ParameterBlocksUseRegisterSpaces,
         "-parameter-blocks-use-register-spaces",
         nullptr,
         "Parameter blocks will use register spaces"},
        {OptionKind::ZeroInitialize,
         "-zero-initialize",
         nullptr,
         "Initialize all variables to zero."
         "Structs will set all struct-fields without an init expression to 0."
         "All variables will call their default constructor if not explicitly initialized as "
         "usual."},
    };
    _addOptions(makeConstArrayView(deprecatedOpts), options);

    // We can now check that the whole range is available. If this fails it means there
    // is an enum in the list that hasn't been setup as an option!
    SLANG_ASSERT(options.hasContiguousUserValueRange(
        CommandOptions::LookupKind::Option,
        UserValue(0),
        UserValue(OptionKind::CountOfParsableOptions)));
    SLANG_ASSERT(options.hasContiguousUserValueRange(
        CommandOptions::LookupKind::Category,
        UserValue(0),
        UserValue(ValueCategory::CountOf)));
}

} // namespace Slang
