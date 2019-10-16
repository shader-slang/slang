// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

#include "../core/slang-math.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RelativeContainer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

RelativeContainer::RelativeContainer()
{
    m_current = 0;
}

void* RelativeContainer::allocate(size_t size, size_t alignment)
{
    size_t offset = (m_current + alignment - 1) & ~(alignment - 1);

    size_t dataSize = m_data.getCount();

    if (offset + size > dataSize)
    {
        const size_t minSize = offset + size;

        size_t calcSize = dataSize;
        if (dataSize < 2048)
        {
            calcSize = 2048;
        }
        else
        {
            // Expand geometrically, but lets not double in size...
            calcSize = dataSize + (dataSize / 2);
        }

        // We must be at least calc size
        size_t newSize = (calcSize < minSize) ? minSize : calcSize;
        m_data.setCount(newSize);

        //m_data.getCapacity();

        dataSize = newSize;
    }
    SLANG_ASSERT(offset + size <= dataSize);

    m_current = offset + size;

    return m_data.getBuffer() + offset;
}

void* RelativeContainer::allocateAndZero(size_t size, size_t alignment)
{
    void* data = allocate(size, alignment);
    memset(data, 0, size);
    return data;
}

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
