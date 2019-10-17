// slang-state-serialize.h
#ifndef SLANG_STATE_SERIALIZE_H_INCLUDED
#define SLANG_STATE_SERIALIZE_H_INCLUDED

#include "../core/slang-riff.h"
#include "../core/slang-string.h"

// For TranslationUnitRequest
#include "slang-compiler.h"

#include "../core/slang-relative-container.h"

namespace Slang {

struct StateSerializeUtil
{
    static const uint32_t kSlangStateFourCC = SLANG_FOUR_CC('S', 'L', 'S', 'T');             ///< Holds all the slang specific chunks
    
    struct Header
    {
        RiffChunk m_chunk;
        uint32_t m_compressionType;         ///< Holds the compression type used (if used at all)
    };

    struct SessionState
    {

    };

        // spSetCodeGenTarget/spAddCodeGenTarget
        // spSetTargetProfile
        // spSetTargetFlags
        // spSetTargetFloatingPointMode
        // spSetTargetMatrixLayoutMode
    struct TargetRequestState
    {
        Profile profile;
        CodeGenTarget target;
        SlangTargetFlags targetFlags;
        FloatingPointMode floatingPointMode;
        SlangMatrixLayoutMode defaultMatrixLayoutMode;
    };

    struct Define
    {
        Relative32Ptr<RelativeString> key;
        Relative32Ptr<RelativeString> value;
    };

        // spAddTranslationUnit
    struct TranslationUnitRequestState
    {
        SourceLanguage language;

        Relative32Ptr<RelativeString> moduleName;

        // spTranslationUnit_addPreprocessorDefine
        Relative32Array<Define> preprocessorDefinitions;
    };

    struct RequestState
    {
        Relative32Ptr<SessionState> session;

        // spSetCompileFlags
        SlangCompileFlags compileFlags;
        // spSetDumpIntermediates
        bool shouldDumpIntermediates;
        // spSetLineDirectiveMode
        LineDirectiveMode lineDirectiveMode;

        Relative32Array<TargetRequestState> targetRequests;

        // spSetDebugInfoLevel
        DebugInfoLevel debugInfoLevel;
        // spSetOptimizationLevel
        OptimizationLevel optimizationLevel;
        // spSetOutputContainerFormat
        ContainerFormat containerFormat;
        // spSetPassThrough
        PassThroughMode passThroughMode;

        // spAddSearchPath
        Relative32Array<Relative32Ptr<RelativeString> > searchPaths;

        // spAddPreprocessorDefine
        Relative32Array<Define> preprocessorDefinitions;

        Relative32Array<TranslationUnitRequestState> translationUnits;
    };

    static SlangResult store(EndToEndCompileRequest* request, RelativeContainer& inOutContainer, Safe32Ptr<RequestState>& outRequest);


    static SlangResult saveState(EndToEndCompileRequest* request, const String& filename);

    static SlangResult saveState(EndToEndCompileRequest* request, Stream* stream);

};

} // namespace Slang

#endif
