// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

#include "../core/slang-math.h"

namespace Slang {


const Safe32Array<StateSerializeUtil::Define> _calcDefines(const Dictionary<String, String>& srcDefines, RelativeContainer& inOutContainer)
{
    typedef StateSerializeUtil::Define Define;

    Safe32Array<Define> dstDefines = inOutContainer.allocateArray<Define>(srcDefines.Count());

    Index index = 0;
    for (const auto& srcDefine : srcDefines)
    {
        // Do allocation before setting
        auto key = inOutContainer.newString(srcDefine.Key.getUnownedSlice());
        auto value = inOutContainer.newString(srcDefine.Value.getUnownedSlice());

        auto& dstDefine = dstDefines[index];
        dstDefine.key = key;
        dstDefine.value = value;

        index++;
    }

    return dstDefines;
}

/* static */SlangResult StateSerializeUtil::store(EndToEndCompileRequest* request, RelativeContainer& inOutContainer, Safe32Ptr<RequestState>& outRequest)
{
    auto linkage = request->getLinkage();

    Safe32Ptr<RequestState> requestState = inOutContainer.allocate<RequestState>();

    {
        RequestState* dst = requestState; 

        dst->compileFlags = request->getFrontEndReq()->compileFlags;
        dst->shouldDumpIntermediates = request->getBackEndReq()->shouldDumpIntermediates;
        dst->lineDirectiveMode = request->getBackEndReq()->lineDirectiveMode;

        dst->debugInfoLevel = linkage->debugInfoLevel;
        dst->optimizationLevel = linkage->optimizationLevel;
        dst->containerFormat = request->containerFormat;
        dst->passThroughMode = request->passThrough;
    }

    {
        const auto defaultMatrixLayoutMode = linkage->defaultMatrixLayoutMode;

        Safe32Array<TargetRequestState> dstTargets = inOutContainer.allocateArray<TargetRequestState>(linkage->targets.getCount());

        for (Index i = 0; i < linkage->targets.getCount(); ++i)
        {
            auto& dst = dstTargets[i];
            TargetRequest* targetRequest = linkage->targets[i];

            dst.target = targetRequest->getTarget();
            dst.profile = targetRequest->getTargetProfile();
            dst.targetFlags = targetRequest->targetFlags;
            dst.floatingPointMode = targetRequest->floatingPointMode;
            dst.defaultMatrixLayoutMode = defaultMatrixLayoutMode;
        }

        requestState->targetRequests = dstTargets;
    }

    {
        const auto& srcPaths = linkage->searchDirectories.searchDirectories;
        Safe32Array<Relative32Ptr<RelativeString> > dstPaths = inOutContainer.allocateArray<Relative32Ptr<RelativeString> >(srcPaths.getCount());

        // We don't handle parents here
        SLANG_ASSERT(linkage->searchDirectories.parent == nullptr);

        for (Index i = 0; i < srcPaths.getCount(); ++i)
        {
            dstPaths[i] = inOutContainer.newString(srcPaths[i].path.getUnownedSlice());
        }

        requestState->searchPaths = dstPaths;
    }

    requestState->preprocessorDefinitions = _calcDefines(linkage->preprocessorDefinitions, inOutContainer);

    {
        const auto& srcTranslationUnits = request->getFrontEndReq()->translationUnits;
        Safe32Array<TranslationUnitRequestState> dstTranslationUnits = inOutContainer.allocateArray<TranslationUnitRequestState>(srcTranslationUnits.getCount());

        for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
        {
            TranslationUnitRequest* srcTranslationUnit = srcTranslationUnits[i];

            // Do before setting, because this can allocate, and therefore break, the following section
            auto defines = _calcDefines(srcTranslationUnit->preprocessorDefinitions, inOutContainer);
            auto moduleName = inOutContainer.newString(srcTranslationUnit->moduleName->text.getUnownedSlice());

            TranslationUnitRequestState& dstTranslationUnit = dstTranslationUnits[i];

            dstTranslationUnit.language = srcTranslationUnit->sourceLanguage;
            dstTranslationUnit.moduleName = moduleName;

            dstTranslationUnit.preprocessorDefinitions = defines;
        }
    }

    outRequest = requestState;
    return SLANG_OK;
}

/* static */SlangResult StateSerializeUtil::saveState(EndToEndCompileRequest* request, Stream* stream)
{
    RelativeContainer container;
    Safe32Ptr<RequestState> requestState;
    SLANG_RETURN_ON_FAIL(store(request, container, requestState));
    return RiffUtil::writeData(kSlangStateFourCC, container.getData(), container.getDataCount(), stream);
}

/* static */SlangResult StateSerializeUtil::saveState(EndToEndCompileRequest* request, const String& filename)
{
    RefPtr<Stream> stream(new FileStream(filename, FileMode::Create));
    return saveState(request, stream);
}

} // namespace Slang
