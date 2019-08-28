// slang-options.cpp

// Implementation of options parsing for `slangc` command line,
// and also for API interface that takes command-line argument strings.

#include "../../slang.h"

#include "slang-compiler.h"
#include "slang-profile.h"

#include "slang-file-system.h"

#include <assert.h>

namespace Slang {

SlangResult tryReadCommandLineArgumentRaw(DiagnosticSink* sink, char const* option, char const* const**ioCursor, char const* const*end, char const** argOut)
{
    *argOut = nullptr;
    char const* const*& cursor = *ioCursor;
    if (cursor == end)
    {
        sink->diagnose(SourceLoc(), Diagnostics::expectedArgumentForOption, option);
        return SLANG_FAIL;
    }
    else
    {
        *argOut = *cursor++;
        return SLANG_OK;
    }
}

SlangResult tryReadCommandLineArgument(DiagnosticSink* sink, char const* option, char const* const**ioCursor, char const* const*end, String& argOut)
{
    const char* arg;
    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgumentRaw(sink, option, ioCursor, end, &arg));
    argOut = arg;
    return SLANG_OK;
}

#define SLANG_PASS_THROUGH_TYPES(x) \
        x(none, NONE) \
        x(fxc, FXC) \
        x(dxc, DXC) \
        x(glslang, GLSLANG) \
        x(vs, VISUAL_STUDIO) \
        x(visualstudio, VISUAL_STUDIO) \
        x(clang, CLANG) \
        x(gcc, GCC) \
        x(c, GENERIC_C_CPP) \
        x(cpp, GENERIC_C_CPP)

