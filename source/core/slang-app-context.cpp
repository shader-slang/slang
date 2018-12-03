
#include "slang-app-context.h"

namespace Slang
{

/* static */AppContext* AppContext::s_singleton = nullptr;


/* static */AppContext* AppContext::initDefault()
{
    static FileWriteStream stdError(stderr, false);
    static FileWriteStream stdOut(stdout, false);

    static AppContext context;

    context.setStream(StreamType::StdError, &stdError);
    context.setStream(StreamType::StdOut, &stdOut);

    setSingleton(&context);
    return &context;
}

/* static */int AppContext::getReturnCode(SlangResult res)
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

