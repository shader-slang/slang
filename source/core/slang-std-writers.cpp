
#include "slang-std-writers.h"

namespace Slang
{

/* static */StdWriters* StdWriters::s_singleton = nullptr;

/* static */RefPtr<StdWriters> StdWriters::createDefault()
{
    RefPtr<StdWriters> stdWriters(new StdWriters);

    RefPtr<FileWriter> stdError(new FileWriter(stderr, WriterFlag::AutoFlush));
    RefPtr<FileWriter> stdOut(new FileWriter(stdout, WriterFlag::AutoFlush));

    stdWriters->setWriter(SLANG_WRITER_CHANNEL_STD_ERROR, stdError);
    stdWriters->setWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT, stdOut);
    
    return stdWriters;
}

/* static */RefPtr<StdWriters> StdWriters::initDefaultSingleton()
{
    if (s_singleton)
    {
        return s_singleton;
    }
    auto defaults = createDefault();
    setSingleton(defaults);
    return defaults;
}

void handleResultFail(SlangResult res, const char* file, int line)
{
    StringBuilder builder;

    builder << "Error: " << file << " (" << line << ") : ";
    builder << res;

    // Where to write to?
    auto writers = StdWriters::getSingleton();
    if (writers && writers->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR))
    {
        writers->getOut().put(builder.getUnownedSlice());
    }
    else
    {
        fprintf(stderr, "%s\n", builder.getBuffer());
    }
}

}

