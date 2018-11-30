
#include "slang-app-context.h"

namespace Slang
{

/* static */AppContext* AppContext::s_singleton = nullptr;


/* static */void AppContext::initDefault()
{
    static FileWriteStream stdError(stderr, false);
    static FileWriteStream stdOut(stdout, false);

    static AppContext context;

    context.m_stdError = &stdError;
    context.m_stdOut = &stdOut;

    setSingleton(&context);
}

}

