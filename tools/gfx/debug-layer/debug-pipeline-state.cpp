// debug-pipeline-state.cpp
#include "debug-pipeline-state.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

Result DebugPipelineState::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeHandle(outHandle);
}

} // namespace debug
} // namespace gfx
