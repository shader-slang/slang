
#include "slang-app-context.h"

#include "slang-writer.h"

namespace Slang
{

/* static */AppContext* AppContext::s_singleton = nullptr;


/* static */AppContext* AppContext::getDefault()
{
    static AppContext* s_context = nullptr;

    if (!s_context)
    {
        static FileWriter s_stdError(stderr, WriterFlag::IsStatic | WriterFlag::IsUnowned | WriterFlag::AutoFlush);
        static FileWriter s_stdOut(stdout, WriterFlag::IsStatic | WriterFlag::IsUnowned | WriterFlag::AutoFlush);

        static AppContext s_contextVar;
        s_context = &s_contextVar;

        s_context->setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, &s_stdError);
        s_context->setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, &s_stdOut);
    }
    return s_context;
}

/* static */AppContext* AppContext::initDefault()
{
    AppContext* context = getDefault();
    setSingleton(context);
    return context;
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

void AppContext::setRequestWriters(SlangCompileRequest* request)
{
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        if (m_replaceWriterFlags & (1 << i))
        {
            spSetWriter(request, SlangWriterChannel(i), m_writers[i]);
        }
    }
}

void AppContext::configureRequest(SlangCompileRequest* request)
{
    setRequestWriters(request);
}

}

