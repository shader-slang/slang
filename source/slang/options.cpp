// options.cpp

// Implementation of options parsing for `slangc` command line,
// and also for API interface that takes command-line argument strings.

#include "../../slang.h"

#include "compiler.h"
#include "profile.h"

#include <assert.h>

namespace Slang {

char const* tryReadCommandLineArgumentRaw(char const* option, char const* const**ioCursor, char const* const*end)
{
    char const* const*& cursor = *ioCursor;
    if (cursor == end)
    {
        fprintf(stderr, "expected an argument for command-line option '%s'", option);
        exit(1);
    }
    else
    {
        return *cursor++;
    }
}

String tryReadCommandLineArgument(char const* option, char const* const**ioCursor, char const* const*end)
{
    return String(tryReadCommandLineArgumentRaw(option, ioCursor, end));
}





struct OptionsParser
{
    SlangSession*           session = nullptr;
    SlangCompileRequest*    compileRequest = nullptr;

    Slang::CompileRequest*  requestImpl = nullptr;

    struct RawTranslationUnit
    {
        SlangSourceLanguage sourceLanguage;
        SlangProfileID      implicitProfile;
        int                 translationUnitIndex;
    };

    // Collect translation units so that we can futz with them later
    List<RawTranslationUnit> rawTranslationUnits;

    struct RawEntryPoint
    {
        String          name;
        SlangProfileID  profileID = SLANG_PROFILE_UNKNOWN;
        int             translationUnitIndex = -1;
        int             outputPathIndex = -1;
    };

    // Collect entry point names, so that we can associate them
    // with entry points later...
    List<RawEntryPoint> rawEntryPoints;

    // The number of input files that have been specified
    int inputPathCount = 0;

    // If we already have a translation unit for Slang code, then this will give its index.
    // If not, it will be `-1`.
    int slangTranslationUnit = -1;

    int translationUnitCount = 0;
    int currentTranslationUnitIndex = -1;

    SlangProfileID currentProfileID = SLANG_PROFILE_UNKNOWN;

    // How many times were `-profile` options given?
    int profileOptionCount = 0;

    SlangCompileFlags flags = 0;

    struct RawOutputPath
    {
        String              path;
        SlangCompileTarget  target;
    };

    List<RawOutputPath> rawOutputPaths;

    SlangCompileTarget chosenTarget = SLANG_TARGET_NONE;

    int addTranslationUnit(
        SlangSourceLanguage language,
        SlangProfileID      implicitProfile = SLANG_PROFILE_UNKNOWN)
    {
        auto translationUnitIndex = spAddTranslationUnit(compileRequest, language, nullptr);

        SLANG_RELEASE_ASSERT(UInt(translationUnitIndex) == rawTranslationUnits.Count());

        RawTranslationUnit rawTranslationUnit;
        rawTranslationUnit.sourceLanguage = language;
        rawTranslationUnit.implicitProfile = implicitProfile;
        rawTranslationUnit.translationUnitIndex = translationUnitIndex;

        rawTranslationUnits.Add(rawTranslationUnit);

        return translationUnitIndex;
    }

    void addInputSlangPath(
        String const& path)
    {
        // All of the input .slang files will be grouped into a single logical translation unit,
        // which we create lazily when the first .slang file is encountered.
        if( slangTranslationUnit == -1 )
        {
            translationUnitCount++;
            slangTranslationUnit = addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG);
        }

        spAddTranslationUnitSourceFile(
            compileRequest,
            slangTranslationUnit,
            path.begin());

        // Set the translation unit to be used by subsequent entry points
        currentTranslationUnitIndex = slangTranslationUnit;
    }

    void addInputForeignShaderPath(
        String const&           path,
        SlangSourceLanguage     language,
        SlangProfileID          implicitProfile = SLANG_PROFILE_UNKNOWN)
    {
        translationUnitCount++;
        currentTranslationUnitIndex = addTranslationUnit(language, implicitProfile);

        spAddTranslationUnitSourceFile(
            compileRequest,
            currentTranslationUnitIndex,
            path.begin());
    }

