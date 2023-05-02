// slang-options.cpp

// Implementation of options parsing for `slangc` command line,
// and also for API interface that takes command-line argument strings.

#include "slang-options.h"

#include "../../slang.h"

#include "slang-compiler.h"
#include "slang-profile.h"

#include "../compiler-core/slang-artifact-desc-util.h"

#include "../compiler-core/slang-artifact-impl.h"
#include "../compiler-core/slang-artifact-representation-impl.h"

#include "../compiler-core/slang-name-convention-util.h"

#include "slang-repro.h"
#include "slang-serialize-ir.h"

#include "../core/slang-castable.h"
#include "../core/slang-file-system.h"
#include "../core/slang-type-text-util.h"
#include "../core/slang-hex-dump-util.h"

#include "../compiler-core/slang-command-line-args.h"
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-core-diagnostics.h"

#include "../core/slang-string-slice-pool.h"
#include "../core/slang-char-util.h"

#include "../core/slang-name-value.h"

#include "../core/slang-command-options-writer.h"

#include <assert.h>

namespace Slang {

namespace { // anonymous

// All of the options are given an unique enum
enum class OptionKind
{
    // General

    MacroDefine,
    DepFile,
    EntryPointName,
    Help,
    HelpStyle,
    Include,
    Language,
    MatrixLayoutColumn,
    MatrixLayoutRow,
    ModuleName,
    Output,
    Profile,
    Stage,
    Target,
    Version,
    WarningsAsErrors,
    DisableWarnings,
    EnableWarning,
    DisableWarning,
    DumpWarningDiagnostics,
    InputFilesRemain,
    EmitIr,

    // Target

    Capability,
    DefaultImageFormatUnknown,
    DisableDynamicDispatch,
    DisableSpecialization,
    FloatingPointMode,
    DebugInformation,
    LineDirectiveMode,
    Optimization,
    Obfuscate,
    
    // Downstream

    CompilerPath,
    DefaultDownstreamCompiler,
    DownstreamArgs,
    PassThrough,

    // Debugging

    DumpAst,
    DumpIntermediatePrefix,
    DumpIntermediates,
    DumpIr,
    DumpIrIds,
    DumpRepro,
    DumpReproOnError,
    PreprocessorOutput,
    ExtractRepro,
    LoadRepro,
    LoadReproDirectory,
    NoCodeGen,
    OutputIncludes,
    ReproFileSystem,
    SerialIr,
    SkipCodeGen,
    ValidateIr,
    VerbosePaths,
    VerifyDebugSerialIr,

    // Experimental

    EmitSpirvDirectly,
    FileSystem,
    Heterogeneous,
    NoMangle,

    // Internal

    ArchiveType,
    CompileStdLib,
    Doc,
    IrCompression,
    LoadStdLib,
    ReferenceModule,
    SaveStdLib,
    SaveStdLibBinSource,
    TrackLiveness,

    // Depreciated
    ParameterBlocksUseRegisterSpaces,

    CountOf,
};

struct Option
{
    OptionKind optionKind;
    const char* name;
    const char* usage = nullptr;
    const char* description = nullptr;
};

enum class ValueCategory
{
    Compiler,
    Target,
    Language,
    FloatingPointMode,
    ArchiveType,
    Stage,
    LineDirectiveMode,
    DebugInfoFormat,
    HelpStyle,
    OptimizationLevel,
    DebugLevel, 
    FileSystemType,

