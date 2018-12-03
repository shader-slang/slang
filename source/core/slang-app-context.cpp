
#include "slang-app-context.h"

#include "slang-writer.h"

namespace Slang
{

/* static */AppContext* AppContext::s_singleton = nullptr;


/* static */AppContext* AppContext::initDefault()
{
    static FileWriter stdError(stderr, FileWriter::getIsConsoleFlag(stderr));
    static FileWriter stdOut(stdout, FileWriter::getIsConsoleFlag(stderr));

    static AppContext context;

    context.setWriter(SLANG_WRITER_TARGET_TYPE_STD_ERROR, &stdError);
    context.setWriter(SLANG_WRITER_TARGET_TYPE_STD_OUTPUT, &stdOut);

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