    void addInputPath(
        char const*  inPath)
    {
        inputPathCount++;

        // look at the extension on the file name to determine
        // how we should handle it.
        String path = String(inPath);

        if( path.EndsWith(".slang") )
        {
            // Plain old slang code
            addInputSlangPath(path);
        }
#define CASE(EXT, LANG) \
        else if(path.EndsWith(EXT)) do { addInputForeignShaderPath(path, SLANG_SOURCE_LANGUAGE_##LANG); } while(0)

        CASE(".hlsl", HLSL);
        CASE(".fx",   HLSL);

        CASE(".glsl", GLSL);
#undef CASE

#define CASE(EXT, LANG, PROFILE) \
        else if(path.EndsWith(EXT)) do { addInputForeignShaderPath(path, SLANG_SOURCE_LANGUAGE_##LANG, SlangProfileID(Slang::Profile::PROFILE)); } while(0)
        // TODO: need a way to pass along stage/profile and entry-point info for these cases...
        CASE(".vert", GLSL, GLSL_Vertex);
        CASE(".frag", GLSL, GLSL_Fragment);
        CASE(".geom", GLSL, GLSL_Geometry);
        CASE(".tesc", GLSL, GLSL_TessControl);
        CASE(".tese", GLSL, GLSL_TessEval);
        CASE(".comp", GLSL, GLSL_Compute);

#undef CASE

        else
        {
            fprintf(stderr, "error: can't deduce language for input file '%s'\n", inPath);
            exit(1);
        }
    }

    void addOutputPath(
        String const&       path,
        SlangCompileTarget  target)
    {
        RawOutputPath rawOutputPath;
        rawOutputPath.path = path;
        rawOutputPath.target = target;

        rawOutputPaths.Add(rawOutputPath);
    }

    void addOutputPath(char const* inPath)
    {
        String path = String(inPath);

        if (!inPath) {}
#define CASE(EXT, TARGET)   \
        else if(path.EndsWith(EXT)) do { addOutputPath(path, SLANG_##TARGET); } while(0)

        CASE(".hlsl", HLSL);
        CASE(".fx",   HLSL);

        CASE(".dxbc", DXBC);
        CASE(".dxbc.asm", DXBC_ASM);

        CASE(".glsl", GLSL);
        CASE(".vert", GLSL);
        CASE(".frag", GLSL);
        CASE(".geom", GLSL);
        CASE(".tesc", GLSL);
        CASE(".tese", GLSL);
        CASE(".comp", GLSL);

        CASE(".spv",        SPIRV);
        CASE(".spv.asm",    SPIRV_ASM);

#undef CASE

        else
        {
            // Allow an unknown-format `-o`, assuming we get a target format
            // from another argument.
            addOutputPath(path, SLANG_TARGET_UNKNOWN);
        }
    }

    int parse(
        int             argc,
        char const* const*  argv)
    {
        char const* const* argCursor = &argv[0];
        char const* const* argEnd = &argv[argc];
        while (argCursor != argEnd)
        {
            char const* arg = *argCursor++;
            if (arg[0] == '-')
            {
                String argStr = String(arg);

                // The argument looks like an option, so try to parse it.
//                if (argStr == "-outdir")
//                    outputDir = tryReadCommandLineArgument(arg, &argCursor, argEnd);
//                if (argStr == "-out")
//                    options.outputName = tryReadCommandLineArgument(arg, &argCursor, argEnd);
//                else if (argStr == "-symbo")
//                    options.SymbolToCompile = tryReadCommandLineArgument(arg, &argCursor, argEnd);
                //else
                if (argStr == "-no-checking")
                    flags |= SLANG_COMPILE_FLAG_NO_CHECKING;
                else if(argStr == "-split-mixed-types" )
                {
                    flags |= SLANG_COMPILE_FLAG_SPLIT_MIXED_TYPES;
                }
                else if(argStr == "-use-ir" )
                {
                    flags |= SLANG_COMPILE_FLAG_USE_IR;
                }
                else if(argStr == "-no-mangle" )
                {
                    flags |= SLANG_COMPILE_FLAG_NO_MANGLING;
                }
                else if(argStr == "-dump-ir" )
                {
                    requestImpl->shouldDumpIR = true;
                }
                else if(argStr == "-skip-codegen" )
                {
                    requestImpl->shouldSkipCodegen = true;
                }
                else if (argStr == "-backend" || argStr == "-target")
                {
                    String name = tryReadCommandLineArgument(arg, &argCursor, argEnd);
                    SlangCompileTarget target = SLANG_TARGET_UNKNOWN;

                    if (name == "glsl")
                    {
                        target = SLANG_GLSL;
                    }
                    else if (name == "glsl_vk")
                    {
                        target = SLANG_GLSL_VULKAN;
                    }
//                    else if (name == "glsl_vk_onedesc")
//                    {
//                        options.Target = CodeGenTarget::GLSL_Vulkan_OneDesc;
//                    }
                    else if (name == "hlsl")
                    {
                        target = SLANG_HLSL;
                    }
                    else if (name == "spriv")
                    {
                        target = SLANG_SPIRV;
                    }
                    else if (name == "dxbc")
                    {
                        target = SLANG_DXBC;
                    }
                    else if (name == "dxbc-assembly")
                    {
                        target = SLANG_DXBC_ASM;
                    }
                #define CASE(NAME, TARGET)  \
                    else if(name == #NAME) do { target = SLANG_##TARGET; } while(0)

                    CASE(spirv, SPIRV);
                    CASE(spirv-assembly, SPIRV_ASM);
                    CASE(none, TARGET_NONE);

                #undef CASE

                    else
                    {
                        fprintf(stderr, "unknown code generation target '%S'\n", name.ToWString().begin());
                        exit(1);
                    }

                    this->chosenTarget = target;
                    spSetCodeGenTarget(compileRequest, target);
                }
                // A "profile" specifies both a specific target stage and a general level
                // of capability required by the program.
                else if (argStr == "-profile")
                {
                    String name = tryReadCommandLineArgument(arg, &argCursor, argEnd);

                    SlangProfileID profileID = spFindProfile(session, name.begin());
                    if( profileID == SLANG_PROFILE_UNKNOWN )
                    {
                        fprintf(stderr, "unknown profile '%s'\n", name.begin());
                    }
                    else
                    {
                        currentProfileID = profileID;
                        profileOptionCount++;
                    }
                }
                else if (argStr == "-entry")
                {
                    String name = tryReadCommandLineArgument(arg, &argCursor, argEnd);

                    RawEntryPoint entry;
                    entry.name = name;
                    entry.translationUnitIndex = currentTranslationUnitIndex;

                    int outputPathCount = (int) rawOutputPaths.Count();
                    int currentOutputPathIndex = outputPathCount - 1;
                    entry.outputPathIndex = currentOutputPathIndex;

                    // TODO(tfoley): Allow user to fold a specification of a profile into the entry-point name,
                    // for the case where they might be compiling multiple entry points in one invocation...
                    //
                    // For now, just use the last profile set on the command-line to specify this

                    entry.profileID = currentProfileID;

                    rawEntryPoints.Add(entry);
                }
#if 0
                else if (argStr == "-stage")
                {
                    String name = tryReadCommandLineArgument(arg, &argCursor, argEnd);
                    StageTarget stage = StageTarget::Unknown;
                    if (name == "vertex") { stage = StageTarget::VertexShader; }
                    else if (name == "fragment") { stage = StageTarget::FragmentShader; }
                    else if (name == "hull") { stage = StageTarget::HullShader; }
                    else if (name == "domain") { stage = StageTarget::DomainShader; }
                    else if (name == "compute") { stage = StageTarget::ComputeShader; }
                    else
                    {
                        fprintf(stderr, "unknown stage '%S'\n", name.ToWString());
                    }
                    options.stage = stage;
                }
#endif
                else if (argStr == "-pass-through")
                {
                    String name = tryReadCommandLineArgument(arg, &argCursor, argEnd);
                    SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
                    if (name == "fxc") { passThrough = SLANG_PASS_THROUGH_FXC; }
                    else if (name == "glslang") { passThrough = SLANG_PASS_THROUGH_GLSLANG; }
                    else
                    {
                        fprintf(stderr, "unknown pass-through target '%S'\n", name.ToWString().begin());
                        exit(1);
                    }

                    spSetPassThrough(
                        compileRequest,
                        passThrough);
                }
//                else if (argStr == "-genchoice")
//                    options.Mode = CompilerMode::GenerateChoice;
                else if (argStr[1] == 'D')
                {
                    // The value to be defined might be part of the same option, as in:
                    //     -DFOO
                    // or it might come separately, as in:
                    //     -D FOO
                    char const* defineStr = arg + 2;
                    if (defineStr[0] == 0)
                    {
                        // Need to read another argument from the command line
                        defineStr = tryReadCommandLineArgumentRaw(arg, &argCursor, argEnd);
                    }
                    // The string that sets up the define can have an `=` between
                    // the name to be defined and its value, so we search for one.
                    char const* eqPos = nullptr;
                    for(char const* dd = defineStr; *dd; ++dd)
                    {
                        if (*dd == '=')
                        {
                            eqPos = dd;
                            break;
                        }
                    }

                    // Now set the preprocessor define
                    //
                    if (eqPos)
                    {
                        // If we found an `=`, we split the string...

                        spAddPreprocessorDefine(
                            compileRequest,
                            String(defineStr, eqPos).begin(),
                            String(eqPos+1).begin());
                    }
                    else
                    {
                        // If there was no `=`, then just #define it to an empty string

                        spAddPreprocessorDefine(
                            compileRequest,
                            String(defineStr).begin(),
                            "");
                    }
                }
                else if (argStr[1] == 'I')
                {
                    // The value to be defined might be part of the same option, as in:
                    //     -IFOO
                    // or it might come separately, as in:
                    //     -I FOO
                    // (see handling of `-D` above)
                    char const* includeDirStr = arg + 2;
                    if (includeDirStr[0] == 0)
                    {
                        // Need to read another argument from the command line
                        includeDirStr = tryReadCommandLineArgumentRaw(arg, &argCursor, argEnd);
                    }

                    spAddSearchPath(
                        compileRequest,
                        String(includeDirStr).begin());
                }
                //
                // A `-o` option is used to specify a desired output file.
                else if (argStr == "-o")
                {
                    char const* outputPath = tryReadCommandLineArgumentRaw(
                        arg, &argCursor, argEnd);
                    if (!outputPath) continue;

                    addOutputPath(outputPath);
                }
                else if (argStr == "--")
                {
                    // The `--` option causes us to stop trying to parse options,
                    // and treat the rest of the command line as input file names:
                    while (argCursor != argEnd)
                    {
                        addInputPath(*argCursor++);
                    }
                    break;
                }
                else
                {
                    fprintf(stderr, "unknown command-line option '%S'\n", argStr.ToWString().begin());
                    // TODO: print a usage message
                    exit(1);
                }
            }
            else
            {
                addInputPath(arg);
            }
        }

        spSetCompileFlags(compileRequest, flags);

        // TODO(tfoley): This kind of validation needs to wait until
        // after all options have been specified for API usage
#if 0
        if (inputPathCount == 0)
        {
            fprintf(stderr, "error: no input file specified\n");
            exit(1);
        }

        // No point in moving forward if there is nothing to compile
        if( translationUnitCount == 0 )
        {
            fprintf(stderr, "error: no compilation requested\n");
            exit(1);
        }
#endif

        // If the user didn't list any explicit entry points, then we can
        // try to infer one from the type of input file
        if(rawEntryPoints.Count() == 0)
        {
            for(auto rawTranslationUnit : rawTranslationUnits)
            {
                // Dont' add implicit entry points when compiling from Slang files,
                // since Slang doesn't require entry points to be named on the
                // command line.
                if(rawTranslationUnit.sourceLanguage == SLANG_SOURCE_LANGUAGE_SLANG )
                    continue;

                // Use a default entry point name
                char const* entryPointName = "main";

                // Try to determine a profile
                SlangProfileID entryPointProfile = SLANG_PROFILE_UNKNOWN;

                // If a profile was specified on the command line, then we use it
                if(currentProfileID != SLANG_PROFILE_UNKNOWN)
                {
                    entryPointProfile = currentProfileID;
                }
                // Otherwise, check if the translation unit implied a profile
                // (e.g., a `*.vert` file implies the `GLSL_Vertex` profile)
                else if(rawTranslationUnit.implicitProfile != SLANG_PROFILE_UNKNOWN)
                {
                    entryPointProfile = rawTranslationUnit.implicitProfile;
                }

                RawEntryPoint entry;
                entry.name = entryPointName;
                entry.translationUnitIndex = rawTranslationUnit.translationUnitIndex;
                entry.profileID = entryPointProfile;
                rawEntryPoints.Add(entry);
            }
        }

        // For any entry points that were given without an explicit profile, we can now apply
        // the profile that was given to them.
        if( rawEntryPoints.Count() != 0 )
        {
            bool anyEntryPointWithoutProfile = false;
            for( auto& entryPoint : rawEntryPoints )
            {
                // Skip entry points that are already associated with a translation unit...
                if( entryPoint.profileID != SLANG_PROFILE_UNKNOWN )
                    continue;

                anyEntryPointWithoutProfile = true;
                break;
            }

            // Issue an error if there are entry points without a profile,
            // and no profile was specified.
            if( anyEntryPointWithoutProfile
                && currentProfileID == SLANG_PROFILE_UNKNOWN)
            {
                fprintf(stderr, "error: no profile specified; use the '-profile <profile name>' option\n");
                exit(1);
            }
            // Issue an error if we have mulitple `-profile` options *and*
            // there were entry points that didn't get a profile, *and*
            // there we m
            if (anyEntryPointWithoutProfile
                && profileOptionCount > 1)
            {
                if (rawEntryPoints.Count() > 1)
                {
                    fprintf(stderr, "error: when multiple entry points are specified, each must have a profile given (with '-profile') before the '-entry' option\n");
                    exit(1);
                }
            }
            // TODO: need to issue an error on a `-profile` option that doesn't actually
            // affect any entry point...

            // Take the profile that was specified on the command line, and
            // apply it to any entry points that don't already have a profile.
            for( auto& e : rawEntryPoints )
            {
                if( e.profileID == SLANG_PROFILE_UNKNOWN )
                {
                    e.profileID = currentProfileID;
                }
            }
        }

        // Did the user try to specify output path(s)?
        if (rawOutputPaths.Count() != 0)
        {
            if (rawEntryPoints.Count() == 1 && rawOutputPaths.Count() == 1)
            {
                // There was exactly one entry point, and exactly one output path,
                // so we can directly use that path for the entry point.
                rawEntryPoints[0].outputPathIndex = 0;
            }
            else if (rawOutputPaths.Count() > rawEntryPoints.Count())
            {
                requestImpl->mSink.diagnose(
                    SourceLoc(),
                    Diagnostics::tooManyOutputPathsSpecified,
                    rawOutputPaths.Count(),
                    rawEntryPoints.Count());
            }
            else
            {
                // If the user tried to apply explicit output paths, but there
                // were any entry points that didn't pick up a path, that is
                // an error:
                for( auto& entryPoint : rawEntryPoints )
                {
                    if (entryPoint.outputPathIndex < 0)
                    {
                        requestImpl->mSink.diagnose(
                            SourceLoc(),
                            Diagnostics::noOutputPathSpecifiedForEntryPoint,
                            entryPoint.name);

                        // Don't emit this same error for other entry
                        // points, even if we have more
                        break;
                    }
                }
            }

            // All of the output paths had better agree on the format
            // they should provide.
            switch (chosenTarget)
            {
            case SLANG_TARGET_NONE:
            case SLANG_TARGET_UNKNOWN:
                // No direct `-target` argument given, so try to infer
                // a target from the entry points:
                {
                    bool anyUnknownTargets = false;
                    for (auto rawOutputPath : rawOutputPaths)
                    {
                        if (rawOutputPath.target == SLANG_TARGET_UNKNOWN)
                        {
                            // This file didn't imply a target, and that
                            // needs to be an error:
                            requestImpl->mSink.diagnose(
                                SourceLoc(),
                                Diagnostics::cannotDeduceOutputFormatFromPath,
                                rawOutputPath.path);

                            // Don't keep looking for errors
                            anyUnknownTargets = true;
                            break;
                        }
                    }

                    if (!anyUnknownTargets)
                    {
                        // Okay, all the files have explicit targets,
                        // so we will set the code generation target
                        // accordingly, and then ensure that all
                        // the other output paths specified (if any)
                        // are consistent with the chosen target.
                        //
                        auto target = rawOutputPaths[0].target;
                        spSetCodeGenTarget(
                            compileRequest,
                            target);

                        for (auto rawOutputPath : rawOutputPaths)
                        {
                            if (rawOutputPath.target != target)
                            {
                                // This file didn't imply a target, and that
                                // needs to be an error:
                                requestImpl->mSink.diagnose(
                                    SourceLoc(),
                                    Diagnostics::outputPathsImplyDifferentFormats,
                                    rawOutputPaths[0].path,
                                    rawOutputPath.path);

                                // Don't keep looking for errors
                                break;
                            }
                        }
                    }

                }
                break;

            default:
                {
                    // An explicit target was given on the command-line.
                    // We will trust that the user knows what they are
                    // doing, even if one of the output files implies
                    // a different format.
                }
                break;

            }
        }

        // Next, we want to make sure that entry points get attached to the appropriate translation
        // unit that will provide them.
        {
            bool anyEntryPointWithoutTranslationUnit = false;
            for( auto& entryPoint : rawEntryPoints )
            {
                // Skip entry points that are already associated with a translation unit...
                if( entryPoint.translationUnitIndex != -1 )
                    continue;

                anyEntryPointWithoutTranslationUnit = true;
                entryPoint.translationUnitIndex = 0;
            }

            if( anyEntryPointWithoutTranslationUnit && translationUnitCount != 1 )
            {
                fprintf(stderr, "error: when using multiple translation units, entry points must be specified after their translation unit file(s)\n");
                exit(1);
            }

            // Now place all those entry points where they belong
            for( auto& entryPoint : rawEntryPoints )
            {
                int entryPointIndex = spAddEntryPoint(
                    compileRequest,
                    entryPoint.translationUnitIndex,
                    entryPoint.name.begin(),
                    entryPoint.profileID);

                // If an output path was specified for the entry point,
                // when we need to provide it here.
                if (entryPoint.outputPathIndex >= 0)
                {
                    auto rawOutputPath = rawOutputPaths[entryPoint.outputPathIndex];

                    requestImpl->entryPoints[entryPointIndex]->outputPath = rawOutputPath.path;
                }
            }
        }

#if 0
        // Automatically derive an output directory based on the first file specified.
        //
        // TODO: require manual specification if there are multiple input files, in different directories
        String fileName = options.translationUnits[0].sourceFilePaths[0];
        if (outputDir.Length() == 0)
        {
            outputDir = Path::GetDirectoryName(fileName);
        }
#endif

        if (requestImpl->mSink.GetErrorCount() != 0)
            return 1;

        return 0;
    }
};


int parseOptions(
    SlangCompileRequest*    compileRequest,
    int                     argc,
    char const* const*      argv)
{
    OptionsParser parser;
    parser.compileRequest = compileRequest;
    parser.requestImpl = (Slang::CompileRequest*) compileRequest;
    return parser.parse(argc, argv);
}


} // namespace Slang

SLANG_API int spProcessCommandLineArguments(
    SlangCompileRequest*    request,
    char const* const*      args,
    int                     argCount)
{
    return Slang::parseOptions(request, argCount, args);
}
