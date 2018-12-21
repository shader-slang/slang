
#include "slang-test-tool-util.h"

namespace Slang
{

/* static */int TestToolUtil::getReturnCode(SlangResult res)
{
    if (SLANG_SUCCEEDED(res))
    {
        return 0;
    }
    else if (res == SLANG_E_INTERNAL_FAIL)
    {
        return -1;
    }
    return 1;
}

}