    CountOf,
};

} // anonymous

static void _addOptions(const ConstArrayView<Option>& options, CommandOptions& cmdOptions)
{
    for (auto& opt : options)
    {
        cmdOptions.add(opt.name, opt.usage, opt.description, CommandOptions::UserValue(opt.optionKind));
    }
}

void initCommandOptions(CommandOptions& options)
{
    typedef CommandOptions::Flag::Enum Flag;
    typedef CommandOptions::CategoryKind CategoryKind;
    typedef CommandOptions::UserValue UserValue;

    // Add all the option categories

    options.addCategory(CategoryKind::Option, "General", "General options");
    options.addCategory(CategoryKind::Option, "Target", "Target code generation options");
    options.addCategory(CategoryKind::Option, "Downstream", "Downstream compiler options");
    options.addCategory(CategoryKind::Option, "Debugging", "Compiler debugging/instrumentation options");
    options.addCategory(CategoryKind::Option, "Experimental", "Experimental options (use at your own risk)");
    options.addCategory(CategoryKind::Option, "Internal", "Internal-use options (use at your own risk)");
    options.addCategory(CategoryKind::Option, "Depreciated", "Deprecated options (allowed but ignored; may be removed in future)");

    // Do the easy ones
    {
        options.addCategory(CategoryKind::Value, "compiler", "Downstream Compilers (aka Pass through)", UserValue(ValueCategory::Compiler));
        options.addValues(TypeTextUtil::getCompilerInfos());
    
        options.addCategory(CategoryKind::Value, "language", "Language", UserValue(ValueCategory::Language));
        options.addValues(TypeTextUtil::getLanguageInfos());

        options.addCategory(CategoryKind::Value, "archive-type", "Archive Type", UserValue(ValueCategory::ArchiveType));
        options.addValues(TypeTextUtil::getArchiveTypeInfos());

        options.addCategory(CategoryKind::Value, "line-directive-mode", "Line Directive Mode", UserValue(ValueCategory::LineDirectiveMode));
        options.addValues(TypeTextUtil::getLineDirectiveInfos());

        options.addCategory(CategoryKind::Value, "debug-info-format", "Debug Info Format", UserValue(ValueCategory::DebugInfoFormat));
        options.addValues(TypeTextUtil::getDebugInfoFormatInfos());

        options.addCategory(CategoryKind::Value, "fp-mode", "Floating Point Mode", UserValue(ValueCategory::FloatingPointMode));
        options.addValues(TypeTextUtil::getFloatingPointModeInfos());

        options.addCategory(CategoryKind::Value, "help-style", "Help Style", UserValue(ValueCategory::HelpStyle));
        options.addValues(CommandOptionsWriter::getStyleInfos());

        options.addCategory(CategoryKind::Value, "optimization-level", "Optimization Level", UserValue(ValueCategory::OptimizationLevel));
        options.addValues(TypeTextUtil::getOptimizationLevelInfos());

        options.addCategory(CategoryKind::Value, "debug-level", "Debug Level", UserValue(ValueCategory::DebugLevel));
        options.addValues(TypeTextUtil::getDebugLevelInfos());

        options.addCategory(CategoryKind::Value, "file-system-type", "File System Type", UserValue(ValueCategory::FileSystemType));
        options.addValues(TypeTextUtil::getFileSystemTypeInfos());
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! target !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(CategoryKind::Value, "target", "Target", UserValue(ValueCategory::Target));
        for (auto opt : TypeTextUtil::getCompileTargetInfos())
        {
            options.addValue(opt.names, opt.description, UserValue(opt.target));
        }
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! stage !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(CategoryKind::Value, "stage", "Stage", UserValue(ValueCategory::Stage));
        List<NameValue> opts;
        for (auto& info: getStageInfos())
        {
            opts.add({ValueInt(info.stage), info.name });
        }
        options.addValuesWithAliases(opts.getArrayView());
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! capabilities !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    {
        options.addCategory(CategoryKind::Value, "capability", 
            "A capability describes an optional feature that a target may or "
            "may not support. When a -capability is specified, the compiler "
            "may assume that the target supports that capability, and generate "
            "code accordingly.");

        List<UnownedStringSlice> names;
        getCapabilityAtomNames(names);

        // We'll just add to keep the list more simple...
        options.addValue("spirv_1_{ 0,1,2,3,4,5 }", "minimum supported SPIR - V version");

        for (auto name : names)
        {
            if (name.startsWith("__") || 
                name.startsWith("spirv_1_"))
            {
                continue;
            }
            else if (name.startsWith("GL_"))
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
        options.addCategory(CategoryKind::Value, "file-extension",
            "A <language>, <format>, and/or <stage> may be inferred from the "
            "extension of an input or -o path");

        // TODO(JS): It's concevable that these are enumerated via some other system
        // rather than just being listed here

        const CommandOptions::ValuePair pairs[] = 
        {
            {"hlsl,fx", "hlsl"},
            {"dxbc"},
            {"dxbc-asm", "dxbc-assembly"},
            {"dxil"},
            {"dxil-asm", "dxil-assembly"},
            {"glsl"},
            {"vert", "glsl (vertex)"},
            {"frag", "glsl (fragment)"},
            {"geom", "glsl (geoemtry)"},
            {"tesc", "glsl (hull)"},
            {"tese", "glsl (domain)"},
            {"comp", "glsl (compute)"},
            {"slang"},
            {"spv", "SPIR-V"},
            {"spv-asm", "SPIR-V assembly"},
            {"c"},
            {"cpp,c++,cxx", "C++"},
            {"exe", "executable"},
            {"dll,so", "sharedlibrary/dll"},
            {"cu", "CUDA"},
            {"ptx", "PTX"},
            {"obj,o", "object-code"},
            {"zip", "container"},
            {"slang-module,slang-library", "Slang Module/Library"},
            {"dir", "Container as a directory"},
        };
        options.addValues(pairs, SLANG_COUNT_OF(pairs));
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! General !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("General");

    const Option generalOpts[] = 
    {
        { OptionKind::MacroDefine,  "-D?...",   "-D<name>[=<value>], -D <name>[=<value>]", 
        "Insert a preprocessor macro.\n" 
        "The space between - D and <name> is optional. If no <value> is specified, Slang will define the macro with an empty value." },
        { OptionKind::DepFile,      "-depfile", "-depfile <path>", "Save the source file dependency list in a file." },
        { OptionKind::EntryPointName, "-entry", "-entry <name>", 
        "Specify the name of an entry-point function.\n"
        "When compiling from a single file, this defaults to main if you specify a stage using -stage.\n"
        "Multiple -entry options may be used in a single invocation. "
        "When they do, the file associated with the entry point will be the first one found when searching to the left in the command line.\n"
        "If no -entry options are given, compiler will use [shader(...)] "
        "attributes to detect entry points."},
        { OptionKind::EmitIr,       "-emit-ir", nullptr, "Emit IR typically as a '.slang-module' when outputting to a container." },
        { OptionKind::Help,         "-h,-help,--help", "-h or -h <help-category>", "Print this message, or help in specified category." },
        { OptionKind::HelpStyle,    "-help-style", "-help-style <help-style>", "Help formatting style" },
        { OptionKind::Include,      "-I?...", "-I<path>, -I <path>", 
        "Add a path to be used in resolving '#include' "
        "and 'import' operations."},
        { OptionKind::Language,     "-lang", "-lang <language>", "Set the language for the following input files."},
        { OptionKind::MatrixLayoutColumn, "-matrix-layout-column-major", nullptr, "Set the default matrix layout to column-major."},
        { OptionKind::MatrixLayoutRow,"-matrix-layout-row-major", nullptr, "Set the default matrix layout to row-major."},
        { OptionKind::ModuleName,     "-module-name", "-module-name <name>", 
        "Set the module name to use when compiling multiple .slang source files into a single module."},
        { OptionKind::Output, "-o", "-o <path>", 
        "Specify a path where generated output should be written.\n"
        "If no -target or -stage is specified, one may be inferred "
        "from file extension (see <file-extension>). "
        "If multiple -target options and a single -entry are present, each -o "
        "associates with the first -target to its left. "
        "Otherwise, if multiple -entry options are present, each -o associates "
        "with the first -entry to its left, and with the -target that matches "
        "the one inferred from <path>."},
        { OptionKind::Profile, "-profile", "-profile <profile>[+<capability>...]",
        "Specify the shader profile for code generation.\n"
        "Accepted profiles are:\n"
        "* sm_{4_0,4_1,5_0,5_1,6_0,6_1,6_2,6_3,6_4,6_5,6_6}\n"
        "* glsl_{110,120,130,140,150,330,400,410,420,430,440,450,460}\n"
        "Additional profiles that include -stage information:\n"
        "* {vs,hs,ds,gs,ps}_<version>\n"
        "See -capability for information on <capability>\n"
        "When multiple -target options are present, each -profile associates "
        "with the first -target to its left."},
        { OptionKind::Stage, "-stage", "-stage <stage>",
        "Specify the stage of an entry-point function.\n"
        "When multiple -entry options are present, each -stage associated with "
        "the first -entry to its left.\n"
        "May be omitted if entry-point function has a [shader(...)] attribute; "
        "otherwise required for each -entry option."},
        { OptionKind::Target, "-target", "-target <target>", "Specifies the format in which code should be generated."},
        { OptionKind::Version, "-v,-version", nullptr, 
            "Display the build version. This is the contents of git describe --tags.\n"
            "It is typically only set from automated builds(such as distros available on github).A user build will by default be 'unknown'."},
        { OptionKind::WarningsAsErrors, "-warnings-as-errors", "-warnings-as-errors all or -warnings-as-errors <id>[,<id>...]", 
        "all - Treat all warnings as errors.\n"
        "<id>[,<id>...]: Treat specific warning ids as errors.\n"},
        { OptionKind::DisableWarnings, "-warnings-disable", "-warnings-disable <id>[,<id>...]", "Disable specific warning ids."},
        { OptionKind::EnableWarning, "-W...", "-W<id>", "Enable a warning with the specified id."},
        { OptionKind::DisableWarning, "-Wno-...", "-Wno-<id>", "Disable warning with <id>"},
        { OptionKind::DumpWarningDiagnostics, "-dump-warning-diagnostics", nullptr, "Dump to output list of warning diagnostic numeric and name ids." },
        { OptionKind::InputFilesRemain, "--", nullptr, "Treat the rest of the command line as input files."}
    };

    _addOptions(makeConstArrayView(generalOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Target !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Target");

    const Option targetOpts[] = 
    {
        { OptionKind::Capability, "-capability", "-capability <capability>[+<capability>...]",
        "Add optional capabilities to a code generation target. See Capabilities below."},
        { OptionKind::DefaultImageFormatUnknown, "-default-image-format-unknown", nullptr,
        "Set the format of R/W images with unspecified format to 'unknown'. Otherwise try to guess the format."},
        { OptionKind::DisableDynamicDispatch, "-disable-dynamic-dispatch", nullptr, "Disables generating dynamic dispatch code." },
        { OptionKind::DisableSpecialization, "-disable-specialization", nullptr, "Disables generics and specialization pass." },
        { OptionKind::FloatingPointMode, "-fp-mode,-floating-point-mode", "-fp-mode <fp-mode>, -floating-point-mode <fp-mode>", 
        "Control floating point optimizations"},
        { OptionKind::DebugInformation, "-g...", "-g, -g<debug-info-format>, -g<debug-level>", 
        "Include debug information in the generated code, where possible.\n"
        "<debug-level> is the amount of information, 0..3, unspecified means 2\n" 
        "<debug-info-format> specifies a debugging info format\n"
        "It is valid to have multiple -g options, such as a <debug-level> and a <debug-info-format>" },
        { OptionKind::LineDirectiveMode, "-line-directive-mode", "-line-directive-mode <line-directive-mode>", 
        "Sets how the `#line` directives should be produced. Available options are:\n"
        "If not specified, default behavior is to use C-style `#line` directives "
        "for HLSL and C/C++ output, and traditional GLSL-style `#line` directives "
        "for GLSL output." },
        { OptionKind::Optimization, "-O...", "-O<optimization-level>", "Set the optimization level."},
        { OptionKind::Obfuscate, "-obfuscate", nullptr, "Remove all source file information from outputs." },
    };

    _addOptions(makeConstArrayView(targetOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Downstream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Downstream");

    {
        auto namesList = NameValueUtil::getNames(NameValueUtil::NameKind::First, TypeTextUtil::getCompilerInfos());
        StringBuilder names;
        for (auto name : namesList)
        {
            names << "-" << name << "-path,";
        }
        // remove last ,
        names.reduceLength(names.getLength() - 1);

        options.add(names.getBuffer(), "-<compiler>-path <path>", 
            "Specify path to a downstream <compiler> "
            "executable or library.\n", 
            UserValue(OptionKind::CompilerPath));
    }

    const Option downstreamOpts[] = 
    {
        { OptionKind::DefaultDownstreamCompiler, "-default-downstream-compiler", "-default-downstream-compiler <language> <compiler>",
        "Set a default compiler for the given language. See -lang for the list of languages." },
        { OptionKind::DownstreamArgs, "-X...", "-X<compiler> <option> -X<compiler>... <options> -X.", 
        "Pass arguments to downstream <compiler>. Just -X<compiler> passes just the next argument "
        "to the downstream compiler. -X<compiler>... options -X. will pass *all* of the options "
        "inbetween the opening -X and -X. to the downstream compiler."},
        { OptionKind::PassThrough, "-pass-through", "-pass-through <compiler>", 
        "Pass the input through mostly unmodified to the "
        "existing compiler <compiler>.\n" 
        "These are intended for debugging/testing purposes, when you want to be able to see what these existing compilers do with the \"same\" input and options"},
    };

    _addOptions(makeConstArrayView(downstreamOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Debugging !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Debugging");

    const Option debuggingOpts[] = 
    {
        { OptionKind::DumpAst, "-dump-ast", nullptr, "Dump the AST to a .slang-ast file next to the input." },
        { OptionKind::DumpIntermediatePrefix, "-dump-intermediate-prefix", "-dump-intermediate-prefix <prefix>", 
        "File name prefix for -dump-intermediates outputs, default is 'slang-dump-'"},
        { OptionKind::DumpIntermediates, "-dump-intermediates", nullptr, "Dump intermediate outputs for debugging." },
        { OptionKind::DumpIr, "-dump-ir", nullptr, "Dump the IR for debugging." },
        { OptionKind::DumpIrIds, "-dump-ir-ids", nullptr, "Dump the IDs with -dump-ir (debug builds only)" },
        { OptionKind::DumpRepro, "-dump-repro", nullptr, "Dump a `.slang-repro` file that can be used to reproduce "
        "a compilation on another machine.\n"},
        { OptionKind::DumpReproOnError, "-dump-repro-on-error", nullptr, "Dump `.slang-repro` file on any compilation error." },
        { OptionKind::PreprocessorOutput, "-E,-output-preprocessor", nullptr, "Output the preprocessing result and exit." },
        { OptionKind::ExtractRepro, "-extract-repro", "-extract-repro <name>", "Extract the repro files into a folder." },
        { OptionKind::LoadReproDirectory, "-load-repro-directory", "-load-repro-directory <path>", "Use repro along specified path" },
        { OptionKind::LoadRepro, "-load-repro", "-load-repro <name>", "Load repro"},
        { OptionKind::NoCodeGen, "-no-codegen", nullptr, "Skip the code generation step, just check the code and generate layout." },
        { OptionKind::OutputIncludes, "-output-includes", nullptr, "Print the hierarchy of the processed source files." },
        { OptionKind::ReproFileSystem, "-repro-file-system", "-repro-file-system <name>", "Use a repro as a file system" },
        { OptionKind::SerialIr, "-serial-ir", nullptr, "Serialize the IR between front-end and back-end." },
        { OptionKind::SkipCodeGen, "-skip-codegen", nullptr, "Skip the code generation phase." },
        { OptionKind::ValidateIr, "-validate-ir", nullptr, "Validate the IR between the phases." },
        { OptionKind::VerbosePaths, "-verbose-paths", nullptr, "When displaying diagnostic output aim to display more detailed path information. "
        "In practice this is typically the complete 'canonical' path to the source file used." },
        { OptionKind::VerifyDebugSerialIr, "-verify-debug-serial-ir", nullptr, "Verify IR in the front-end." }
    };
    _addOptions(makeConstArrayView(debuggingOpts), options);
    
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Experimental !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Experimental");

    const Option experimentalOpts[] = 
    {
        { OptionKind::EmitSpirvDirectly, "-emit-spirv-directly", nullptr, 
        "Generate SPIR-V output directly (otherwise through "
        "GLSL and using the glslang compiler)"},
        { OptionKind::FileSystem, "-file-system", "-file-system <file-system-type>", 
        "Set the filesystem hook to use for a compile request."},
        { OptionKind::Heterogeneous, "-heterogeneous", nullptr, "Output heterogeneity-related code." },
        { OptionKind::NoMangle, "-no-mangle", nullptr, "Do as little mangling of names as possible." }
    };
    _addOptions(makeConstArrayView(experimentalOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Internal !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Internal");

    const Option internalOpts[] = 
    {
        { OptionKind::ArchiveType, "-archive-type", "-archive-type <archive-type>", "Set the archive type for -save-stdlib. Default is zip." },
        { OptionKind::CompileStdLib, "-compile-stdlib", nullptr, 
        "Compile the StdLib from embedded sources. "
        "Will return a failure if there is already a StdLib available."},
        { OptionKind::Doc, "-doc", nullptr, "Write documentation for -compile-stdlib" },
        { OptionKind::IrCompression,"-ir-compression", "-ir-compression <type>", 
        "Set compression for IR and AST outputs.\n"
        "Accepted compression types: none, lite"},
        { OptionKind::LoadStdLib, "-load-stdlib", "-load-stdlib <filename>", "Load the StdLib from file." },
        { OptionKind::ReferenceModule, "-r", "-r <name>", "reference module <name>" },
        { OptionKind::SaveStdLib, "-save-stdlib", "-save-stdlib <filename>", "Save the StdLib modules to an archive file." },
        { OptionKind::SaveStdLibBinSource, "-save-stdlib-bin-source","-save-stdlib-bin-source <filename>", "Same as -save-stdlib but output "
        "the data as a C array.\n"},
        { OptionKind::TrackLiveness, "-track-liveness", nullptr, "Enable liveness tracking. Places SLANG_LIVE_START, and SLANG_LIVE_END in output source to indicate value liveness." },
    };
    _addOptions(makeConstArrayView(internalOpts), options);

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Depreciated !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

    options.setCategory("Depreciated");

    const Option depreciatedOpts[] = 
    {
        { OptionKind::ParameterBlocksUseRegisterSpaces, "-parameter-blocks-use-register-spaces", nullptr, "Parameter blocks will use register spaces" },
    };
    _addOptions(makeConstArrayView(depreciatedOpts), options);

    // We can now check that the whole range is available. If this fails it means there 
    // is an enum in the list that hasn't been setup as an option!
    SLANG_ASSERT(options.hasContiguousUserValueRange(CommandOptions::LookupKind::Option, UserValue(0), UserValue(OptionKind::CountOf)));
    SLANG_ASSERT(options.hasContiguousUserValueRange(CommandOptions::LookupKind::Category, UserValue(0), UserValue(ValueCategory::CountOf)));
}

SlangResult _addLibraryReference(EndToEndCompileRequest* req, IArtifact* artifact);

struct OptionsParser
{
    SlangSession*           session = nullptr;
    SlangCompileRequest*    compileRequest = nullptr;

    Slang::EndToEndCompileRequest*  requestImpl = nullptr;

    // A "translation unit" represents one or more source files
    // that are processed as a single entity when it comes to
    // semantic checking.
    //
    // For languages like HLSL, GLSL, and C, a translation unit
    // is usually a single source file (which can then go on
    // to `#include` other files into the same translation unit).
    //
    // For Slang, we support having multiple source files in
    // a single translation unit, and indeed command-line `slangc`
    // will always put all the source files into a single translation
    // unit.
    //
    // We track information on the translation units that we
    // create during options parsing, so that we can assocaite
    // other entities with these translation units:
    //
    struct RawTranslationUnit
    {
        // What language is the translation unit using?
        //
        // Note: We do not support translation units that mix
        // languages.
        //
        SlangSourceLanguage sourceLanguage;

        // Certain naming conventions imply a stage for
        // a file with only a single entry point, and in
        // those cases we will try to infer the stage from
        // the file when it is possible.
        //
        Stage impliedStage;

        // We retain the Slang API level translation unit index,
        // which we will call an "ID" inside the options parsing code.
        //
        // This will almost always be the index into the
        // `rawTranslationUnits` array below, but could conceivably,
        // be mismatched if we were parsing options for a compile
        // request that already had some translation unit(s) added
        // manually.
        //
        int                 translationUnitID;
    };
    List<RawTranslationUnit> rawTranslationUnits;

    // If we already have a translation unit for Slang code, then this will give its index.
    // If not, it will be `-1`.
    int slangTranslationUnitIndex = -1;

    // The number of input files that have been specified
    int inputPathCount = 0;

    int translationUnitCount = 0;
    int currentTranslationUnitIndex= -1;

    // An entry point represents a function to be checked and possibly have
    // code generated in one of our translation units. An entry point
    // needs to have an associated stage, which might come via the
    // `-stage` command line option, or a `[shader("...")]` attribute
    // in the source code.
    //
    struct RawEntryPoint
    {
        String  name;
        Stage   stage = Stage::Unknown;
        int     translationUnitIndex = -1;
        int     entryPointID = -1;

        // State for tracking command-line errors
        bool conflictingStagesSet = false;
        bool redundantStageSet = false;
    };
    //
    // We collect the entry points in a "raw" array so that we can
    // possibly associate them with a stage or translation unit
    // after the fact.
    //
    List<RawEntryPoint> rawEntryPoints;

    // In the case where we have only a single entry point,
    // the entry point and its options might be specified out
    // of order, so we will keep a single `RawEntryPoint` around
    // and use it as the target for any state-setting options
    // before the first "proper" entry point is specified.
    RawEntryPoint defaultEntryPoint;

    SlangCompileFlags flags = 0;

    struct RawOutput
    {
        String          path;
        CodeGenTarget   impliedFormat = CodeGenTarget::Unknown;
        int             targetIndex = -1;
        int             entryPointIndex = -1;
        bool            isWholeProgram = false;
    };
    List<RawOutput> rawOutputs;

    struct RawTarget
    {
        CodeGenTarget       format = CodeGenTarget::Unknown;
        ProfileVersion      profileVersion = ProfileVersion::Unknown;
        SlangTargetFlags    targetFlags = 0;
        int                 targetID = -1;
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;

        List<CapabilityAtom> capabilityAtoms;

        // State for tracking command-line errors
        bool conflictingProfilesSet = false;
        bool redundantProfileSet = false;

    };
    List<RawTarget> rawTargets;

    RawTarget defaultTarget;

    int addTranslationUnit(
        SlangSourceLanguage language,
        Stage               impliedStage)
    {
        auto translationUnitIndex = rawTranslationUnits.getCount();
        auto translationUnitID = compileRequest->addTranslationUnit(language, nullptr);

        // As a sanity check: the API should be returning the same translation
        // unit index as we maintain internally. This invariant would only
        // be broken if we decide to support a mix of translation units specified
        // via API, and ones specified via command-line arguments.
        //
        SLANG_RELEASE_ASSERT(Index(translationUnitID) == translationUnitIndex);

        RawTranslationUnit rawTranslationUnit;
        rawTranslationUnit.sourceLanguage = language;
        rawTranslationUnit.translationUnitID = translationUnitID;
        rawTranslationUnit.impliedStage = impliedStage;

        rawTranslationUnits.add(rawTranslationUnit);

        return int(translationUnitIndex);
    }

    void addInputSlangPath(
        String const& path)
    {
        // All of the input .slang files will be grouped into a single logical translation unit,
        // which we create lazily when the first .slang file is encountered.
        if( slangTranslationUnitIndex == -1 )
        {
            translationUnitCount++;
            slangTranslationUnitIndex = addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, Stage::Unknown);
        }

        compileRequest->addTranslationUnitSourceFile(rawTranslationUnits[slangTranslationUnitIndex].translationUnitID, path.begin());

        // Set the translation unit to be used by subsequent entry points
        currentTranslationUnitIndex = slangTranslationUnitIndex;
    }

    void addInputForeignShaderPath(
        String const&           path,
        SlangSourceLanguage     language,
        Stage                   impliedStage)
    {
        translationUnitCount++;
        currentTranslationUnitIndex = addTranslationUnit(language, impliedStage);

        compileRequest->addTranslationUnitSourceFile(rawTranslationUnits[currentTranslationUnitIndex].translationUnitID, path.begin());
    }

    static Profile::RawVal findGlslProfileFromPath(const String& path)
    {
        struct Entry
        {
            const char* ext;
            Profile::RawVal profileId;
        };

        static const Entry entries[] = 
        {
            { ".frag", Profile::GLSL_Fragment },
            { ".geom", Profile::GLSL_Geometry },
            { ".tesc", Profile::GLSL_TessControl },
            { ".tese", Profile::GLSL_TessEval },
            { ".comp", Profile::GLSL_Compute } 
        };

        for (Index i = 0; i < SLANG_COUNT_OF(entries); ++i)
        {
            const Entry& entry = entries[i];
            if (path.endsWith(entry.ext))
            {
                return entry.profileId;
            }
        }
        return Profile::Unknown;
    }

    static SlangSourceLanguage findSourceLanguageFromPath(const String& path, Stage& outImpliedStage)
    {
        struct Entry
        {
            const char*         ext;
            SlangSourceLanguage sourceLanguage;
            SlangStage          impliedStage;
        };

        static const Entry entries[] = 
        {
            { ".slang", SLANG_SOURCE_LANGUAGE_SLANG, SLANG_STAGE_NONE },

            { ".hlsl",  SLANG_SOURCE_LANGUAGE_HLSL, SLANG_STAGE_NONE },
            { ".fx",    SLANG_SOURCE_LANGUAGE_HLSL, SLANG_STAGE_NONE },

            { ".glsl", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_NONE },
            { ".vert", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_VERTEX },
            { ".frag", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_FRAGMENT },
            { ".geom", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_GEOMETRY },
            { ".tesc", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_HULL },
            { ".tese", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_DOMAIN },
            { ".comp", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_COMPUTE },
            { ".mesh", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_MESH },
            { ".task", SLANG_SOURCE_LANGUAGE_GLSL,  SLANG_STAGE_AMPLIFICATION },

            { ".c",    SLANG_SOURCE_LANGUAGE_C,     SLANG_STAGE_NONE },
            { ".cpp",  SLANG_SOURCE_LANGUAGE_CPP,   SLANG_STAGE_NONE },
            { ".cu",   SLANG_SOURCE_LANGUAGE_CUDA,  SLANG_STAGE_NONE }

        };

        for (Index i = 0; i < SLANG_COUNT_OF(entries); ++i)
        {
            const Entry& entry = entries[i];
            if (path.endsWith(entry.ext))
            {
                outImpliedStage = Stage(entry.impliedStage);
                return entry.sourceLanguage;
            }
        }
        return SLANG_SOURCE_LANGUAGE_UNKNOWN;
    }

    SlangResult addInputPath(
        char const*  inPath,
        SourceLanguage langOverride = SourceLanguage::Unknown)
    {
        inputPathCount++;

        // look at the extension on the file name to determine
        // how we should handle it.
        String path = String(inPath);

        if( path.endsWith(".slang") || langOverride == SourceLanguage::Slang)
        {
            // Plain old slang code
            addInputSlangPath(path);
            return SLANG_OK;
        }

        Stage impliedStage = Stage::Unknown;
        SlangSourceLanguage sourceLanguage = langOverride == SourceLanguage::Unknown ? findSourceLanguageFromPath(path, impliedStage) : SlangSourceLanguage(langOverride);

        if (sourceLanguage == SLANG_SOURCE_LANGUAGE_UNKNOWN)
        {
            requestImpl->getSink()->diagnose(SourceLoc(), Diagnostics::cannotDeduceSourceLanguage, inPath);
            return SLANG_FAIL;
        }

        addInputForeignShaderPath(path, sourceLanguage, impliedStage);

        return SLANG_OK;
    }

    void addOutputPath(
        String const&   path,
        CodeGenTarget   impliedFormat)
    {
        RawOutput rawOutput;
        rawOutput.path = path;
        rawOutput.impliedFormat = impliedFormat;
        rawOutputs.add(rawOutput);
    }

    void addOutputPath(char const* inPath)
    {
        String path = String(inPath);
        String ext = Path::getPathExt(path);

        if (ext == toSlice("slang-module") || 
            ext == toSlice("slang-lib") || 
            ext == toSlice("dir") || 
            ext == toSlice("zip"))
        {
            // These extensions don't indicate a artifact container, just that we want to emit IR
            if (ext == toSlice("slang-module") ||
                ext == toSlice("slang-lib"))
            {
                // We want to emit IR 
                requestImpl->m_emitIr = true;
            }
            else
            {
                // We want to write out in an artfact "container", that can hold multiple artifacts.
                compileRequest->setOutputContainerFormat(SLANG_CONTAINER_FORMAT_SLANG_MODULE);
            }

            requestImpl->m_containerOutputPath = path;
        }
        else
        {
            const SlangCompileTarget target = TypeTextUtil::findCompileTargetFromExtension(ext.getUnownedSlice());
            // If the target is not found the value returned is Unknown. This is okay because
            // we allow an unknown-format `-o`, assuming we get a target format
            // from another argument.
            addOutputPath(path, CodeGenTarget(target));
        }
    }

    RawEntryPoint* getCurrentEntryPoint()
    {
        auto rawEntryPointCount = rawEntryPoints.getCount();
        return rawEntryPointCount ? &rawEntryPoints[rawEntryPointCount-1] : &defaultEntryPoint;
    }

    void setStage(RawEntryPoint* rawEntryPoint, Stage stage)
    {
        if(rawEntryPoint->stage != Stage::Unknown)
        {
            rawEntryPoint->redundantStageSet = true;
            if( stage != rawEntryPoint->stage )
            {
                rawEntryPoint->conflictingStagesSet = true;
            }
        }
        rawEntryPoint->stage = stage;
    }

    RawTarget* getCurrentTarget()
    {
        auto rawTargetCount = rawTargets.getCount();
        return rawTargetCount ? &rawTargets[rawTargetCount-1] : &defaultTarget;
    }

    void setProfileVersion(RawTarget* rawTarget, ProfileVersion profileVersion)
    {
        if(rawTarget->profileVersion != ProfileVersion::Unknown)
        {
            rawTarget->redundantProfileSet = true;

            if(profileVersion != rawTarget->profileVersion)
            {
                rawTarget->conflictingProfilesSet = true;
            }
        }
        rawTarget->profileVersion = profileVersion;
    }

    void addCapabilityAtom(RawTarget* rawTarget, CapabilityAtom atom)
    {
        rawTarget->capabilityAtoms.add(atom);
    }

    void setFloatingPointMode(RawTarget* rawTarget, FloatingPointMode mode)
    {
        rawTarget->floatingPointMode = mode;
    }

    static bool _passThroughRequiresStage(PassThroughMode passThrough)
    {
        switch (passThrough)
        {
            case PassThroughMode::Glslang:
            case PassThroughMode::Dxc:
            case PassThroughMode::Fxc:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }

    class ReproPathVisitor : public Slang::Path::Visitor
    {
    public:
        virtual void accept(Slang::Path::Type type, const Slang::UnownedStringSlice& filename) SLANG_OVERRIDE
        {
            if (type == Path::Type::File && Path::getPathExt(filename) == "slang-repro")
            {
                m_filenames.add(filename);
            }
        }

        Slang::List<String> m_filenames;
    };

    static SlangResult _compileReproDirectory(SlangSession* session, EndToEndCompileRequest* originalRequest, const String& dir, DiagnosticSink* sink)
    {
        auto stdOut = originalRequest->getWriter(WriterChannel::StdOutput);

        ReproPathVisitor visitor;
        Path::find(dir, nullptr, &visitor);

        for (auto filename : visitor.m_filenames)
        {
            auto path = Path::combine(dir, filename);

            ComPtr<slang::ICompileRequest> request;
            SLANG_RETURN_ON_FAIL(session->createCompileRequest(request.writeRef()));

            auto requestImpl = asInternal(request);

            List<uint8_t> buffer;
            SLANG_RETURN_ON_FAIL(ReproUtil::loadState(path, sink, buffer));

            auto requestState = ReproUtil::getRequest(buffer);
            MemoryOffsetBase base;
            base.set(buffer.getBuffer(), buffer.getCount());

            // If we can find a directory, that exists, we will set up a file system to load from that directory
            ComPtr<ISlangFileSystem> fileSystem;
            String dirPath;
            if (SLANG_SUCCEEDED(ReproUtil::calcDirectoryPathFromFilename(path, dirPath)))
            {
                SlangPathType pathType;
                if (SLANG_SUCCEEDED(Path::getPathType(dirPath, &pathType)) && pathType == SLANG_PATH_TYPE_DIRECTORY)
                {
                    fileSystem = new RelativeFileSystem(OSFileSystem::getExtSingleton(), dirPath);
                }
            }

            SLANG_RETURN_ON_FAIL(ReproUtil::load(base, requestState, fileSystem, requestImpl));

            if (stdOut)
            {
                StringBuilder buf;
                buf << filename << "\n";
                stdOut->write(buf.getBuffer(), buf.getLength());
            }

            StringBuilder bufs[Index(WriterChannel::CountOf)];
            ComPtr<ISlangWriter> writers[Index(WriterChannel::CountOf)];
            for (Index i = 0; i < Index(WriterChannel::CountOf); ++i)
            {
                writers[i] = new StringWriter(&bufs[0], 0);
                requestImpl->setWriter(WriterChannel(i), writers[i]);
            }

            if (SLANG_FAILED(requestImpl->compile()))
            {
                const char failed[] = "FAILED!\n";
                stdOut->write(failed, SLANG_COUNT_OF(failed) - 1);

                const auto& diagnostics = bufs[Index(WriterChannel::Diagnostic)];

                stdOut->write(diagnostics.getBuffer(), diagnostics.getLength());

                return SLANG_FAIL;
            }
        }

        if (stdOut)
        {
            const char end[] = "(END)\n";
            stdOut->write(end, SLANG_COUNT_OF(end) - 1);
        }

        return SLANG_OK;
    }

    // Pass Severity::Disabled to allow any original severity
    SlangResult _overrideDiagnostics(const UnownedStringSlice& identifierList, Severity originalSeverity, Severity overrideSeverity, DiagnosticSink* sink)
    {
        List<UnownedStringSlice> slices;
        StringUtil::split(identifierList, ',', slices);
        
        for (const auto& slice : slices)
        {
            SLANG_RETURN_ON_FAIL(_overrideDiagnostic(slice, originalSeverity, overrideSeverity, sink));
        }
        return SLANG_OK;
    }

    // Pass Severity::Disabled to allow any original severity
    SlangResult _overrideDiagnostic(const UnownedStringSlice& identifier, Severity originalSeverity, Severity overrideSeverity, DiagnosticSink* sink)
    {
        auto diagnosticsLookup = getDiagnosticsLookup();

        const DiagnosticInfo* diagnostic = nullptr;
        Int diagnosticId = -1;

        // If it starts with a digit we assume it a number 
        if (identifier.getLength() > 0 && (CharUtil::isDigit(identifier[0]) || identifier[0] == '-'))
        {
            if (SLANG_FAILED(StringUtil::parseInt(identifier, diagnosticId)))
            {
                sink->diagnose(SourceLoc(), Diagnostics::unknownDiagnosticName, identifier);
                return SLANG_FAIL;
            }

            // If we use numbers, we don't worry if we can't find a diagnostic
            // and silently ignore. This was the previous behavior, and perhaps
            // provides a way to safely disable warnings, without worrying about
            // the version of the compiler.
            diagnostic = diagnosticsLookup->getDiagnosticById(diagnosticId);
        }
        else
        {
            diagnostic = diagnosticsLookup->findDiagnosticByName(identifier);
            if (!diagnostic)
            {
                sink->diagnose(SourceLoc(), Diagnostics::unknownDiagnosticName, identifier);
                return SLANG_FAIL;
            }
            diagnosticId = diagnostic->id;
        }

        // If we are only allowing certain original severities check it's the right type
        if (diagnostic && originalSeverity != Severity::Disable && diagnostic->severity != originalSeverity)
        {
            // Strictly speaking the diagnostic name is known, but it's not the right severity
            // to be converted from, so it is an 'unknown name' in the context of severity...
            // Or perhaps we want another diagnostic
            sink->diagnose(SourceLoc(), Diagnostics::unknownDiagnosticName, identifier);
            return SLANG_FAIL;
        }

        // Override the diagnostic severity in the sink
        requestImpl->getSink()->overrideDiagnosticSeverity(int(diagnosticId), overrideSeverity, diagnostic);

        return SLANG_OK;
    }

    SlangResult _dumpDiagnostics(Severity originalSeverity, DiagnosticSink* sink)
    {
        // Get the diagnostics and dump them
        auto diagnosticsLookup = getDiagnosticsLookup();

        StringBuilder buf;

        for (const auto& diagnostic : diagnosticsLookup->getDiagnostics())
        {
            if (originalSeverity != Severity::Disable &&
                diagnostic->severity != originalSeverity)
            {
                continue;
            }

            buf.clear();

            buf << diagnostic->id << " : ";
            NameConventionUtil::convert(NameStyle::Camel, UnownedStringSlice(diagnostic->name), NameConvention::LowerKabab, buf);
            buf << "\n";
            sink->diagnoseRaw(Severity::Note, buf.getUnownedSlice());
        }

        return SLANG_OK;
    }

    SlangResult _getValue(ValueCategory valueCategory, const CommandLineArg& arg, const UnownedStringSlice& name, DiagnosticSink* sink, CommandOptions::UserValue& outValue)
    {
        auto& cmdOptions = asInternal(session)->m_commandOptions;

        const auto optionIndex = cmdOptions.findOptionByCategoryUserValue(CommandOptions::UserValue(valueCategory), name);
        if (optionIndex < 0)
        {                
            const auto categoryIndex = cmdOptions.findCategoryByUserValue(CommandOptions::UserValue(valueCategory));
            SLANG_ASSERT(categoryIndex >= 0);
            if (categoryIndex < 0)
            {
                return SLANG_FAIL;
            }

            List<UnownedStringSlice> names;
            cmdOptions.getCategoryOptionNames(categoryIndex, names);

            StringBuilder buf;
            StringUtil::join(names.getBuffer(), names.getCount(), toSlice(", "), buf);

            sink->diagnose(arg.loc, Diagnostics::unknownCommandLineValue, buf);
            return SLANG_FAIL;
        }

        outValue = cmdOptions.getOptionAt(optionIndex).userValue;
        return SLANG_OK;
    }

    SlangResult _getValue(ValueCategory valueCategory, const CommandLineArg& arg, DiagnosticSink* sink, CommandOptions::UserValue& outValue)
    {
        return _getValue(valueCategory, arg, arg.value.getUnownedSlice(), sink, outValue);
    }

    SlangResult _getValue(const ConstArrayView<ValueCategory>& valueCategories, const CommandLineArg& arg, const UnownedStringSlice& name, DiagnosticSink* sink, ValueCategory& outCat, CommandOptions::UserValue& outValue)
    {
        auto& cmdOptions = asInternal(session)->m_commandOptions;

        for (auto valueCategory : valueCategories)
        {
            const auto optionIndex = cmdOptions.findOptionByCategoryUserValue(CommandOptions::UserValue(valueCategory), name);
            if (optionIndex >= 0)
            {
                outCat = valueCategory;
                outValue = cmdOptions.getOptionAt(optionIndex).userValue;
                return SLANG_OK;
            }
        }

        List<UnownedStringSlice> names;
        for (auto valueCategory : valueCategories)
        { 
            const auto categoryIndex = cmdOptions.findCategoryByUserValue(CommandOptions::UserValue(valueCategory));
            SLANG_ASSERT(categoryIndex >= 0);
            if (categoryIndex < 0)
            {
                return SLANG_FAIL;
            }
            cmdOptions.appendCategoryOptionNames(categoryIndex, names);
        }

        StringBuilder buf;
        StringUtil::join(names.getBuffer(), names.getCount(), toSlice(", "), buf);

        sink->diagnose(arg.loc, Diagnostics::unknownCommandLineValue, buf);
        return SLANG_FAIL;
    }

    SlangResult _expectValue(ValueCategory valueCategory, CommandLineReader& reader, DiagnosticSink* sink, CommandOptions::UserValue& outValue)
    {
        CommandLineArg arg;
        SLANG_RETURN_ON_FAIL(reader.expectArg(arg));
        SLANG_RETURN_ON_FAIL(_getValue(valueCategory, arg, sink, outValue));
        return SLANG_OK;
    }

    void _appendUsageTitle(StringBuilder& out)
    {
        out << "Usage: slangc [options...] [--] <input files>\n\n";
    }
    void _appendMinimalUsage(StringBuilder& out)
    {
        _appendUsageTitle(out);
        out << "For help: slangc -h\n";
    }
    void _outputMinimalUsage(DiagnosticSink* sink)
    {
        // Output usage info
        StringBuilder buf;
        _appendMinimalUsage(buf);

        sink->diagnoseRaw(Severity::Note, buf.getUnownedSlice());
    }

    SlangResult parse(
        int             argc,
        char const* const*  argv)
    {
       
        // Copy some state out of the current request, in case we've been called
        // after some other initialization has been performed.
        flags = requestImpl->getFrontEndReq()->compileFlags;

        DiagnosticSink* requestSink = requestImpl->getSink();

        CommandLineContext* cmdLineContext = requestImpl->getLinkage()->m_downstreamArgs.getContext();
        
        // Why create a new DiagnosticSink?
        // We *don't* want the lexer that comes as default (it's for Slang source!)
        // We may want to set flags that are different
        // We will need to use a new sourceManager that will just last for this parse and will map locs to
        // source lines.
        //
        // The *problem* is that we still need to communicate to the requestSink in some suitable way.
        //
        // 1) We could have some kind of scoping mechanism (and only one sink)
        // 2) We could have a 'parent' diagnostic sink, that if we set we route output too
        // 3) We use something like the ISlangWriter to always be the thing output too (this has problems because
        // some code assumes the diagnostics are accessible as a string)
        //
        // The solution used here is to have DiagnosticsSink have a 'parent' that also gets diagnostics reported to.
      
        DiagnosticSink parseSink(cmdLineContext->getSourceManager(), nullptr);
        
        {
            parseSink.setFlags(requestSink->getFlags());
            // Allow HumaneLoc - it won't display much for command line parsing - just (1):
            // Leaving allows for diagnostics to be compatible with other Slang diagnostic parsing.
            //parseSink.resetFlag(DiagnosticSink::Flag::HumaneLoc);
            parseSink.setFlag(DiagnosticSink::Flag::SourceLocationLine);
        }

        // All diagnostics will also be sent to requestSink
        parseSink.setParentSink(requestSink);

        DiagnosticSink* sink = &parseSink;

        // Set up the args
        CommandLineArgs args(cmdLineContext);
        // Converts input args into args in 'args'.
        // Doing so will allocate some SourceLoc space from the CommandLineContext.
        args.setArgs(argv, argc);

        {
            auto linkage = requestImpl->getLinkage();
            // Before we do anything else lets strip out all of the downstream arguments.
            SLANG_RETURN_ON_FAIL(linkage->m_downstreamArgs.stripDownstreamArgs(args, 0, sink));
        }

        CommandLineReader reader(&args, sink);

        SlangMatrixLayoutMode defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

        // The default archive type is zip
        SlangArchiveType archiveType = SLANG_ARCHIVE_TYPE_ZIP;

        bool compileStdLib = false;
        slang::CompileStdLibFlags compileStdLibFlags = 0;
        bool hasLoadedRepro = false;

        // Get the options on the session
        CommandOptions& options = asInternal(session)->m_commandOptions;
        CommandOptionsWriter::Style helpStyle = CommandOptionsWriter::Style::Text; 

        auto frontEndReq = requestImpl->getFrontEndReq();

        while (reader.hasArg())
        {
            auto arg = reader.getArgAndAdvance();
            const auto& argValue = arg.value;

            // If it's not an option we assume it's a path
            if (argValue[0] != '-')
            {
                SLANG_RETURN_ON_FAIL(addInputPath(argValue.getBuffer()));
                continue;
            }

            const Index optionIndex = options.findOptionByName(argValue.getUnownedSlice());

            if (optionIndex < 0)
            {
                sink->diagnose(arg.loc, Diagnostics::unknownCommandLineOption, argValue);
                _outputMinimalUsage(sink);
                return SLANG_FAIL;
            }

            const auto optionKind = OptionKind(options.getOptionAt(optionIndex).userValue);

            switch (optionKind)
            {
                case OptionKind::NoMangle: flags |= SLANG_COMPILE_FLAG_NO_MANGLING; break;
                case OptionKind::EmitIr: requestImpl->m_emitIr = true; break;
                case OptionKind::LoadStdLib:
                {
                    CommandLineArg fileName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(fileName));

                    // Load the file
                    ScopedAllocation contents;
                    SLANG_RETURN_ON_FAIL(File::readAllBytes(fileName.value, contents));
                    SLANG_RETURN_ON_FAIL(session->loadStdLib(contents.getData(), contents.getSizeInBytes()));
                    break;
                }
                case OptionKind::CompileStdLib: compileStdLib = true; break;
                case OptionKind::ArchiveType:
                {
                    CommandOptions::UserValue value;
                    SLANG_RETURN_ON_FAIL(_expectValue(ValueCategory::ArchiveType, reader, sink, value)); 
                    archiveType = SlangArchiveType(value);
                    break;
                }
                case OptionKind::SaveStdLib:
                {
                    CommandLineArg fileName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(fileName));

                    ComPtr<ISlangBlob> blob;

                    SLANG_RETURN_ON_FAIL(session->saveStdLib(archiveType, blob.writeRef()));
                    SLANG_RETURN_ON_FAIL(File::writeAllBytes(fileName.value, blob->getBufferPointer(), blob->getBufferSize()));
                    break;
                }
                case OptionKind::SaveStdLibBinSource:
                {
                    CommandLineArg fileName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(fileName));

                    ComPtr<ISlangBlob> blob;

                    SLANG_RETURN_ON_FAIL(session->saveStdLib(archiveType, blob.writeRef()));

                    StringBuilder builder;
                    StringWriter writer(&builder, 0);

                    SLANG_RETURN_ON_FAIL(HexDumpUtil::dumpSourceBytes((const uint8_t*)blob->getBufferPointer(), blob->getBufferSize(), 16, &writer));

                    File::writeAllText(fileName.value, builder);
                    break;
                }
                case OptionKind::NoCodeGen: flags |= SLANG_COMPILE_FLAG_NO_CODEGEN; break;
                case OptionKind::DumpIntermediates: compileRequest->setDumpIntermediates(true); break;
                case OptionKind::DumpIrIds:
                {
                    frontEndReq->m_irDumpOptions.flags |= IRDumpOptions::Flag::DumpDebugIds;
                    break;
                }
                case OptionKind::DumpIntermediatePrefix:
                {
                    CommandLineArg prefix;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(prefix));
                    requestImpl->m_dumpIntermediatePrefix = prefix.value;
                    break;
                }
                case OptionKind::OutputIncludes: frontEndReq->outputIncludes = true; break;
                case OptionKind::DumpIr: frontEndReq->shouldDumpIR = true; break;
                case OptionKind::PreprocessorOutput: frontEndReq->outputPreprocessor = true; break;
                case OptionKind::DumpAst: frontEndReq->shouldDumpAST = true; break;
                case OptionKind::Doc:
                {
                    // If compiling stdlib is enabled, will write out documentation
                    compileStdLibFlags |= slang::CompileStdLibFlag::WriteDocumentation;

                    // Enable writing out documentation on the req
                    frontEndReq->shouldDocument = true;
                    break;
                }
                case OptionKind::DumpRepro:
                {
                    CommandLineArg dumpRepro;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(dumpRepro));
                    requestImpl->m_dumpRepro = dumpRepro.value;
                    compileRequest->enableReproCapture();
                    break;
                }
                case OptionKind::DumpReproOnError: requestImpl->m_dumpReproOnError = true; break;
                case OptionKind::ExtractRepro:
                {
                    CommandLineArg reproName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(reproName));

                    {
                        const Result res = ReproUtil::extractFilesToDirectory(reproName.value, sink);
                        if (SLANG_FAILED(res))
                        {
                            sink->diagnose(reproName.loc, Diagnostics::unableExtractReproToDirectory, reproName.value);
                            return res;
                        }
                    }
                    break;
                }
                case OptionKind::ModuleName:
                {
                    CommandLineArg moduleName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(moduleName));

                    compileRequest->setDefaultModuleName(moduleName.value.getBuffer());
                    break;
                }
                case OptionKind::LoadRepro:
                {
                    CommandLineArg reproName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(reproName));

                    List<uint8_t> buffer;
                    {
                        const Result res = ReproUtil::loadState(reproName.value, sink, buffer);
                        if (SLANG_FAILED(res))
                        {
                            sink->diagnose(reproName.loc, Diagnostics::unableToReadFile, reproName.value);
                            return res;
                        }
                    }

                    auto requestState = ReproUtil::getRequest(buffer);
                    MemoryOffsetBase base;
                    base.set(buffer.getBuffer(), buffer.getCount());

                    // If we can find a directory, that exists, we will set up a file system to load from that directory
                    ComPtr<ISlangFileSystem> fileSystem;
                    String dirPath;
                    if (SLANG_SUCCEEDED(ReproUtil::calcDirectoryPathFromFilename(reproName.value, dirPath)))
                    {
                        SlangPathType pathType;
                        if (SLANG_SUCCEEDED(Path::getPathType(dirPath, &pathType)) && pathType == SLANG_PATH_TYPE_DIRECTORY)
                        {
                            fileSystem = new RelativeFileSystem(OSFileSystem::getExtSingleton(), dirPath);
                        }
                    }

                    SLANG_RETURN_ON_FAIL(ReproUtil::load(base, requestState, fileSystem, requestImpl));

                    hasLoadedRepro = true;
                    break;
                }
                case OptionKind::LoadReproDirectory:
                {
                    CommandLineArg reproDirectory;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(reproDirectory));

                    SLANG_RETURN_ON_FAIL(_compileReproDirectory(session, requestImpl, reproDirectory.value, sink));
                    break;
                }
                case OptionKind::ReproFileSystem:
                {
                    CommandLineArg reproName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(reproName));

                    List<uint8_t> buffer;
                    {
                        const Result res = ReproUtil::loadState(reproName.value, sink, buffer);
                        if (SLANG_FAILED(res))
                        {
                            sink->diagnose(reproName.loc, Diagnostics::unableToReadFile, reproName.value);
                            return res;
                        }
                    }

                    auto requestState = ReproUtil::getRequest(buffer);
                    MemoryOffsetBase base;
                    base.set(buffer.getBuffer(), buffer.getCount());

                    // If we can find a directory, that exists, we will set up a file system to load from that directory
                    ComPtr<ISlangFileSystem> dirFileSystem;
                    String dirPath;
                    if (SLANG_SUCCEEDED(ReproUtil::calcDirectoryPathFromFilename(reproName.value, dirPath)))
                    {
                        SlangPathType pathType;
                        if (SLANG_SUCCEEDED(Path::getPathType(dirPath, &pathType)) && pathType == SLANG_PATH_TYPE_DIRECTORY)
                        {
                            dirFileSystem = new RelativeFileSystem(OSFileSystem::getExtSingleton(), dirPath, true);
                        }
                    }

                    ComPtr<ISlangFileSystemExt> fileSystem;
                    SLANG_RETURN_ON_FAIL(ReproUtil::loadFileSystem(base, requestState, dirFileSystem, fileSystem));

                    auto cacheFileSystem = as<CacheFileSystem>(fileSystem);
                    SLANG_ASSERT(cacheFileSystem);

                    // I might want to make the dir file system the fallback file system...
                    cacheFileSystem->setInnerFileSystem(dirFileSystem, cacheFileSystem->getUniqueIdentityMode(), cacheFileSystem->getPathStyle());

                    // Set as the file system
                    compileRequest->setFileSystem(fileSystem);
                    break;
                }
                case OptionKind::SerialIr: frontEndReq->useSerialIRBottleneck = true; break;
                case OptionKind::DisableSpecialization: requestImpl->disableSpecialization = true; break;
                case OptionKind::DisableDynamicDispatch: requestImpl->disableDynamicDispatch = true; break;
                case OptionKind::TrackLiveness: requestImpl->setTrackLiveness(true); break;
                case OptionKind::VerbosePaths: requestImpl->getSink()->setFlag(DiagnosticSink::Flag::VerbosePath); break;
                case OptionKind::DumpWarningDiagnostics: _dumpDiagnostics(Severity::Warning, sink); break;
                case OptionKind::WarningsAsErrors:
                {
                    CommandLineArg operand;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(operand));

                    if (operand.value == "all")
                    {
                        // TODO(JS):
                        // Perhaps there needs to be a way to disable this selectively.
                        requestImpl->getSink()->setFlag(DiagnosticSink::Flag::TreatWarningsAsErrors);
                    }
                    else
                    {
                        SLANG_RETURN_ON_FAIL(_overrideDiagnostics(operand.value.getUnownedSlice(), Severity::Warning, Severity::Error, sink));
                    }
                    break;
                }
                case OptionKind::DisableWarnings:
                {
                    CommandLineArg operand;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(operand));
                    SLANG_RETURN_ON_FAIL(_overrideDiagnostics(operand.value.getUnownedSlice(), Severity::Warning, Severity::Disable, sink));
                    break;
                }
                case OptionKind::DisableWarning:
                {
                    // 5 because -Wno-
                    auto name = argValue.getUnownedSlice().tail(5);
                    SLANG_RETURN_ON_FAIL(_overrideDiagnostic(name, Severity::Warning, Severity::Disable, sink));
                    break;
                }
                case OptionKind::EnableWarning:
                {
                    // 2 because -W
                    auto name = argValue.getUnownedSlice().tail(5);
                    // Enable the warning
                    SLANG_RETURN_ON_FAIL(_overrideDiagnostic(name, Severity::Warning, Severity::Warning, sink));
                    break;
                }
                case OptionKind::VerifyDebugSerialIr: frontEndReq->verifyDebugSerialization = true; break;
                case OptionKind::ValidateIr: frontEndReq->shouldValidateIR = true; break;
                case OptionKind::SkipCodeGen: requestImpl->m_shouldSkipCodegen = true; break;
                case OptionKind::ParameterBlocksUseRegisterSpaces: 
                {
                    getCurrentTarget()->targetFlags |= SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES; 
                    break;
                }
                case OptionKind::IrCompression:
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    SLANG_RETURN_ON_FAIL(SerialParseUtil::parseCompressionType(name.value.getUnownedSlice(), requestImpl->getLinkage()->serialCompressionType));
                    break;
                }
                case OptionKind::Target:
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    const CodeGenTarget format = (CodeGenTarget)TypeTextUtil::findCompileTargetFromName(name.value.getUnownedSlice());

                    if (format == CodeGenTarget::Unknown)
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownCodeGenerationTarget, name.value);
                        return SLANG_FAIL;
                    }

