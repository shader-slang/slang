// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

#include "../core/slang-math.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CompileState !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult CompileState::loadState(Session* session)
{
    SLANG_UNUSED(session);
    //

    return SLANG_OK;
}


SlangResult CompileState::loadState(EndToEndCompileRequest* request)
{
    RequestState& dstState = requestState;

    dstState.compileFlags = request->getFrontEndReq()->compileFlags;
    dstState.shouldDumpIntermediates = request->getBackEndReq()->shouldDumpIntermediates;
    dstState.lineDirectiveMode = request->getBackEndReq()->lineDirectiveMode;

    auto linkage = request->getLinkage();

    dstState.debugInfoLevel = linkage->debugInfoLevel;
    dstState.optimizationLevel = linkage->optimizationLevel;
    dstState.containerFormat = request->containerFormat;
    dstState.passThroughMode = request->passThrough;

    {
        const auto defaultMatrixLayoutMode = linkage->defaultMatrixLayoutMode;

        dstState.targets.clear();
        for (TargetRequest* targetRequest : linkage->targets)
        {
            TargetState targetState;
            targetState.target = targetRequest->getTarget();
            targetState.profile = targetRequest->getTargetProfile();
            targetState.targetFlags = targetRequest->targetFlags;
            targetState.floatingPointMode = targetRequest->floatingPointMode;
            targetState.defaultMatrixLayoutMode = defaultMatrixLayoutMode;

            dstState.targets.add(targetState);
        }
    }

    {
        dstState.searchPaths.clear();
        // We don't handle parents here
        SLANG_ASSERT(linkage->searchDirectories.parent == nullptr);
        for (auto& searchPath : linkage->searchDirectories.searchDirectories)
        {
            dstState.searchPaths.add(searchPath.path);
        }
    }

    {
        dstState.preprocessorDefinitions.clear();

        for (const auto srcDefine : linkage->preprocessorDefinitions)
        {
            Define define;
            define.key = srcDefine.Key;
            define.value = srcDefine.Value;

            dstState.preprocessorDefinitions.add(define);
        }
    }

    {
        dstState.translationUnits.clear();

        for (TranslationUnitRequest* srcTranslationUnit : request->getFrontEndReq()->translationUnits)
        {
            TranslationUnitState dstTranslationUnit;

            dstTranslationUnit.language = srcTranslationUnit->sourceLanguage;
            dstTranslationUnit.moduleName = srcTranslationUnit->moduleName->text;

            for (const auto srcDefine : srcTranslationUnit->preprocessorDefinitions)
            {
                Define define;
                define.key = srcDefine.Key;
                define.value = srcDefine.Value;

                dstTranslationUnit.preprocessorDefinitions.add(define);
            }
            
            dstState.translationUnits.add(dstTranslationUnit);
        }
    }

    return SLANG_OK;
}

} // namespace Slang
