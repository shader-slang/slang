// slang-state-serialize.h
#ifndef SLANG_STATE_SERIALIZE_H_INCLUDED
#define SLANG_STATE_SERIALIZE_H_INCLUDED

#include "../core/slang-basic.h"
#include "../core/slang-stream.h"
#include "../core/slang-string.h"

// For TranslationUnitRequest
#include "slang-compiler.h"

namespace Slang {


// Work out the state based on the api
class CompileState
{
    class File
    {
        enum Type
        {
            FileSystem,             ///< Loaded from the file system
            String,           ///< Specified as a String
        };

        Type type;
        Slang::String path;
        Slang::String contents;
    };

    // Ignore
    // spSessionSetSharedLibraryLoader
    // spAddBuiltins
    // spSetFileSystem
    // spSetDiagnosticCallback
    // spSetWriter

    struct SessionState
    {
        
        //List<File> builtIns;


    };

    // spSetCodeGenTarget/spAddCodeGenTarget
    // spSetTargetProfile
    // spSetTargetFlags
    // spSetTargetFloatingPointMode
    // spSetTargetMatrixLayoutMode
    struct TargetState
    {
        Slang::Profile profile;
        Slang::CodeGenTarget target;
        SlangTargetFlags targetFlags;
        Slang::FloatingPointMode floatingPointMode;

        SlangMatrixLayoutMode defaultMatrixLayoutMode;
    };

    struct Define
    {
        String key;
        String value;
    };

    // spAddTranslationUnit
    struct TranslationUnitState
    {
        SourceLanguage language;

        String moduleName;

        // spTranslationUnit_addPreprocessorDefine
        List<Define> preprocessorDefinitions;
    };

    struct RequestState
    {
        // spSetCompileFlags
        SlangCompileFlags compileFlags;
        // spSetDumpIntermediates
        bool shouldDumpIntermediates;
        // spSetLineDirectiveMode
        Slang::LineDirectiveMode lineDirectiveMode;
        
        List<TargetState> targets;

        // spSetDebugInfoLevel
        Slang::DebugInfoLevel debugInfoLevel;
        // spSetOptimizationLevel
        Slang::OptimizationLevel optimizationLevel;
        // spSetOutputContainerFormat
        Slang::ContainerFormat containerFormat;
        // spSetPassThrough
        Slang::PassThroughMode passThroughMode;

        // spAddSearchPath
        List<String> searchPaths;

        // spAddPreprocessorDefine
        List<Define> preprocessorDefinitions;

        List<TranslationUnitState> translationUnits;
    };

    SessionState sessionState;
    RequestState requestState;

    SlangResult loadState(Session* session);
    SlangResult loadState(EndToEndCompileRequest* request);
};

} // namespace Slang

#endif
