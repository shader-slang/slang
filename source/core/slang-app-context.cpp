
#include "slang-app-context.h"

namespace Slang
{

/* static */AppContext* AppContext::s_singleton = nullptr;


/* static */void AppContext::initDefault()
{
    static FileWriteStream stdError(stderr, false);
    static FileWriteStream stdOut(stdout, false);

    static AppContext context;

    context.setStream(StreamType::StdError, &stdError);
    context.setStream(StreamType::StdOut, &stdOut);

    setSingleton(&context);
}

}