                    RawTarget rawTarget;
                    rawTarget.format = CodeGenTarget(format);

                    rawTargets.add(rawTarget);
                    break;
                }
                case OptionKind::Profile:
                {
                    // A "profile" can specify both a general capability level for
                    // a target, and also (as a legacy/compatibility feature) a
                    // specific stage to use for an entry point.

                    CommandLineArg operand;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(operand));

                    // A a convenience, the `-profile` option supports an operand that consists
                    // of multiple tokens separated with `+`. The eventual goal is that each
                    // of these tokens will represent a capability that should be assumed to
                    // be present on the target.
                    //
                    List<UnownedStringSlice> slices;
                    StringUtil::split(operand.value.getUnownedSlice(), '+', slices);
                    Index sliceCount = slices.getCount();

                    // For now, we will require that the *first* capability in the list is
                    // special, and represents the traditional `Profile` to compile for in
                    // the existing Slang model.
                    //
                    UnownedStringSlice profileName = sliceCount >= 1 ? slices[0] : UnownedTerminatedStringSlice("");

                    SlangProfileID profileID = SlangProfileID(Slang::Profile::lookUp(profileName).raw);
                    if( profileID == SLANG_PROFILE_UNKNOWN )
                    {
                        sink->diagnose(operand.loc, Diagnostics::unknownProfile, profileName);
                        return SLANG_FAIL;
                    }
                    else
                    {
                        auto profile = Profile(profileID);

                        setProfileVersion(getCurrentTarget(), profile.getVersion());

                        // A `-profile` option that also specifies a stage (e.g., `-profile vs_5_0`)
                        // should be treated like a composite (e.g., `-profile sm_5_0 -stage vertex`)
                        auto stage = profile.getStage();
                        if(stage != Stage::Unknown)
                        {
                            setStage(getCurrentEntryPoint(), stage);
                        }
                    }

                    // Any additional capability tokens will be assumed to represent `CapabilityAtom`s.
                    // Those atoms will need to be added to the supported capabilities of the target.
                    // 
                    for(Index i = 1; i < sliceCount; ++i)
                    {
                        UnownedStringSlice atomName = slices[i];
                        CapabilityAtom atom = findCapabilityAtom(atomName);
                        if( atom == CapabilityAtom::Invalid )
                        {
                            sink->diagnose(operand.loc, Diagnostics::unknownProfile, atomName);
                            return SLANG_FAIL;
                        }

                        addCapabilityAtom(getCurrentTarget(), atom);
                    }

                    break;
                }
                case OptionKind::Capability:
                {
                    // The `-capability` option is similar to `-profile` but does not set the actual profile
                    // for a target (it just adds capabilities).
                    //
                    // TODO: Once profiles are treated as capabilities themselves, it might be possible
                    // to treat `-profile` and `-capability` as aliases, although there might still be
                    // value in only allowing a single `-profile` option per target while still allowing
                    // zero or more `-capability` options.

                    CommandLineArg operand;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(operand));

                    List<UnownedStringSlice> slices;
                    StringUtil::split(operand.value.getUnownedSlice(), '+', slices);
                    Index sliceCount = slices.getCount();
                    for(Index i = 0; i < sliceCount; ++i)
                    {
                        UnownedStringSlice atomName = slices[i];
                        CapabilityAtom atom = findCapabilityAtom(atomName);
                        if( atom == CapabilityAtom::Invalid )
                        {
                            sink->diagnose(operand.loc, Diagnostics::unknownProfile, atomName);
                            return SLANG_FAIL;
                        }

                        addCapabilityAtom(getCurrentTarget(), atom);
                    }
                    break;
                }
                case OptionKind::Stage:
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    Stage stage = findStageByName(name.value);
                    if( stage == Stage::Unknown )
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownStage, name.value);
                        return SLANG_FAIL;
                    }
                    else
                    {
                        setStage(getCurrentEntryPoint(), stage);
                    }
                    break;
                }
                case OptionKind::EntryPointName:
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    RawEntryPoint rawEntryPoint;
                    rawEntryPoint.name = name.value;
                    rawEntryPoint.translationUnitIndex = currentTranslationUnitIndex;

                    rawEntryPoints.add(rawEntryPoint);
                    break;
                }
                case OptionKind::Language:
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    const SourceLanguage sourceLanguage = (SourceLanguage)TypeTextUtil::findSourceLanguage(name.value.getUnownedSlice());

                    if (sourceLanguage == SourceLanguage::Unknown)
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownSourceLanguage, name.value);
                        return SLANG_FAIL;
                    }
                    else
                    {
                        while (reader.hasArg() && !reader.peekValue().startsWith("-"))
                        {
                            SLANG_RETURN_ON_FAIL(addInputPath(reader.getValueAndAdvance().getBuffer(), sourceLanguage));
                        }
                    }
                    break;
                }
                case OptionKind::PassThrough:
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
                    if (SLANG_FAILED(TypeTextUtil::findPassThrough(name.value.getUnownedSlice(), passThrough)))
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownPassThroughTarget, name.value);
                        return SLANG_FAIL;
                    }

                    compileRequest->setPassThrough(passThrough);
                    break;
                }
                case OptionKind::MacroDefine:
                {
                    // The value to be defined might be part of the same option, as in:
                    //     -DFOO
                    // or it might come separately, as in:
                    //     -D FOO

                    UnownedStringSlice slice = argValue.getUnownedSlice().tail(2);

                    CommandLineArg nextArg;
                    if (slice.getLength() <= 0)
                    {
                        SLANG_RETURN_ON_FAIL(reader.expectArg(nextArg));
                        slice = nextArg.value.getUnownedSlice();
                    }

                    // The string that sets up the define can have an `=` between
                    // the name to be defined and its value, so we search for one.
                    const Index equalIndex = slice.indexOf('=');

                    // Now set the preprocessor define

                    if (equalIndex >= 0)
                    {
                        // If we found an `=`, we split the string...
                        compileRequest->addPreprocessorDefine(String(slice.head(equalIndex)).getBuffer(), String(slice.tail(equalIndex + 1)).getBuffer());
                    }
                    else
                    {
                        // If there was no `=`, then just #define it to an empty string
                        compileRequest->addPreprocessorDefine(String(slice).getBuffer(), "");
                    }
                    break;
                }
                case OptionKind::Include:
                {
                    // The value to be defined might be part of the same option, as in:
                    //     -IFOO
                    // or it might come separately, as in:
                    //     -I FOO
                    // (see handling of `-D` above)
                    UnownedStringSlice slice = argValue.getUnownedSlice().tail(2);

                    CommandLineArg nextArg;
                    if (slice.getLength() <= 0)
                    {
                        // Need to read another argument from the command line
                        SLANG_RETURN_ON_FAIL(reader.expectArg(nextArg));
                        slice = nextArg.value.getUnownedSlice();
                    }

                    compileRequest->addSearchPath(String(slice).getBuffer());
                    break;
                }
                case OptionKind::Output:
                {
                    //
                    // A `-o` option is used to specify a desired output file.
                    CommandLineArg outputPath;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(outputPath));

                    addOutputPath(outputPath.value.getBuffer());
                    break;
                }
                case OptionKind::DepFile:
                {
                    CommandLineArg dependencyPath;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(dependencyPath));

                    if (requestImpl->m_dependencyOutputPath.getLength() == 0)
                    {
                        requestImpl->m_dependencyOutputPath = dependencyPath.value;
                    }
                    else
                    {
                        sink->diagnose(dependencyPath.loc, Diagnostics::duplicateDependencyOutputPaths);
                        return SLANG_FAIL;
                    }
                    break;
                }
                case OptionKind::MatrixLayoutRow:       defaultMatrixLayoutMode = SlangMatrixLayoutMode(kMatrixLayoutMode_RowMajor); break;
                case OptionKind::MatrixLayoutColumn:    defaultMatrixLayoutMode = SlangMatrixLayoutMode(kMatrixLayoutMode_ColumnMajor); break;            
                case OptionKind::LineDirectiveMode:
                {
                    CommandOptions::UserValue value;
                    SLANG_RETURN_ON_FAIL(_expectValue(ValueCategory::LineDirectiveMode, reader, sink, value)); 
                    compileRequest->setLineDirectiveMode(SlangLineDirectiveMode(value));
                    break;
                }
                case OptionKind::FloatingPointMode:
                {
                    CommandOptions::UserValue value;
                    SLANG_RETURN_ON_FAIL(_expectValue(ValueCategory::FloatingPointMode, reader, sink, value)); 
                    setFloatingPointMode(getCurrentTarget(), FloatingPointMode(value));
                    break;
                }
                case OptionKind::Optimization:
                {
                    UnownedStringSlice levelSlice = argValue.getUnownedSlice().tail(2);
                    SlangOptimizationLevel level = SLANG_OPTIMIZATION_LEVEL_DEFAULT;

                    if (levelSlice.getLength())
                    {
                        CommandOptions::UserValue value;
                        SLANG_RETURN_ON_FAIL(_getValue(ValueCategory::OptimizationLevel, arg, levelSlice, sink, value));
                        level = SlangOptimizationLevel(value);
                    }

                    compileRequest->setOptimizationLevel(level);
                    break;
                }
                case OptionKind::DebugInformation:
                {
                    auto name = argValue.getUnownedSlice().tail(2);

                    // Note: unlike with `-O` above, we have to consider that other
                    // options might have names that start with `-g` and so cannot
                    // just detect it as a prefix.
                    if (name.getLength() == 0)
                    {
                        // The default is standard
                        compileRequest->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_STANDARD);
                    }
                    else 
                    {
                        CommandOptions::UserValue value;
                        ValueCategory valueCat;
                        ValueCategory valueCats[] = { ValueCategory::DebugLevel, ValueCategory::DebugInfoFormat };
                        SLANG_RETURN_ON_FAIL(_getValue(makeConstArrayView(valueCats), arg, name, sink, valueCat, value));

                        if (valueCat == ValueCategory::DebugLevel)
                        {
                            const auto level = (SlangDebugInfoLevel)value;
                            compileRequest->setDebugInfoLevel(level);
                        }
                        else
                        {
                            const auto debugFormat = (SlangDebugInfoFormat)value;
                            compileRequest->setDebugInfoFormat(debugFormat);
                        }
                    }
                    break;
                }
                case OptionKind::DefaultImageFormatUnknown: requestImpl->useUnknownImageFormatAsDefault = true; break;
                case OptionKind::Obfuscate: requestImpl->getLinkage()->m_obfuscateCode = true; break;
                case OptionKind::FileSystem:
                {
                    CommandOptions::UserValue value;
                    SLANG_RETURN_ON_FAIL(_expectValue(ValueCategory::FileSystemType, reader, sink, value));
                    typedef TypeTextUtil::FileSystemType FileSystemType;

                    switch (FileSystemType(value))
                    {
                        case FileSystemType::Default:   compileRequest->setFileSystem(nullptr); break;
                        case FileSystemType::LoadFile:  compileRequest->setFileSystem(OSFileSystem::getLoadSingleton()); break;
                        case FileSystemType::Os:        compileRequest->setFileSystem(OSFileSystem::getExtSingleton()); break;
                    }
                    break;
                }
                case OptionKind::ReferenceModule:
                {
                    CommandLineArg referenceModuleName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(referenceModuleName));

                    const auto path = referenceModuleName.value;

                    auto desc = ArtifactDescUtil::getDescFromPath(path.getUnownedSlice());

                    if (desc.kind == ArtifactKind::Unknown)
                    {
                        sink->diagnose(referenceModuleName.loc, Diagnostics::unknownLibraryKind, Path::getPathExt(path));
                        return SLANG_FAIL;
                    }

                    // If it's a GPU binary, then we'll assume it's a library
                    if (ArtifactDescUtil::isGpuUsable(desc))
                    {
                        desc.kind = ArtifactKind::Library;
                    }

                    // If its a zip we'll *assume* its a zip holding compilation results
                    if (desc.kind == ArtifactKind::Zip)
                    {
                        desc.payload = ArtifactPayload::CompileResults;
                    }
                    
                    if (!ArtifactDescUtil::isLinkable(desc))
                    {
                        sink->diagnose(referenceModuleName.loc, Diagnostics::kindNotLinkable, Path::getPathExt(path));
                        return SLANG_FAIL;
                    }

                    const String name = ArtifactDescUtil::getBaseNameFromPath(desc, path.getUnownedSlice());

                    // Create the artifact
                    auto artifact = Artifact::create(desc, name.getUnownedSlice()); 

                    // There is a problem here if I want to reference a library that is a 'system' library or is not directly a file
                    // In that case the path shouldn't be set and the name should completely define the library.
                    // Seeing as on all targets the baseName doesn't have an extension, and all library types do
                    // if the name doesn't have an extension we can assume there is no path to it.
                    
                    ComPtr<IOSFileArtifactRepresentation> fileRep;
                    if (Path::getPathExt(path).getLength() <= 0)
                    {
                        // If there is no extension *assume* it is the name of a system level library
                        fileRep = new OSFileArtifactRepresentation(IOSFileArtifactRepresentation::Kind::NameOnly, path.getUnownedSlice(), nullptr);
                    }
                    else
                    {
                        fileRep = new OSFileArtifactRepresentation(IOSFileArtifactRepresentation::Kind::Reference, path.getUnownedSlice(), nullptr);
                        if (!fileRep->exists())
                        {
                            sink->diagnose(referenceModuleName.loc, Diagnostics::libraryDoesNotExist, path);
                            return SLANG_FAIL;
                        }
                    }
                    artifact->addRepresentation(fileRep);

                    SLANG_RETURN_ON_FAIL(_addLibraryReference(requestImpl, artifact));
                    break;
                }
                case OptionKind::Version:
                {
                    sink->diagnoseRaw(Severity::Note, session->getBuildTagString());
                    break;
                }
                case OptionKind::HelpStyle:
                {
                    CommandOptions::UserValue value;
                    SLANG_RETURN_ON_FAIL(_expectValue(ValueCategory::HelpStyle, reader, sink, value));
                    helpStyle = CommandOptionsWriter::Style(value);
                    break;
                }
                case OptionKind::Help:
                {
                    Index categoryIndex = -1;

                    if (reader.hasArg())
                    {
                        auto catArg = reader.getArgAndAdvance();

                        categoryIndex = options.findCategoryByCaseInsensitiveName(catArg.value.getUnownedSlice());
                        if (categoryIndex < 0)
                        {
                            sink->diagnose(catArg.loc, Diagnostics::unknownHelpCategory);
                            return SLANG_FAIL;
                        }
                    }

                    CommandOptionsWriter::Options writerOptions;
                    writerOptions.style = helpStyle;

                    auto writer = CommandOptionsWriter::create(writerOptions);

                    auto& buf = writer->getBuilder();

                    if (categoryIndex < 0)
                    {
                        // If it's the text style we can inject usage at the top
                        if (helpStyle == CommandOptionsWriter::Style::Text)
                        {
                            _appendUsageTitle(buf);
                        }
                        else
                        {
                            // NOTE! We need this preamble because if we have links,
                            // we have to make sure the first thing in markdown *isn't* <>

                            buf << "# Slang Command Line Options\n\n";
                            buf << "*Usage:*\n";
                            buf << "```\n";
                            buf << "slangc [options...] [--] <input files>\n\n";
                            buf << "# For help\n";
                            buf << "slangc -h\n\n";
                            buf << "# To generate this file\n";
                            buf << "slangc -help-style markdown -h\n";
                            buf << "```\n";
                        }

                        writer->appendDescription(&options);
                    }
                    else
                    {
                        writer->appendDescriptionForCategory(&options, categoryIndex);
                    }
                    
                    sink->diagnoseRaw(Severity::Note, buf.getBuffer());
                 
                    return SLANG_FAIL;
                }
                case OptionKind::EmitSpirvDirectly: getCurrentTarget()->targetFlags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY; break;
             
                case OptionKind::DefaultDownstreamCompiler:
                {
                    CommandLineArg sourceLanguageArg, compilerArg;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(sourceLanguageArg));
                    SLANG_RETURN_ON_FAIL(reader.expectArg(compilerArg));

                    SlangSourceLanguage sourceLanguage = TypeTextUtil::findSourceLanguage(sourceLanguageArg.value.getUnownedSlice());
                    if (sourceLanguage == SLANG_SOURCE_LANGUAGE_UNKNOWN)
                    {
                        sink->diagnose(sourceLanguageArg.loc, Diagnostics::unknownSourceLanguage, sourceLanguageArg.value);
                        return SLANG_FAIL;
                    }

                    SlangPassThrough compiler;
                    if (SLANG_FAILED(TypeTextUtil::findPassThrough(compilerArg.value.getUnownedSlice(), compiler)))
                    {
                        sink->diagnose(compilerArg.loc, Diagnostics::unknownPassThroughTarget, compilerArg.value);
                        return SLANG_FAIL;
                    }

                    if (SLANG_FAILED(session->setDefaultDownstreamCompiler(sourceLanguage, compiler)))
                    {
                        sink->diagnose(arg.loc, Diagnostics::unableToSetDefaultDownstreamCompiler, compilerArg.value, sourceLanguageArg.value);
                        return SLANG_FAIL;
                    }
                    break;
                }       
                case OptionKind::CompilerPath:
                {
                    const Index index = argValue.lastIndexOf('-');
                    if (index >= 0)
                    {
                        CommandLineArg name;
                        SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                        UnownedStringSlice passThroughSlice = argValue.getUnownedSlice().head(index).tail(1);

                        // Skip the initial -, up to the last -
                        SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
                        if (SLANG_SUCCEEDED(TypeTextUtil::findPassThrough(passThroughSlice, passThrough)))
                        {
                            session->setDownstreamCompilerPath(passThrough, name.value.getBuffer());
                            continue;
                        }
                        else
                        {
                            sink->diagnose(arg.loc, Diagnostics::unknownDownstreamCompiler, passThroughSlice);
                            return SLANG_FAIL;
                        }
                    }
                    break;
                }
                case OptionKind::InputFilesRemain:
                {
                    // The `--` option causes us to stop trying to parse options,
                    // and treat the rest of the command line as input file names:
                    while (reader.hasArg())
                    {
                        SLANG_RETURN_ON_FAIL(addInputPath(reader.getValueAndAdvance().getBuffer()));
                    }
                    break;
                } 
                default:
                {
                    // Hmmm, we looked up and produced a valid enum, but it wasn't handled in the switch... 
                    sink->diagnose(arg.loc, Diagnostics::unknownCommandLineOption, argValue);

                    _outputMinimalUsage(sink);
                    return SLANG_FAIL;
                }
            }
        }

        if (compileStdLib)
        {
            SLANG_RETURN_ON_FAIL(session->compileStdLib(compileStdLibFlags));
        }

        // TODO(JS): This is a restriction because of how setting of state works for load repro
        // If a repro has been loaded, then many of the following options will overwrite
        // what was set up. So for now they are ignored, and only parameters set as part
        // of the loop work if they are after -load-repro
        if (hasLoadedRepro)
        {
            return SLANG_OK;
        }

        compileRequest->setCompileFlags(flags);

        // As a compatability feature, if the user didn't list any explicit entry
        // point names, *and* they are compiling a single translation unit, *and* they
        // have either specified a stage, or we can assume one from the naming
        // of the translation unit, then we assume they wanted to compile a single
        // entry point named `main`.
        //
        if(rawEntryPoints.getCount() == 0
           && rawTranslationUnits.getCount() == 1
           && (defaultEntryPoint.stage != Stage::Unknown
                || rawTranslationUnits[0].impliedStage != Stage::Unknown))
        {
            RawEntryPoint entry;
            entry.name = "main";
            entry.translationUnitIndex = 0;
            rawEntryPoints.add(entry);
        }

        // If the user (manually or implicitly) specified only a single entry point,
        // then we allow the associated stage to be specified either before or after
        // the entry point. This means that if there is a stage attached
        // to the "default" entry point, we should copy it over to the
        // explicit one.
        //
        if( rawEntryPoints.getCount() == 1 )
        {
            if(defaultEntryPoint.stage != Stage::Unknown)
            {
                setStage(getCurrentEntryPoint(), defaultEntryPoint.stage);
            }

            if(defaultEntryPoint.redundantStageSet)
                getCurrentEntryPoint()->redundantStageSet = true;
            if(defaultEntryPoint.conflictingStagesSet)
                getCurrentEntryPoint()->conflictingStagesSet = true;
        }
        else
        {
            // If the "default" entry point has had a stage (or
            // other state, if we add other per-entry-point state)
            // specified, but there is more than one entry point,
            // then that state doesn't apply to anything and we
            // should issue an error to tell the user something
            // funky is going on.
            //
            if( defaultEntryPoint.stage != Stage::Unknown )
            {
                if( rawEntryPoints.getCount() == 0 )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::stageSpecificationIgnoredBecauseNoEntryPoints);
                }
                else
                {
                    sink->diagnose(SourceLoc(), Diagnostics::stageSpecificationIgnoredBecauseBeforeAllEntryPoints);
                }
            }
        }

        // Slang requires that every explicit entry point indicate the translation
        // unit it comes from. If there is only one translation unit specified,
        // then implicitly all entry points come from it.
        //
        if(translationUnitCount == 1)
        {
            for( auto& entryPoint : rawEntryPoints )
            {
                entryPoint.translationUnitIndex = 0;
            }
        }
        else
        {
            // Otherwise, we require that all entry points be specified after
            // the translation unit to which tye belong.
            bool anyEntryPointWithoutTranslationUnit = false;
            for( auto& entryPoint : rawEntryPoints )
            {
                // Skip entry points that are already associated with a translation unit...
                if( entryPoint.translationUnitIndex != -1 )
                    continue;

                anyEntryPointWithoutTranslationUnit = true;
            }
            if( anyEntryPointWithoutTranslationUnit )
            {
                sink->diagnose(SourceLoc(), Diagnostics::entryPointsNeedToBeAssociatedWithTranslationUnits);
                return SLANG_FAIL;
            }
        }

        // Now that entry points are associated with translation units,
        // we can make one additional pass where if an entry point has
        // no specified stage, but the nameing of its translation unit
        // implies a stage, we will use that (a manual `-stage` annotation
        // will always win out in such a case).
        //
        for( auto& rawEntryPoint : rawEntryPoints )
        {
            // Skip entry points that already have a stage.
            if(rawEntryPoint.stage != Stage::Unknown)
                continue;

            // Sanity check: don't process entry points with no associated translation unit.
            if( rawEntryPoint.translationUnitIndex == -1 )
                continue;

            auto impliedStage = rawTranslationUnits[rawEntryPoint.translationUnitIndex].impliedStage;
            if(impliedStage != Stage::Unknown)
                rawEntryPoint.stage = impliedStage;
        }

        // Note: it is possible that some entry points still won't have associated
        // stages at this point, but we don't want to error out here, because
        // those entry points might get stages later, as part of semantic checking,
        // if the corresponding function has a `[shader("...")]` attribute.

        // Now that we've tried to establish stages for entry points, we can
        // issue diagnostics for cases where stages were set redundantly or
        // in conflicting ways.
        //
        for( auto& rawEntryPoint : rawEntryPoints )
        {
            if( rawEntryPoint.conflictingStagesSet )
            {
                sink->diagnose(SourceLoc(), Diagnostics::conflictingStagesForEntryPoint, rawEntryPoint.name);
            }
            else if( rawEntryPoint.redundantStageSet )
            {
                sink->diagnose(SourceLoc(), Diagnostics::sameStageSpecifiedMoreThanOnce, rawEntryPoint.stage, rawEntryPoint.name);
            }
            else if( rawEntryPoint.translationUnitIndex != -1 )
            {
                // As a quality-of-life feature, if the file name implies a particular
                // stage, but the user manually specified something different for
                // their entry point, give a warning in case they made a mistake.

                auto& rawTranslationUnit = rawTranslationUnits[rawEntryPoint.translationUnitIndex];
                if( rawTranslationUnit.impliedStage != Stage::Unknown
                    && rawEntryPoint.stage != Stage::Unknown
                    && rawTranslationUnit.impliedStage != rawEntryPoint.stage )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::explicitStageDoesntMatchImpliedStage, rawEntryPoint.name, rawEntryPoint.stage, rawTranslationUnit.impliedStage);
                }
            }
        }

        // If the user is requesting code generation via pass-through,
        // then any entry points they specify need to have a stage set,
        // because fxc/dxc/glslang don't have a facility for taking
        // a named entry point and pulling its stage from an attribute.
        //
        if(_passThroughRequiresStage(requestImpl->m_passThrough) )
        {
            for( auto& rawEntryPoint : rawEntryPoints )
            {
                if( rawEntryPoint.stage == Stage::Unknown )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::noStageSpecifiedInPassThroughMode, rawEntryPoint.name);
                }
            }
        }

        // We now have inferred enough information to add the
        // entry points to our compile request.
        //
        for( auto& rawEntryPoint : rawEntryPoints )
        {
            if(rawEntryPoint.translationUnitIndex < 0)
                continue;

            auto translationUnitID = rawTranslationUnits[rawEntryPoint.translationUnitIndex].translationUnitID;

            int entryPointID = compileRequest->addEntryPoint(
                translationUnitID,
                rawEntryPoint.name.begin(),
                SlangStage(rawEntryPoint.stage));

            rawEntryPoint.entryPointID = entryPointID;
        }

        // We are going to build a mapping from target formats to the
        // target that handles that format.
        Dictionary<CodeGenTarget, int> mapFormatToTargetIndex;

        // If there was no explicit `-target` specified, then we will look
        // at the `-o` options to see what we can infer.
        //
        if(rawTargets.getCount() == 0)
        {
            // If there are no targets and no outputs
            if (rawOutputs.getCount() == 0)
            {
                // And we have a container for output, then enable emitting SlangIR module
                if (requestImpl->m_containerFormat != ContainerFormat::None)
                {
                    requestImpl->m_emitIr = true;
                }
            }
            else
            {
                for(auto& rawOutput : rawOutputs)
                {
                    // Some outputs don't imply a target format, and we shouldn't use those for inference.
                    auto impliedFormat = rawOutput.impliedFormat;
                    if( impliedFormat == CodeGenTarget::Unknown )
                        continue;

                    int targetIndex = 0;
                    if( !mapFormatToTargetIndex.tryGetValue(impliedFormat, targetIndex) )
                    {
                        targetIndex = (int) rawTargets.getCount();

                        RawTarget rawTarget;
                        rawTarget.format = impliedFormat;
                        rawTargets.add(rawTarget);

                        mapFormatToTargetIndex[impliedFormat] = targetIndex;
                    }

                    rawOutput.targetIndex = targetIndex;
                }
            }
        }
        else
        {
            // If there were explicit targets, then we will use those, but still
            // build up our mapping. We should object if the same target format
            // is specified more than once (just because of the ambiguities
            // it will create).
            //
            int targetCount = (int) rawTargets.getCount();
            for(int targetIndex = 0; targetIndex < targetCount; ++targetIndex)
            {
                auto format = rawTargets[targetIndex].format;

                if( mapFormatToTargetIndex.containsKey(format) )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::duplicateTargets, format);
                }
                else
                {
                    mapFormatToTargetIndex[format] = targetIndex;
                }
            }
        }

        // If we weren't able to infer any targets from output paths (perhaps
        // because there were no output paths), but there was a profile specified,
        // then we can try to infer a target from the profile.
        //
        if( rawTargets.getCount() == 0
            && defaultTarget.profileVersion != ProfileVersion::Unknown
            && !defaultTarget.conflictingProfilesSet)
        {
            // Let's see if the chosen profile allows us to infer
            // the code gen target format that the user probably meant.
            //
            CodeGenTarget inferredFormat = CodeGenTarget::Unknown;
            auto profileVersion = defaultTarget.profileVersion;
            switch( Profile(profileVersion).getFamily() )
            {
            default:
                break;

                // For GLSL profile versions, we will assume SPIR-V
                // is the output format the user intended.
            case ProfileFamily::GLSL:
                inferredFormat = CodeGenTarget::SPIRV;
                break;

                // For DX profile versions, we will assume that the
                // user wants DXIL for Shader Model 6.0 and up,
                // and DXBC for all earlier versions.
                //
                // Note: There is overlap where both DXBC and DXIL
                // nominally support SM 5.1, but in general we
                // expect users to prefer to make a clean break
                // at SM 6.0. Anybody who cares about the overlap
                // cases should manually specify `-target dxil`.
                //
            case ProfileFamily::DX:
                if( profileVersion >= ProfileVersion::DX_6_0 )
                {
                    inferredFormat = CodeGenTarget::DXIL;
                }
                else
                {
                    inferredFormat = CodeGenTarget::DXBytecode;
                }
                break;
            }

            if( inferredFormat != CodeGenTarget::Unknown )
            {
                RawTarget rawTarget;
                rawTarget.format = inferredFormat;
                rawTargets.add(rawTarget);
            }
        }

        // Similar to the case for entry points, if there is a single target,
        // then we allow some of its options to come from the "default"
        // target state.
        if(rawTargets.getCount() == 1)
        {
            if(defaultTarget.profileVersion != ProfileVersion::Unknown)
            {
                setProfileVersion(getCurrentTarget(), defaultTarget.profileVersion);
            }
            for( auto atom : defaultTarget.capabilityAtoms )
            {
                addCapabilityAtom(getCurrentTarget(), atom);
            }

            getCurrentTarget()->targetFlags |= defaultTarget.targetFlags;

            if( defaultTarget.floatingPointMode != FloatingPointMode::Default )
            {
                setFloatingPointMode(getCurrentTarget(), defaultTarget.floatingPointMode);
            }
        }
        else
        {
            // If the "default" target has had a profile (or other state)
            // specified, but there is != 1 taget, then that state doesn't
            // apply to anythign and we should give the user an error.
            //
            if( defaultTarget.profileVersion != ProfileVersion::Unknown )
            {
                if( rawTargets.getCount() == 0 )
                {
                    // This should only happen if there were multiple `-profile` options,
                    // so we didn't try to infer a target, or if the `-profile` option
                    // somehow didn't imply a target.
                    //
                    sink->diagnose(SourceLoc(), Diagnostics::profileSpecificationIgnoredBecauseNoTargets);
                }
                else
                {
                    sink->diagnose(SourceLoc(), Diagnostics::profileSpecificationIgnoredBecauseBeforeAllTargets);
                }
            }

            if( defaultTarget.targetFlags )
            {
                if( rawTargets.getCount() == 0 )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::targetFlagsIgnoredBecauseNoTargets);
                }
                else
                {
                    sink->diagnose(SourceLoc(), Diagnostics::targetFlagsIgnoredBecauseBeforeAllTargets);
                }
            }

            if( defaultTarget.floatingPointMode != FloatingPointMode::Default )
            {
                if( rawTargets.getCount() == 0 )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::targetFlagsIgnoredBecauseNoTargets);
                }
                else
                {
                    sink->diagnose(SourceLoc(), Diagnostics::targetFlagsIgnoredBecauseBeforeAllTargets);
                }
            }

        }

        for(auto& rawTarget : rawTargets)
        {
            if( rawTarget.conflictingProfilesSet )
            {
                sink->diagnose(SourceLoc(), Diagnostics::conflictingProfilesSpecifiedForTarget, rawTarget.format);
            }
            else if( rawTarget.redundantProfileSet )
            {
                sink->diagnose(SourceLoc(), Diagnostics::sameProfileSpecifiedMoreThanOnce, rawTarget.profileVersion, rawTarget.format);
            }
        }

        // TODO: do we need to require that a target must have a profile specified,
        // or will we continue to allow the profile to be inferred from the target?

        // We now have enough information to go ahead and declare the targets
        // through the Slang API:
        //
        for(auto& rawTarget : rawTargets)
        {
            int targetID = compileRequest->addCodeGenTarget(SlangCompileTarget(rawTarget.format));
            rawTarget.targetID = targetID;

            if( rawTarget.profileVersion != ProfileVersion::Unknown )
            {
                compileRequest->setTargetProfile(targetID, SlangProfileID(Profile(rawTarget.profileVersion).raw));
            }
            for( auto atom : rawTarget.capabilityAtoms )
            {
                requestImpl->addTargetCapability(targetID, SlangCapabilityID(atom));
            }

            if( rawTarget.targetFlags )
            {
                compileRequest->setTargetFlags(targetID, rawTarget.targetFlags);
            }

            if( rawTarget.floatingPointMode != FloatingPointMode::Default )
            {
                compileRequest->setTargetFloatingPointMode(targetID, SlangFloatingPointMode(rawTarget.floatingPointMode));
            }
        }

        if(defaultMatrixLayoutMode != SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
        {
            compileRequest->setMatrixLayoutMode(defaultMatrixLayoutMode);
        }

        // Next we need to sort out the output files specified with `-o`, and
        // figure out which entry point and/or target they apply to.
        //
        // If there is only a single entry point, then that is automatically
        // the entry point that should be associated with all outputs.
        //
        if( rawEntryPoints.getCount() == 1 )
        {
            for( auto& rawOutput : rawOutputs )
            {
                rawOutput.entryPointIndex = 0;
            }
        }
        //
        // Similarly, if there is only one target, then all outputs must
        // implicitly appertain to that target.
        //
        if( rawTargets.getCount() == 1 )
        {
            for( auto& rawOutput : rawOutputs )
            {
                rawOutput.targetIndex = 0;
            }
        }

        // If we don't have any raw outputs but do have a raw target,
        // add an empty' rawOutput for certain targets where the expected behavior is obvious.
        if (rawOutputs.getCount() == 0 &&
            rawTargets.getCount() == 1 &&
            (rawTargets[0].format == CodeGenTarget::HostCPPSource ||
            rawTargets[0].format == CodeGenTarget::PyTorchCppBinding ||
            rawTargets[0].format == CodeGenTarget::CUDASource ||
            ArtifactDescUtil::makeDescForCompileTarget(asExternal(rawTargets[0].format)).kind == ArtifactKind::HostCallable))
        {
            RawOutput rawOutput;
            rawOutput.impliedFormat = rawTargets[0].format;
            rawOutput.targetIndex = 0;
            rawOutputs.add(rawOutput);
        }

        // Consider the output files specified via `-o` and try to figure
        // out how to deal with them.
        //
        for(auto& rawOutput : rawOutputs)
        {
            // For now, most output formats need to be tightly bound to
            // both a target and an entry point.

            // If an output doesn't have a target associated with
            // it, then search for the target with the matching format.
            if( rawOutput.targetIndex == -1 )
            {
                auto impliedFormat = rawOutput.impliedFormat;
                int targetIndex = -1;

                if(impliedFormat == CodeGenTarget::Unknown)
                {
                    // If we hit this case, then it means that we need to pick the
                    // target to assocaite with this output based on its implied
                    // format, but the file path doesn't direclty imply a format
                    // (it doesn't have a suffix like `.spv` that tells us what to write).
                    //
                    sink->diagnose(SourceLoc(), Diagnostics::cannotDeduceOutputFormatFromPath, rawOutput.path);
                }
                else if( mapFormatToTargetIndex.tryGetValue(rawOutput.impliedFormat, targetIndex) )
                {
                    rawOutput.targetIndex = targetIndex;
                }
                else
                {
                    sink->diagnose(SourceLoc(), Diagnostics::cannotMatchOutputFileToTarget, rawOutput.path, rawOutput.impliedFormat);
                }
            }

            // We won't do any searching to match an output file
            // with an entry point, since the case of a single entry
            // point was handled above, and the user is expected to
            // follow the ordering rules when using multiple entry points.
            if( rawOutput.entryPointIndex == -1 )
            {
                if (rawOutput.targetIndex != -1 )
                {
                    auto outputFormat = rawTargets[rawOutput.targetIndex].format;
                    // Here we check whether the given output format supports multiple entry points
                    // When we add targets with support for multiple entry points,
                    // we should update this switch with those new formats
                    switch (outputFormat)
                    {
                    case CodeGenTarget::CPPSource:
                    case CodeGenTarget::PTX:
                    case CodeGenTarget::CUDASource:

                    case CodeGenTarget::HostHostCallable:
                    case CodeGenTarget::ShaderHostCallable:
                    case CodeGenTarget::HostExecutable:
                    case CodeGenTarget::ShaderSharedLibrary:
                    case CodeGenTarget::PyTorchCppBinding:
                    case CodeGenTarget::DXIL:

                        rawOutput.isWholeProgram = true;
                        break;
                    default:
                        sink->diagnose(SourceLoc(), Diagnostics::cannotMatchOutputFileToEntryPoint, rawOutput.path);
                        break;
                    }
                }
            }
        }


        // Now that we've diagnosed the output paths, we can add them
        // to the compile request at the appropriate locations.
        //
        // We will consider the output files specified via `-o` and try to figure
        // out how to deal with them.
        //
        for(auto& rawOutput : rawOutputs)
        {
            if(rawOutput.targetIndex == -1) continue;
            auto targetID = rawTargets[rawOutput.targetIndex].targetID;
            auto target = requestImpl->getLinkage()->targets[targetID];
            RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
            if( !requestImpl->m_targetInfos.tryGetValue(target, targetInfo) )
            {
                targetInfo = new EndToEndCompileRequest::TargetInfo();
                requestImpl->m_targetInfos[target] = targetInfo;
            }

            if (rawOutput.isWholeProgram)
            {
                if (targetInfo->wholeTargetOutputPath != "")
                {
                    sink->diagnose(SourceLoc(), Diagnostics::duplicateOutputPathsForTarget, target->getTarget());
                }
                else
                {
                    target->addTargetFlags(SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);
                    targetInfo->wholeTargetOutputPath = rawOutput.path;
                }
            }
            else
            {
                if (rawOutput.entryPointIndex == -1) continue;

                Int entryPointID = rawEntryPoints[rawOutput.entryPointIndex].entryPointID;
                auto entryPointReq = requestImpl->getFrontEndReq()->getEntryPointReqs()[entryPointID];

                //String outputPath;
                if (targetInfo->entryPointOutputPaths.containsKey(entryPointID))
                {
                    sink->diagnose(SourceLoc(), Diagnostics::duplicateOutputPathsForEntryPointAndTarget, entryPointReq->getName(), target->getTarget());
                }
                else
                {
                    targetInfo->entryPointOutputPaths[entryPointID] = rawOutput.path;
                }
            }
        }

        return (sink->getErrorCount() == 0) ? SLANG_OK : SLANG_FAIL;
    }
};

SlangResult parseOptions(
    SlangCompileRequest*    inCompileRequest,
    int                     argc,
    char const* const*      argv)
{
    Slang::EndToEndCompileRequest* compileRequest = asInternal(inCompileRequest);

    OptionsParser parser;
    parser.compileRequest = inCompileRequest;
    parser.requestImpl = compileRequest;
    parser.session = asInternal(compileRequest->getSession());

    Result res = parser.parse(argc, argv);

    DiagnosticSink* sink = compileRequest->getSink();
    if (sink->getErrorCount() > 0)
    {
        // Put the errors in the diagnostic 
        compileRequest->m_diagnosticOutput = sink->outputBuffer.produceString();
    }

    return res;
}


} // namespace Slang


