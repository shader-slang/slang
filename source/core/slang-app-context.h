#ifndef SLANG_APP_CONTEXT_H
#define SLANG_APP_CONTEXT_H

#include "slang-write-stream.h"

namespace Slang
{

/* A structure to hold general state shared across an application */
class AppContext
{
public:

    WriteStream * getStdError() const { return m_stdError;  }
    WriteStream* getStdOut() const { return m_stdOut;  }

        /// Initialize a default context
    static void initDefault();

    static AppContext* getSingleton() { return s_singleton; }
    static void setSingleton(AppContext* context) { s_singleton = context;  }

protected:

    WriteStream * m_stdError;
    WriteStream* m_stdOut;

    static AppContext* s_singleton;
};

}

#endif
