// slang-options.cpp

// Implementation of options parsing for `slangc` command line,
// and also for API interface that takes command-line argument strings.

#include "slang-options.h"

#include "../../slang.h"

#include "slang-compiler.h"
#include "slang-profile.h"

#include "../compiler-core/slang-artifact.h"

#include "slang-repro.h"
#include "slang-serialize-ir.h"

#include "../core/slang-file-system.h"
#include "../core/slang-type-text-util.h"
#include "../core/slang-hex-dump-util.h"

#include "../compiler-core/slang-command-line-args.h"
#include "../compiler-core/slang-artifact-info.h"
#include "../compiler-core/slang-core-diagnostics.h"

#include <assert.h>

namespace Slang {

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

        for (int i = 0; i < SLANG_COUNT_OF(entries); ++i)
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

            { ".c",    SLANG_SOURCE_LANGUAGE_C,     SLANG_STAGE_NONE },
            { ".cpp",  SLANG_SOURCE_LANGUAGE_CPP,   SLANG_STAGE_NONE },
            { ".cu",   SLANG_SOURCE_LANGUAGE_CUDA,  SLANG_STAGE_NONE }

        };

        for (int i = 0; i < SLANG_COUNT_OF(entries); ++i)
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

        if (ext == "slang-module" || ext == "slang-lib")
        {
            compileRequest->setOutputContainerFormat(SLANG_CONTAINER_FORMAT_SLANG_MODULE);
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

    static char const* getHelpText()
    {
#ifdef _WIN32
#define EXECUTABLE_EXTENSION ".exe"
#else
#define EXECUTABLE_EXTENSION ""
#endif

        return
            "Usage: slangc" EXECUTABLE_EXTENSION " [options...] [--] <input files>\n"
            "\n"
            "General options:\n"
            "\n"
            "  -D<name>[=<value>], -D <name>[=<value>]: Insert a preprocessor macro.\n"
            "  -depfile <path>: Save the source file dependency list in a file.\n"
            "  -entry <name>: Specify the name of an entry-point function.\n"
            "    Multiple -entry options may be used in a single invocation.\n"
            "    If no -entry options are given, compiler will use [shader(...)]\n"
            "    attributes to detect entry points.\n"
            "  -h, -help, --help: Print this message.\n"
            "  -I<path>, -I <path>: Add a path to be used in resolving '#include'\n"
            "    and 'import' operations.\n"
            "  -lang <language>: Set the language for the following input files.\n"
            "    Accepted languages are:\n"
            "      c, cpp, c++, cxx, slang, glsl, hlsl, cu, cuda\n"
            "  -matrix-layout-column-major: Set the default matrix layout to column-major.\n"
            "  -matrix-layout-row-major: Set the default matrix layout to row-major.\n"
            "  -module-name <name>: Set the module name to use when compiling multiple\n"
            "    .slang source files into a single module.\n"
            "  -o <path>: Specify a path where generated output should be written.\n"
            "    If no -target or -stage is specified, one may be inferred\n"
            "    from file extension (see File Extensions).\n"
            "    If multiple -target options and a single -entry are present, each -o\n"
            "    associates with the first -target to its left.\n"
            "    Otherwise, if multiple -entry options are present, each -o associates\n"
            "    with the first -entry to its left, and with the -target that matches\n"
            "    the one inferred from <path>.\n"
            "  -profile <profile>[+<capability>...]: Specify the shader profile for code\n"
            "    generation.\n"
            "    Accepted profiles are:\n"
            "      sm_{4_0,4_1,5_0,5_1,6_0,6_1,6_2,6_3,6_4,6_5,6_6}\n"
            "      glsl_{110,120,130,140,150,330,400,410,420,430,440,450,460}\n"
            "    Additional profiles that include -stage information:\n"
            "      {vs,hs,ds,gs,ps}_<version>\n"
            "    See -capability for information on <capability>\n"
            "    When multiple -target options are present, each -profile associates\n"
            "    with the first -target to its left.\n"
            "  -stage <name>: Specify the stage of an entry-point function.\n"
            "    Accepted stages are:\n"
            "      vertex, hull, domain, geometry, fragment, compute,\n"
            "      raygeneration, intersection, anyhit, closesthit, miss, callable\n"
            "    When multiple -entry options are present, each -stage associated with\n"
            "    the first -entry to its left.\n"
            "    May be omitted if entry-point function has a [shader(...)] attribute;\n"
            "    otherwise required for each -entry option.\n"
            "  -target <format>: Specifies the format in which code should be generated.\n"
            "    Accepted formats are:\n"
            "      glsl, hlsl, spirv, spirv-assembly, dxbc,\n"
            "      dxbc-assembly, dxil, dxil-assembly\n"
            "  -v, -version: Display the build version.\n"
            "  -warnings-as-errors all: Treat all warnings as errors.\n"
            "  -warnings-as-errors <id>[,<id>...]: Treat specific warning ids as errors.\n"
            "  -warnings-disable <id>[,<id>...]: Disable specific warning ids.\n"
            "  --: Treat the rest of the command line as input files.\n"
            "\n"
            "Target code generation options:\n"
            "\n"
            "  -capability <capability>[+<capability>...]: Add optional capabilities\n"
            "    to a code generation target. See Capabilities below.\n"
            "  -default-image-format-unknown: Set the format of R/W images with unspecified\n"
            "    format to 'unknown'. Otherwise try to guess the format.\n"
            "  -disable-dynamic-dispatch: Disables generating dynamic dispatch code.\n"
            "  -disable-specialization: Disables generics and specialization pass.\n"
            "  -fp-mode <mode>, -floating-point-mode <mode>: Set the floating point mode.\n"
            "    Accepted modes are:\n"
            "      precise : Disable optimization that could change the output of floating-\n"
            "        point computations, including around infinities, NaNs, denormalized\n"
            "        values, and negative zero. Prefer the most precise versions of special\n"
            "        functions supported by the target.\n"
            "      fast : Allow optimizations that may change results of floating-point\n"
            "        computations. Prefer the fastest version of special functions supported\n"
            "        by the target.\n"
            "  -g, -g<N>: Include debug information in the generated code, where possible.\n"
            "    N is the amount of information, 0..3, unspecified means 2\n"
            "  -line-directive-mode <mode>: Sets how the `#line` directives should be\n"
            "      produced. Available options are:\n"
            "        none : Don't emit `#line` directives at all\n"
            "      If not specified, default behavior is to use C-style `#line` directives\n"
            "      for HLSL and C/C++ output, and traditional GLSL-style `#line` directives\n"
            "      for GLSL output.\n"
            "  -O<N>: Set the optimization level.\n"
            "    N is the amount of optimization, 0..3, default is 1\n"
            "  -obfuscate: Remove all source file information from outputs.\n"
            "\n"
            "Downstream compiler options:\n"
            "\n"
            "  -<compiler>-path: Specify path to a downstream <compiler>\n"
            "    executable or library. Accepted compilers are:\n"
            "      fxc (d3dcompiler_47.dll)\n"
            "      dxc (dxcompiler.*)\n"
            "      glslang (slang-glslang.*)\n"
            "      vs = visualstudio (cl.exe)\n"
            "      clang\n"
            "      gcc (g++)\n"
            "      c = cpp = genericcpp\n"
            "      nvrtc\n"
            "      llvm\n"
            "  -default-downstream-compiler <language> <compiler>: Set a default compiler\n"
            "      for the given language. See -lang for the list of languages.\n"
            "  -X<compiler> <option>: Pass arguments to downstream <compiler>.\n"
            "\n"
            "Compiler debugging/instrumentation options:\n"
            "\n"
            "  -dump-ast: Dump the AST to a .slang-ast file next to the input.\n"
            "  -dump-intermediate-prefix <prefix>: File name prefix for -dump-intermediates \n"
            "      outputs, default is 'slang-dump-'\n"
            "  -dump-intermediates: Dump intermediate outputs for debugging.\n"
            "  -dump-ir: Dump the IR for debugging.\n"
            "  -dump-ir-ids: Dump the IDs with -dump-ir (debug builds only)\n"
            "  -dump-repro: Dump a `.slang-repro` file that can be used to reproduce\n"
            "    a compilation on another machine.\n"
            "  -dump-repro-on-error: Dump `.slang-repro` file on any compilation error.\n"
            "  -E, -output-preprocessor: Output the preprocessing result and exit.\n"
            "  -extract-repro <name>: Extract the repro files into a folder.\n"
            "  -load-repro <name>\n"
            "  -load-repro-directory <path>\n"
            "  -no-codegen: Skip the code generation step, just check the code and\n"
            "      generate layout.\n"
            "  -output-includes: Print the hierarchy of the processed source files.\n"
            "  -pass-through <name>: Pass the input through mostly unmodified to the \n"
            "      existing compiler <name>. Accepted compilers are:\n"
            "      fxc, glslang, dxc\n"
            "  -repro-file-system <name>\n"
            "  -serial-ir: Serialize the IR between front-end and back-end.\n"
            "  -skip-codegen: Skip the code generation phase.\n"
            "  -validate-ir: Validate the IR between the phases.\n"
            "  -verbose-paths: Display more detailed paths in diagnostic output.\n"
            "  -verify-debug-serial-ir: Verify IR in the front-end.\n"
            "\n"
            "Experimental options (use at your own risk):\n"
            "\n"
            "  -emit-spirv-directly: Generate SPIR-V output directly (otherwise through \n"
            "      GLSL and using the glslang compiler)\n"
            "  -file-system <fs>: Set the filesystem hook to use for a compile request.\n"
            "    Accepted file systems:\n"
            "      default, load-file, os\n"
            "  -heterogeneous: Output heterogeneity-related code.\n"
            "  -no-mangle: Do as little mangling of names as possible.\n"
            "\n"
            "Internal-use options (use at your own risk):\n"
            "\n"
            "  -archive-type <type>: Set the archive type for -save-stdlib. Default is zip.\n"
            "    Accepted archive types:\n"
            "      zip, riff, riff-deflate, riff-lz4\n"
            "  -compile-stdlib: Compile the StdLib from embedded sources.\n"
            "      Will return a failure if there is already a StdLib available.\n"
            "  -doc: Write documentation for -compile-stdlib\n"
            "  -ir-compression <type>: Set compression for IR and AST outputs.\n"
            "      Accepted compression types:\n"
            "      none, lite\n"
            "  -load-stdlib <filename>: Load the StdLib from file.\n"
            "  -r <name>: reference module <name>\n"
            "  -save-stdlib <filename>: Save the StdLib modules to an archive file.\n"
            "  -save-stdlib-bin-source <filename>: Same as -save-stdlib but output\n"
            "      the data as a C array.\n"
            "  -track-liveness: Enable liveness tracking. Places SLANG_LIVE_START, and SLANG_LIVE_END in output source to indicate value liveness.\n"
            "\n"
            "Deprecated options (allowed but ignored; may be removed in future):\n"
            "\n"
            "  -parameter-blocks-use-register-spaces\n"
            "\n"
            "File Extensions:\n"
            "\n"
            "  A <language>, <format>, and/or <stage> may be inferred from the\n"
            "  extension of an input or -o path:\n"
            "\n"
            "    extension           | language/format | stage\n"
            "    ====================|=================|======\n"
            "      .hlsl, .fx        -> hlsl\n"
            "      .dxbc             -> dxbc\n"
            "      .dxbc.asm         -> dxbc-assembly\n"
            "      .dxil             -> dxil\n"
            "      .dxil.asm         -> dxil-assembly\n"
            "      .glsl             -> glsl\n"
            "      .vert             -> glsl            vertex\n"
            "      .frag             -> glsl            fragment\n"
            "      .geom             -> glsl            geoemtry\n"
            "      .tesc             -> glsl            hull\n"
            "      .tese             -> glsl            domain\n"
            "      .comp             -> glsl            compute\n"
            "      .slang            -> slang\n"
            "      .spv              -> spirv\n"
            "      .spv.asm          -> spirv-assembly\n"
            "      .c                -> c\n"
            "      .cpp, .c++, .cxx  -> c++\n"
            "      .exe              -> executable\n"
            "      .dll, .so         -> sharedlibrary\n"
            "      .cu               -> cuda\n"
            "      .ptx              -> ptx\n"
            "      .obj, .o          -> object-code\n"
            "\n"
            "Capabilities:\n"
            "\n"
            "  A capability describes an optional feature that a target may or\n"
            "  may not support. When a -capability is specified, the compiler\n"
            "  may assume that the target supports that capability, and generate\n"
            "  code accordingly.\n"
            "  Currently defined capabilities are:\n"
            "\n"
            "    spriv_1_{0,1,2,3,4,5}   - minimum supported SPIR-V version\n"
            "    GL_NV_ray_tracing       - enables the GL_NV_ray_tracing extension\n"
            "    GL_EXT_ray_tracing      - enables the GL_EXT_ray_tracing extension\n"
            "\n";

#undef EXECUTABLE_EXTENSION
    }

    SlangResult overrideDiagnosticSeverity(String const& identifierList, Severity overrideSeverity)
    {
        List<UnownedStringSlice> slices;
        StringUtil::split(identifierList.getUnownedSlice(), ',', slices);
        Index sliceCount = slices.getCount();

        for (Index i = 0; i < sliceCount; ++i)
        {
            UnownedStringSlice warningIdentifier = slices[i];

            Int warningIndex = -1;
            SLANG_RETURN_ON_FAIL(StringUtil::parseInt(warningIdentifier, warningIndex));

            requestImpl->getSink()->overrideDiagnosticSeverity(int(warningIndex), overrideSeverity);
        }

        return SLANG_OK;
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

        while (reader.hasArg())
        {
            auto arg = reader.getArgAndAdvance();
            const auto& argValue = arg.value;

            if (argValue[0] == '-')
            {
                if(argValue == "-no-mangle" )
                {
                    flags |= SLANG_COMPILE_FLAG_NO_MANGLING;
                }
                else if (argValue == "-load-stdlib")
                {
                    CommandLineArg fileName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(fileName));

                    // Load the file
                    ScopedAllocation contents;
                    SLANG_RETURN_ON_FAIL(File::readAllBytes(fileName.value, contents));
                    SLANG_RETURN_ON_FAIL(session->loadStdLib(contents.getData(), contents.getSizeInBytes()));
                }
                else if (argValue == "-compile-stdlib")
                {
                    compileStdLib = true;
                }
                else if (argValue == "-archive-type")
                {
                    CommandLineArg archiveTypeName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(archiveTypeName));

                    archiveType = TypeTextUtil::findArchiveType(archiveTypeName.value.getUnownedSlice());
                    if (archiveType == SLANG_ARCHIVE_TYPE_UNDEFINED)
                    {
                        sink->diagnose(archiveTypeName.loc, Diagnostics::unknownArchiveType, archiveTypeName.value);
                        return SLANG_FAIL;
                    }
                }
                else if (argValue == "-save-stdlib")
                {
                    CommandLineArg fileName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(fileName));

                    ComPtr<ISlangBlob> blob;

                    SLANG_RETURN_ON_FAIL(session->saveStdLib(archiveType, blob.writeRef()));
                    SLANG_RETURN_ON_FAIL(File::writeAllBytes(fileName.value, blob->getBufferPointer(), blob->getBufferSize()));
                }
                else if (argValue == "-save-stdlib-bin-source")
                {
                    CommandLineArg fileName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(fileName));

                    ComPtr<ISlangBlob> blob;

                    SLANG_RETURN_ON_FAIL(session->saveStdLib(archiveType, blob.writeRef()));

                    StringBuilder builder;
                    StringWriter writer(&builder, 0);

                    SLANG_RETURN_ON_FAIL(HexDumpUtil::dumpSourceBytes((const uint8_t*)blob->getBufferPointer(), blob->getBufferSize(), 16, &writer));

                    File::writeAllText(fileName.value, builder);
                }
                else if (argValue == "-no-codegen")
                {
                    flags |= SLANG_COMPILE_FLAG_NO_CODEGEN;
                }
                else if (argValue == "-dump-intermediates")
                {
                    compileRequest->setDumpIntermediates(true);
                }
                else if (argValue == "-dump-ir-ids")
                {
                    requestImpl->getFrontEndReq()->m_irDumpOptions.flags |= IRDumpOptions::Flag::DumpDebugIds;
                }
                else if (argValue == "-dump-intermediate-prefix")
                {
                    CommandLineArg prefix;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(prefix));
                    requestImpl->m_dumpIntermediatePrefix = prefix.value;
                }
                else if (argValue == "-output-includes")
                {
                    requestImpl->getFrontEndReq()->outputIncludes = true;
                }
                else if(argValue == "-dump-ir" )
                {
                    requestImpl->getFrontEndReq()->shouldDumpIR = true;
                }
                else if (argValue == "-E" || argValue == "-output-preprocessor")
                {
                    requestImpl->getFrontEndReq()->outputPreprocessor = true;
                }
                else if (argValue == "-dump-ast")
                {
                    requestImpl->getFrontEndReq()->shouldDumpAST = true;
                }
                else if (argValue == "-doc")
                {
                    // If compiling stdlib is enabled, will write out documentation
                    compileStdLibFlags |= slang::CompileStdLibFlag::WriteDocumentation;

                    // Enable writing out documentation on the req
                    requestImpl->getFrontEndReq()->shouldDocument = true;
                }
                else if (argValue == "-dump-repro")
                {
                    CommandLineArg dumpRepro;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(dumpRepro));
                    requestImpl->m_dumpRepro = dumpRepro.value;
                    compileRequest->enableReproCapture();
                }
                else if (argValue == "-dump-repro-on-error")
                {
                    requestImpl->m_dumpReproOnError = true;
                }
                else if (argValue == "-extract-repro")
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
                }
                else if (argValue == "-module-name")
                {
                    CommandLineArg moduleName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(moduleName));

                    compileRequest->setDefaultModuleName(moduleName.value.getBuffer());
                }
                else if(argValue == "-load-repro")
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
                }
                else if (argValue == "-load-repro-directory")
                {
                    CommandLineArg reproDirectory;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(reproDirectory));

                    SLANG_RETURN_ON_FAIL(_compileReproDirectory(session, requestImpl, reproDirectory.value, sink));
                }
                else if (argValue == "-repro-file-system")
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

                    RefPtr<CacheFileSystem> cacheFileSystem;
                    SLANG_RETURN_ON_FAIL(ReproUtil::loadFileSystem(base, requestState, dirFileSystem, cacheFileSystem));

                    // I might want to make the dir file system the fallback file system...
                    cacheFileSystem->setInnerFileSystem(dirFileSystem, cacheFileSystem->getUniqueIdentityMode(), cacheFileSystem->getPathStyle());

                    // Set as the file system
                    compileRequest->setFileSystem(cacheFileSystem);
                }
                else if (argValue == "-serial-ir")
                {
                    requestImpl->getFrontEndReq()->useSerialIRBottleneck = true;
                }
                else if (argValue == "-disable-specialization")
                {
                    requestImpl->disableSpecialization = true;
                }
                else if (argValue == "-disable-dynamic-dispatch")
                {
                    requestImpl->disableDynamicDispatch = true;
                }
                else if (argValue == "-track-liveness")
                {
                    requestImpl->setTrackLiveness(true);
                }
                else if (argValue == "-verbose-paths")
                {
                    requestImpl->getSink()->setFlag(DiagnosticSink::Flag::VerbosePath);
                }
                else if (argValue == "-warnings-as-errors")
                {
                    CommandLineArg operand;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(operand));

                    if (operand.value == "all")
                        requestImpl->getSink()->setFlag(DiagnosticSink::Flag::TreatWarningsAsErrors);
                    else
                    {
                        if (SLANG_FAILED(overrideDiagnosticSeverity(operand.value, Severity::Error)))
                        {
                            sink->diagnose(operand.loc, MiscDiagnostics::invalidArgumentForOption, "-warnings-as-errors");
                            return SLANG_FAIL;
                        }
                    }
                }
                else if (argValue == "-warnings-disable")
                {
                    CommandLineArg operand;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(operand));

                    if (SLANG_FAILED(overrideDiagnosticSeverity(operand.value, Severity::Disable)))
                    {
                        sink->diagnose(operand.loc, MiscDiagnostics::invalidArgumentForOption, "-warnings-disable");
                        return SLANG_FAIL;
                    }
                }
                else if (argValue == "-verify-debug-serial-ir")
                {
                    requestImpl->getFrontEndReq()->verifyDebugSerialization = true;
                }
                else if(argValue == "-validate-ir" )
                {
                    requestImpl->getFrontEndReq()->shouldValidateIR = true;
                }
                else if(argValue == "-skip-codegen" )
                {
                    requestImpl->m_shouldSkipCodegen = true;
                }
                else if(argValue == "-parameter-blocks-use-register-spaces" )
                {
                    getCurrentTarget()->targetFlags |= SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES;
                }
                else if (argValue == "-ir-compression")
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    SLANG_RETURN_ON_FAIL(SerialParseUtil::parseCompressionType(name.value.getUnownedSlice(), requestImpl->getLinkage()->serialCompressionType));
                }
                else if (argValue == "-target")
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
                }
                // A "profile" can specify both a general capability level for
                // a target, and also (as a legacy/compatibility feature) a
                // specific stage to use for an entry point.
                else if (argValue == "-profile")
                {
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

                    SlangProfileID profileID = Slang::Profile::lookUp(profileName).raw;
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
                }
                else if( argValue == "-capability" )
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
                }
                else if (argValue == "-stage")
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
                }
                else if (argValue == "-entry")
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    RawEntryPoint rawEntryPoint;
                    rawEntryPoint.name = name.value;
                    rawEntryPoint.translationUnitIndex = currentTranslationUnitIndex;

                    rawEntryPoints.add(rawEntryPoint);
                }
                else if (argValue == "-lang")
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
                }
                else if (argValue == "-pass-through")
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
                }
                else if (argValue.getLength() >= 2 && argValue[1] == 'D')
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
                }
                else if (argValue.getLength() >= 2 && argValue[1] == 'I')
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
                }
                //
                // A `-o` option is used to specify a desired output file.
                else if (argValue == "-o")
                {
                    CommandLineArg outputPath;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(outputPath));

                    addOutputPath(outputPath.value.getBuffer());
                }
                // A -depfile option is used to specify the file name where the dependency lists will be written
                else if (argValue == "-depfile")
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
                }
                else if(argValue == "-matrix-layout-row-major")
                {
                    defaultMatrixLayoutMode = kMatrixLayoutMode_RowMajor;
                }
                else if(argValue == "-matrix-layout-column-major")
                {
                    defaultMatrixLayoutMode = kMatrixLayoutMode_ColumnMajor;
                }
                else if(argValue == "-line-directive-mode")
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    SlangLineDirectiveMode mode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
                    if(name.value == "none")
                    {
                        mode = SLANG_LINE_DIRECTIVE_MODE_NONE;
                    }
                    else
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownLineDirectiveMode, name.value);
                        return SLANG_FAIL;
                    }

                    compileRequest->setLineDirectiveMode(mode);

                }
                else if( argValue == "-fp-mode" || argValue == "-floating-point-mode" )
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    FloatingPointMode mode = FloatingPointMode::Default;
                    if(name.value == "fast")
                    {
                        mode = FloatingPointMode::Fast;
                    }
                    else if(name.value == "precise")
                    {
                        mode = FloatingPointMode::Precise;
                    }
                    else
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownFloatingPointMode, name.value);
                        return SLANG_FAIL;
                    }

                    setFloatingPointMode(getCurrentTarget(), mode);
                }
                else if( argValue.getLength() >= 2 && argValue[1] == 'O' )
                {
                    UnownedStringSlice levelSlice = argValue.getUnownedSlice().tail(2);
                    SlangOptimizationLevel level = SLANG_OPTIMIZATION_LEVEL_DEFAULT;

                    const char c = levelSlice.getLength() == 1 ? levelSlice[0] : 0;

                    switch (c)
                    {
                        case '0':   level = SLANG_OPTIMIZATION_LEVEL_NONE;      break;
                        case '1':   level = SLANG_OPTIMIZATION_LEVEL_DEFAULT;   break;
                        case '2':   level = SLANG_OPTIMIZATION_LEVEL_HIGH;      break;
                        case '3':   level = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;   break;
                        default:
                        {
                            sink->diagnose(arg.loc, Diagnostics::unknownOptimiziationLevel, arg.value);
                            return SLANG_FAIL;
                        }
                    }
                 
                    compileRequest->setOptimizationLevel(level);
                }

                // Note: unlike with `-O` above, we have to consider that other
                // options might have names that start with `-g` and so cannot
                // just detect it as a prefix.
                else if( argValue == "-g" || argValue == "-g2" )
                {
                    compileRequest->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_STANDARD);
                }
                else if( argValue == "-g0" )
                {
                    compileRequest->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_NONE);
                }
                else if( argValue == "-g1" )
                {
                    compileRequest->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_MINIMAL);
                }
                else if( argValue == "-g3" )
                {
                    compileRequest->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
                }
                else if( argValue == "-default-image-format-unknown" )
                {
                    requestImpl->useUnknownImageFormatAsDefault = true;
                }
                else if (argValue == "-obfuscate")
                {
                    requestImpl->getLinkage()->m_obfuscateCode = true;
                }
                else if (argValue == "-file-system")
                {
                    CommandLineArg name;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(name));

                    if (name.value == "default")
                    {
                        compileRequest->setFileSystem(nullptr);
                    }
                    else if (name.value == "load-file")
                    {
                        // 'Simple' just implements loadFile interface, so will be wrapped with CacheFileSystem internally
                        compileRequest->setFileSystem(OSFileSystem::getLoadSingleton());
                    }
                    else if (name.value == "os")
                    {
                        // 'Immutable' implements the ISlangFileSystemExt interface - and will be used directly
                        compileRequest->setFileSystem(OSFileSystem::getExtSingleton());
                    }
                    else
                    {
                        sink->diagnose(name.loc, Diagnostics::unknownFileSystemOption, name.value);
                        return SLANG_FAIL;
                    }
                }
                else if (argValue == "-r")
                {
                    CommandLineArg referenceModuleName;
                    SLANG_RETURN_ON_FAIL(reader.expectArg(referenceModuleName));

                    const auto path = referenceModuleName.value;

                    auto desc = ArtifactInfoUtil::getDescFromPath(path.getUnownedSlice());

                    if (desc.kind == ArtifactKind::Unknown)
                    {
                        sink->diagnose(referenceModuleName.loc, Diagnostics::unknownLibraryKind, Path::getPathExt(path));
                        return SLANG_FAIL;
                    }

                    // If it's a GPU binary, then we'll assume it's a library
                    if (ArtifactInfoUtil::isGpuBinary(desc))
                    {
                        desc.kind = ArtifactKind::Library;
                    }

                    if (!ArtifactInfoUtil::isBinaryLinkable(desc))
                    {
                        sink->diagnose(referenceModuleName.loc, Diagnostics::kindNotLinkable, Path::getPathExt(path));
                        return SLANG_FAIL;
                    }

                    const String name = ArtifactInfoUtil::getBaseNameFromPath(desc, path.getUnownedSlice());

                    // Create the artifact
                    ComPtr<IArtifact> artifact(new Artifact(desc, name)); 

                    // There is a problem here if I want to reference a library that is a 'system' library or is not directly a file
                    // In that case the path shouldn't be set and the name should completely define the library.
                    // Seeing as on all targets the baseName doesn't have an extension, and all library types do
                    // if the name doesn't have an extension we can assume there is no path to it.
                    
                    if (Path::getPathExt(path).getLength() > 0)
                    {
                        // Set the path
                        artifact->setPath(Artifact::PathType::Existing, path.getBuffer());
                    }

                    // TODO(JS): We might want to check if the artifact exists.
                    // If the artifact is a CPU (or downstream compiler) library
                    // it may be findable by some other mechanism, so we probably don't want to check existance and just let the
                    // downstream compiler handle.
                    
                    SLANG_RETURN_ON_FAIL(_addLibraryReference(requestImpl, artifact));
                }
                else if (argValue == "-v" || argValue == "-version")
                {
                    sink->diagnoseRaw(Severity::Note, session->getBuildTagString());
                }
                else if (argValue == "-h" || argValue == "-help" || argValue == "--help")
                {
                    sink->diagnoseRaw(Severity::Note, getHelpText());
                    return SLANG_FAIL;
                }
                else if( argValue == "-emit-spirv-directly" )
                {
                    getCurrentTarget()->targetFlags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
                }
                else if (argValue == "-default-downstream-compiler")
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
                }       
                else if (argValue == "--")
                {
                    // The `--` option causes us to stop trying to parse options,
                    // and treat the rest of the command line as input file names:
                    while (reader.hasArg())
                    {
                        SLANG_RETURN_ON_FAIL(addInputPath(reader.getValueAndAdvance().getBuffer()));
                    }
                    break;
                }
                else
                {
                    if (argValue.endsWith("-path"))
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
                    }

                    sink->diagnose(arg.loc, Diagnostics::unknownCommandLineOption, argValue);
                    // TODO: print a usage message
                    return SLANG_FAIL;
                }
            }
            else
            {
                SLANG_RETURN_ON_FAIL(addInputPath(argValue.getBuffer()));
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
            for(auto& rawOutput : rawOutputs)
            {
                // Some outputs don't imply a target format, and we shouldn't use those for inference.
                auto impliedFormat = rawOutput.impliedFormat;
                if( impliedFormat == CodeGenTarget::Unknown )
                    continue;

                int targetIndex = 0;
                if( !mapFormatToTargetIndex.TryGetValue(impliedFormat, targetIndex) )
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

                if( mapFormatToTargetIndex.ContainsKey(format) )
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
                compileRequest->setTargetProfile(targetID, Profile(rawTarget.profileVersion).raw);
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
        // and output type is callable, add an empty' rawOutput.
        if (rawOutputs.getCount() == 0 && 
            rawTargets.getCount() == 1 && 
            ArtifactDesc::makeFromCompileTarget(asExternal(rawTargets[0].format)).kind == ArtifactKind::Callable)
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
                else if( mapFormatToTargetIndex.TryGetValue(rawOutput.impliedFormat, targetIndex) )
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
            if( !requestImpl->m_targetInfos.TryGetValue(target, targetInfo) )
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
                if (targetInfo->entryPointOutputPaths.ContainsKey(entryPointID))
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
        compileRequest->m_diagnosticOutput = sink->outputBuffer.ProduceString();
    }

    return res;
}


} // namespace Slang


