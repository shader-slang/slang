
#include "slang-std-writers.h"

namespace Slang
{

/* static */StdWriters* GlobalWriters::s_singleton = nullptr;

/* static */RefPtr<StdWriters> StdWriters::createDefault()
{
    RefPtr<StdWriters> stdWriters(new StdWriters);

    RefPtr<FileWriter> stdError(new FileWriter(stderr, WriterFlag::AutoFlush));
    RefPtr<FileWriter> stdOut(new FileWriter(stdout, WriterFlag::AutoFlush));

    stdWriters->setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, stdError);
    stdWriters->setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, stdOut);
    
    return stdWriters;
}

/* static */RefPtr<StdWriters> StdWriters::createNull()
{
    RefPtr<StdWriters> stdWriters(new StdWriters);
    ComPtr<ISlangWriter> writer(new NullWriter(WriterFlag::AutoFlush));

    stdWriters->setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, writer);
    stdWriters->setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, writer);
    stdWriters->setWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC, writer);

    return stdWriters;
}

/* static */RefPtr<StdWriters> GlobalWriters::initDefaultSingleton()
{
    if (s_singleton)
    {
        return s_singleton;
    }
    auto defaults = StdWriters::createDefault();
    setSingleton(defaults);
    return defaults;
}

void StdWriters::setRequestWriters(SlangCompileRequest* request)
{
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        spSetWriter(request, SlangWriterChannel(i), m_writers[i]);
    }
}

}