static SlangResult _parsePassThrough(const UnownedStringSlice& name, SlangPassThrough& outPassThrough)
{
#define SLANG_PASS_THROUGH_TYPE_CHECK(x, y) \
    if (name == #x) { outPassThrough = SLANG_PASS_THROUGH_##y; return SLANG_OK; }
    SLANG_PASS_THROUGH_TYPES(SLANG_PASS_THROUGH_TYPE_CHECK)
    return SLANG_FAIL;
}

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
    };
    List<RawOutput> rawOutputs;

    struct RawTarget
    {
        CodeGenTarget       format = CodeGenTarget::Unknown;
        ProfileVersion      profileVersion = ProfileVersion::Unknown;
        SlangTargetFlags    targetFlags = 0;
        int                 targetID = -1;
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;

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
        auto translationUnitID = spAddTranslationUnit(compileRequest, language, nullptr);

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

        spAddTranslationUnitSourceFile(
            compileRequest,
            rawTranslationUnits[slangTranslationUnitIndex].translationUnitID,
            path.begin());

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

        spAddTranslationUnitSourceFile(
            compileRequest,
            rawTranslationUnits[currentTranslationUnitIndex].translationUnitID,
            path.begin());
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
        char const*  inPath)
    {
        inputPathCount++;

        // look at the extension on the file name to determine
        // how we should handle it.
        String path = String(inPath);

        if( path.endsWith(".slang") )
        {
            // Plain old slang code
            addInputSlangPath(path);
            return SLANG_OK;
        }

        Stage impliedStage = Stage::Unknown;
        SlangSourceLanguage sourceLanguage = findSourceLanguageFromPath(path, impliedStage);
        
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

        if (!inPath) {}
#define CASE(EXT, TARGET)   \
        else if(path.endsWith(EXT)) do { addOutputPath(path, CodeGenTarget(SLANG_##TARGET)); } while(0)

        CASE(".hlsl", HLSL);
        CASE(".fx",   HLSL);

        CASE(".dxbc", DXBC);
        CASE(".dxbc.asm", DXBC_ASM);

        CASE(".dxil", DXIL);
        CASE(".dxil.asm", DXIL_ASM);

        CASE(".glsl", GLSL);
        CASE(".vert", GLSL);
        CASE(".frag", GLSL);
        CASE(".geom", GLSL);
        CASE(".tesc", GLSL);
        CASE(".tese", GLSL);
        CASE(".comp", GLSL);

        CASE(".spv",        SPIRV);
        CASE(".spv.asm",    SPIRV_ASM);

        CASE(".c",      C_SOURCE);
        CASE(".cpp",    CPP_SOURCE);

        CASE(".exe",    EXECUTABLE);
        CASE(".dll",    SHARED_LIBRARY);
        CASE(".so",     SHARED_LIBRARY);

#undef CASE

        else if (path.endsWith(".slang-module"))
        {
            spSetOutputContainerFormat(compileRequest, SLANG_CONTAINER_FORMAT_SLANG_MODULE);
            requestImpl->containerOutputPath = path;
        }
        else
        {
            // Allow an unknown-format `-o`, assuming we get a target format
            // from another argument.
            addOutputPath(path, CodeGenTarget::Unknown);
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

    SlangResult parse(
        int             argc,
        char const* const*  argv)
    {
        // Copy some state out of the current request, in case we've been called
        // after some other initialization has been performed.
        flags = requestImpl->getFrontEndReq()->compileFlags;

        DiagnosticSink* sink = requestImpl->getSink();

        SlangMatrixLayoutMode defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

        char const* const* argCursor = &argv[0];
        char const* const* argEnd = &argv[argc];
        while (argCursor != argEnd)
        {
            char const* arg = *argCursor++;
            if (arg[0] == '-')
            {
                String argStr = String(arg);

                if(argStr == "-no-mangle" )
                {
                    flags |= SLANG_COMPILE_FLAG_NO_MANGLING;
                }
                else if (argStr == "-no-codegen")
                {
                    flags |= SLANG_COMPILE_FLAG_NO_CODEGEN;
                }
                else if(argStr == "-dump-ir" )
                {
                    requestImpl->getFrontEndReq()->shouldDumpIR = true;
                    requestImpl->getBackEndReq()->shouldDumpIR = true;
                }
                else if (argStr == "-serial-ir")
                {
                    requestImpl->getFrontEndReq()->useSerialIRBottleneck = true;
                }
                else if (argStr == "-verbose-paths")
                {
                    requestImpl->getSink()->flags |= DiagnosticSink::Flag::VerbosePath;
                }
                else if (argStr == "-verify-debug-serial-ir")
                {
                    requestImpl->getFrontEndReq()->verifyDebugSerialization = true;
                }
                else if(argStr == "-validate-ir" )
                {
                    requestImpl->getFrontEndReq()->shouldValidateIR = true;
                    requestImpl->getBackEndReq()->shouldValidateIR = true;
                }
                else if(argStr == "-skip-codegen" )
                {
                    requestImpl->shouldSkipCodegen = true;
                }
                else if(argStr == "-parameter-blocks-use-register-spaces" )
                {
                    getCurrentTarget()->targetFlags |= SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES;
                }
                else if (argStr == "-target")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    const CodeGenTarget format = calcCodeGenTargetFromName(name.getUnownedSlice());

                    if (format == CodeGenTarget::Unknown)
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownCodeGenerationTarget, name);
                        return SLANG_FAIL;
                    }

                    RawTarget rawTarget;
                    rawTarget.format = CodeGenTarget(format);

                    rawTargets.add(rawTarget);
                }
                // A "profile" can specify both a general capability level for
                // a target, and also (as a legacy/compatibility feature) a
                // specific stage to use for an entry point.
                else if (argStr == "-profile")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    SlangProfileID profileID = spFindProfile(session, name.begin());
                    if( profileID == SLANG_PROFILE_UNKNOWN )
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownProfile, name);
                        return SLANG_FAIL;
                    }
                    else
                    {
                        auto profile = Profile(profileID);

                        setProfileVersion(getCurrentTarget(), profile.GetVersion());

                        // A `-profile` option that also specifies a stage (e.g., `-profile vs_5_0`)
                        // should be treated like a composite (e.g., `-profile sm_5_0 -stage vertex`)
                        auto stage = profile.GetStage();
                        if(stage != Stage::Unknown)
                        {
                            setStage(getCurrentEntryPoint(), stage);
                        }
                    }
                }
                else if (argStr == "-stage")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    Stage stage = findStageByName(name);
                    if( stage == Stage::Unknown )
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownStage, name);
                        return SLANG_FAIL;
                    }
                    else
                    {
                        setStage(getCurrentEntryPoint(), stage);
                    }
                }
                else if (argStr == "-entry")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    RawEntryPoint rawEntryPoint;
                    rawEntryPoint.name = name;
                    rawEntryPoint.translationUnitIndex = currentTranslationUnitIndex;

                    rawEntryPoints.add(rawEntryPoint);
                }
                else if (argStr == "-pass-through")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
                    if (SLANG_FAILED(_parsePassThrough(name.getUnownedSlice(), passThrough)))
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownPassThroughTarget, name);
                        return SLANG_FAIL;
                    }

                    spSetPassThrough(compileRequest, passThrough);
                }
                else if (argStr.getLength() >= 2 && argStr[1] == 'D')
                {
                    // The value to be defined might be part of the same option, as in:
                    //     -DFOO
                    // or it might come separately, as in:
                    //     -D FOO
                    char const* defineStr = arg + 2;
                    if (defineStr[0] == 0)
                    {
                        // Need to read another argument from the command line
                        SLANG_RETURN_ON_FAIL(tryReadCommandLineArgumentRaw(sink, arg, &argCursor, argEnd, &defineStr));
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
                else if (argStr.getLength() >= 2 && argStr[1] == 'I')
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
                        SLANG_RETURN_ON_FAIL(tryReadCommandLineArgumentRaw(sink, arg, &argCursor, argEnd, &includeDirStr));
                    }

                    spAddSearchPath(
                        compileRequest,
                        String(includeDirStr).begin());
                }
                //
                // A `-o` option is used to specify a desired output file.
                else if (argStr == "-o")
                {
                    char const* outputPath = nullptr;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgumentRaw(sink, arg, &argCursor, argEnd, &outputPath));
                    if (!outputPath) continue;

                    addOutputPath(outputPath);
                }
                else if(argStr == "-matrix-layout-row-major")
                {
                    defaultMatrixLayoutMode = kMatrixLayoutMode_RowMajor;
                }
                else if(argStr == "-matrix-layout-column-major")
                {
                    defaultMatrixLayoutMode = kMatrixLayoutMode_ColumnMajor;
                }
                else if(argStr == "-line-directive-mode")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    SlangLineDirectiveMode mode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
                    if(name == "none")
                    {
                        mode = SLANG_LINE_DIRECTIVE_MODE_NONE;
                    }
                    else
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownLineDirectiveMode, name);
                        return SLANG_FAIL;
                    }

                    spSetLineDirectiveMode(compileRequest, mode);

                }
                else if( argStr == "-fp-mode" || argStr == "-floating-point-mode" )
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    FloatingPointMode mode = FloatingPointMode::Default;
                    if(name == "fast")
                    {
                        mode = FloatingPointMode::Fast;
                    }
                    else if(name == "precise")
                    {
                        mode = FloatingPointMode::Precise;
                    }
                    else
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownFloatingPointMode, name);
                        return SLANG_FAIL;
                    }

                    setFloatingPointMode(getCurrentTarget(), mode);
                }
                else if( argStr[1] == 'O' )
                {
                    char const* name = arg + 2;
                    SlangOptimizationLevel level = SLANG_OPTIMIZATION_LEVEL_DEFAULT;

                    bool invalidOptimizationLevel = strlen(name) > 2;
                    switch( name[0] )
                    {
                    case '0':   level = SLANG_OPTIMIZATION_LEVEL_NONE;      break;
                    case '1':   level = SLANG_OPTIMIZATION_LEVEL_DEFAULT;   break;
                    case '2':   level = SLANG_OPTIMIZATION_LEVEL_HIGH;      break;
                    case '3':   level = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;   break;
                    case  0 :   level = SLANG_OPTIMIZATION_LEVEL_DEFAULT;   break;
                    default:
                        invalidOptimizationLevel = true;
                        break;
                    }
                    if( invalidOptimizationLevel )
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownOptimiziationLevel, name);
                        return SLANG_FAIL;
                    }

                    spSetOptimizationLevel(compileRequest, level);
                }

                // Note: unlike with `-O` above, we have to consider that other
                // options might have names that start with `-g` and so cannot
                // just detect it as a prefix.
                else if( argStr == "-g" || argStr == "-g2" )
                {
                    spSetDebugInfoLevel(compileRequest, SLANG_DEBUG_INFO_LEVEL_STANDARD);
                }
                else if( argStr == "-g0" )
                {
                    spSetDebugInfoLevel(compileRequest, SLANG_DEBUG_INFO_LEVEL_NONE);
                }
                else if( argStr == "-g1" )
                {
                    spSetDebugInfoLevel(compileRequest, SLANG_DEBUG_INFO_LEVEL_MINIMAL);
                }
                else if( argStr == "-g3" )
                {
                    spSetDebugInfoLevel(compileRequest, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
                }
                else if( argStr == "-default-image-format-unknown" )
                {
                    requestImpl->getBackEndReq()->useUnknownImageFormatAsDefault = true;
                }
                else if (argStr == "-file-system")
                {
                    String name;
                    SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                    if (name == "default")
                    {
                        spSetFileSystem(compileRequest, nullptr);
                    }
                    else if (name == "load-file")
                    {
                        // OSFileSystem just implements loadFile interface, so will be wrapped with CacheFileSystem internally
                        spSetFileSystem(compileRequest, OSFileSystem::getSingleton());
                    }
                    else if (name == "os")
                    {
                        // OSFileSystemExt implements the ISlangFileSystemExt interface - and will be used directly
                        spSetFileSystem(compileRequest, OSFileSystemExt::getSingleton());
                    }
                    else
                    {
                        sink->diagnose(SourceLoc(), Diagnostics::unknownFileSystemOption, name);
                        return SLANG_FAIL;
                    }
                }
                else if (argStr == "-v")
                {
                    sink->diagnoseRaw(Severity::Note, session->getBuildTagString());
                }
                else if (argStr == "--")
                {
                    // The `--` option causes us to stop trying to parse options,
                    // and treat the rest of the command line as input file names:
                    while (argCursor != argEnd)
                    {
                        SLANG_RETURN_ON_FAIL(addInputPath(*argCursor++));
                    }
                    break;
                }
                else
                {
                    if (argStr.endsWith("-path"))
                    {
                        Index index = argStr.lastIndexOf('-');
                        if (index >= 0)
                        {
                            String name;
                            SLANG_RETURN_ON_FAIL(tryReadCommandLineArgument(sink, arg, &argCursor, argEnd, name));

                            String slice = argStr.subString(1, index - 1);
                            SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
                            if (SLANG_SUCCEEDED(_parsePassThrough(slice.getUnownedSlice(), passThrough)))
                            {
                                session->setDownstreamCompilerPath(passThrough, name.getBuffer());
                                continue;
                            }
                        }
                    }

                    sink->diagnose(SourceLoc(), Diagnostics::unknownCommandLineOption, argStr);
                    // TODO: print a usage message
                    return SLANG_FAIL;
                }
            }
            else
            {
                SLANG_RETURN_ON_FAIL(addInputPath(arg));
            }
        }

        spSetCompileFlags(compileRequest, flags);

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
        if(_passThroughRequiresStage(requestImpl->passThrough) )
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

            int entryPointID = spAddEntryPoint(
                compileRequest,
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
            int targetID = spAddCodeGenTarget(compileRequest, SlangCompileTarget(rawTarget.format));
            rawTarget.targetID = targetID;

            if( rawTarget.profileVersion != ProfileVersion::Unknown )
            {
                spSetTargetProfile(compileRequest, targetID, Profile(rawTarget.profileVersion).raw);
            }

            if( rawTarget.targetFlags )
            {
                spSetTargetFlags(compileRequest, targetID, rawTarget.targetFlags);
            }

            if( rawTarget.floatingPointMode != FloatingPointMode::Default )
            {
                spSetTargetFloatingPointMode(compileRequest, targetID, SlangFloatingPointMode(rawTarget.floatingPointMode));
            }
        }

        if(defaultMatrixLayoutMode != SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
        {
            spSetMatrixLayoutMode(compileRequest, defaultMatrixLayoutMode);
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

        // Consider the output files specified via `-o` and try to figure
        // out how to deal with them.
        //
        for(auto& rawOutput : rawOutputs)
        {
            // For now, all output formats need to be tightly bound to
            // both a target and an entry point (down the road we will
            // need to support output formats that can store multiple
            // entry points in one file).

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
            //
            if( rawOutput.entryPointIndex == -1 )
            {
                sink->diagnose(SourceLoc(), Diagnostics::cannotMatchOutputFileToEntryPoint, rawOutput.path);
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
            if(rawOutput.entryPointIndex == -1) continue;

            auto targetID = rawTargets[rawOutput.targetIndex].targetID;
            Int entryPointID = rawEntryPoints[rawOutput.entryPointIndex].entryPointID;

            auto target = requestImpl->getLinkage()->targets[targetID];
            auto entryPointReq = requestImpl->getFrontEndReq()->getEntryPointReqs()[entryPointID];

            RefPtr<EndToEndCompileRequest::TargetInfo> targetInfo;
            if( !requestImpl->targetInfos.TryGetValue(target, targetInfo) )
            {
                targetInfo = new EndToEndCompileRequest::TargetInfo();
                requestImpl->targetInfos[target] = targetInfo;
            }

            String outputPath;
            if( targetInfo->entryPointOutputPaths.ContainsKey(entryPointID) )
            {
                sink->diagnose(SourceLoc(), Diagnostics::duplicateOutputPathsForEntryPointAndTarget, entryPointReq->getName(), target->getTarget());
            }
            else
            {
                targetInfo->entryPointOutputPaths[entryPointID] = rawOutput.path;
            }
        }

        return (sink->GetErrorCount() == 0) ? SLANG_OK : SLANG_FAIL;
    }
};


SlangResult parseOptions(
    SlangCompileRequest*    compileRequestIn,
    int                     argc,
    char const* const*      argv)
{
    Slang::EndToEndCompileRequest* compileRequest = (Slang::EndToEndCompileRequest*) compileRequestIn;

    OptionsParser parser;
    parser.compileRequest = compileRequestIn;
    parser.requestImpl = compileRequest;
    parser.session = (SlangSession*)compileRequest->getSession();

    Result res = parser.parse(argc, argv);

    DiagnosticSink* sink = compileRequest->getSink();
    if (sink->GetErrorCount() > 0)
    {
        // Put the errors in the diagnostic 
        compileRequest->mDiagnosticOutput = sink->outputBuffer.ProduceString();
    }

    return res;
}


} // namespace Slang

SLANG_API SlangResult spProcessCommandLineArguments(
    SlangCompileRequest*    request,
    char const* const*      args,
    int                     argCount)
{
    return Slang::parseOptions(request, argCount, args);
}
