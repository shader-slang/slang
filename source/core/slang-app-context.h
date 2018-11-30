#ifndef SLANG_APP_CONTEXT_H
#define SLANG_APP_CONTEXT_H

#include "slang-write-stream.h"

namespace Slang
{

/* A structure to hold general state shared across an application */
class AppContext
{
public:
    enum class StreamType
    {
        StdError,
        StdOut,
        CountOf,
    };

    WriteStream * getStream(StreamType type) const { return m_streams[int(type)]; }
    void setStream(StreamType type, WriteStream* stream) { m_streams[int(type)] = stream; }

        /// Initialize a default context
    static void initDefault();

    static AppContext* getSingleton() { return s_singleton; }
    static void setSingleton(AppContext* context) { s_singleton = context;  }

    static WriteStream * getStdError() { return getSingleton()->getStream(StreamType::StdError); }
    static WriteStream* getStdOut() { return getSingleton()->getStream(StreamType::StdOut); }

    AppContext()
    {
        for (int i = 0; i < SLANG_COUNT_OF(m_streams); ++i)
        {
            m_streams[i] = nullptr;
        }
    }

protected:

    WriteStream* m_streams[int(StreamType::CountOf)];
    
    static AppContext* s_singleton;
};

}

#endif
