
#include "slang-std-writers.h"

namespace Slang
{

/* static */StdWriters* StdWriters::s_singleton = nullptr;

/* static */StdWriters* StdWriters::getDefault()
{
    static StdWriters* s_context = nullptr;

    if (!s_context)
    {
        static FileWriter s_stdError(stderr, WriterFlag::IsStatic | WriterFlag::IsUnowned | WriterFlag::AutoFlush);
        static FileWriter s_stdOut(stdout, WriterFlag::IsStatic | WriterFlag::IsUnowned | WriterFlag::AutoFlush);

        static StdWriters s_contextVar;
        s_context = &s_contextVar;

        s_context->setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, &s_stdError);
        s_context->setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, &s_stdOut);
    }
    return s_context;
}

/* static */StdWriters* StdWriters::initDefault()
{
    StdWriters* context = getDefault();
    setSingleton(context);
    return context;
}

void StdWriters::setRequestWriters(SlangCompileRequest* request)
{
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        spSetWriter(request, SlangWriterChannel(i), m_writers[i]);
    }
}

}

