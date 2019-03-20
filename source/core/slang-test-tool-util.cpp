
#include "slang-test-tool-util.h"

namespace Slang
{

/* static */ToolReturnCode TestToolUtil::getReturnCode(SlangResult res)
{
    switch (res)
    {
        case SLANG_OK:              return ToolReturnCode::Success;
        case SLANG_E_INTERNAL_FAIL: return ToolReturnCode::CompilationFailed;
        case SLANG_FAIL:            return ToolReturnCode::Failed;
        case SLANG_E_NOT_AVAILABLE: return ToolReturnCode::Ignored;
        default:
        {
            return (SLANG_SUCCEEDED(res)) ? ToolReturnCode::Success : ToolReturnCode::Failed;
        }
    }
}

/* static */ToolReturnCode TestToolUtil::getReturnCodeFromInt(int code)
{
    if (code >= int(ToolReturnCodeSpan::First) && code <= int(ToolReturnCodeSpan::Last))
    {
        return ToolReturnCode(code);
    }
    else
    {
        SLANG_ASSERT(!"Invalid integral code");
        return ToolReturnCode::Failed;
    }
}


}

