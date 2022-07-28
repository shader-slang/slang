// debug-transient-heap.cpp
#include "debug-transient-heap.h"

#include "debug-command-buffer.h"

#include "debug-helper-functions.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

Result DebugTransientResourceHeap::synchronizeAndReset()
{
    SLANG_GFX_API_FUNC;
    return baseObject->synchronizeAndReset();
}

Result DebugTransientResourceHeap::finish()
{
    SLANG_GFX_API_FUNC;
    return baseObject->finish();
}

Result DebugTransientResourceHeap::createCommandBuffer(ICommandBuffer** outCommandBuffer)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugCommandBuffer> outObject = new DebugCommandBuffer();
    outObject->m_transientHeap = this;
    auto result = baseObject->createCommandBuffer(outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outCommandBuffer, outObject);
    return result;
}

} // namespace debug
} // namespace gfx
