#ifndef SLANG_APP_CONTEXT_H
#define SLANG_APP_CONTEXT_H

#include "slang-writer.h"
#include "../../slang-com-ptr.h"

namespace Slang
{

#ifdef SLANG_SHARED_LIBRARY_TOOL
#   define SLANG_SHARED_LIBRARY_TOOL_API SLANG_EXTERN_C SLANG_DLL_EXPORT 
#else
#   define SLANG_SHARED_LIBRARY_TOOL_API
#endif

/* A structure to hold general state shared across an application */
class AppContext
{
public:
    
    ISlangWriter * getWriter(SlangWriterChannel chan) const { return m_writers[chan]; }
    void setWriter(SlangWriterChannel chan, ISlangWriter* writer) { m_writers[chan] = writer; }

        /// Make modifications to the request
    void configureRequest(SlangCompileRequest* request);

    void setRequestWriters(SlangCompileRequest* request);

    void setReplaceWriterFlagsAll() { setReplaceWriterFlags((1 << SLANG_WRITER_CHANNEL_COUNT_OF) - 1); }
    void setReplaceWriterFlags(int flags) { m_replaceWriterFlags = flags;  }
    int getReplaceWriterFlags() const { return m_replaceWriterFlags;  }
 
        /// Ctor
    AppContext() : m_replaceWriterFlags(0) {}

        /// Initialize a default context
    static AppContext* initDefault();

    static AppContext* getDefault();

    static AppContext* getSingleton() { return s_singleton; }
    static void setSingleton(AppContext* context) { s_singleton = context;  }

    static WriterHelper getStdError() { return getSingleton()->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR); }
    static WriterHelper getStdOut() { return getSingleton()->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT); }
    static WriterHelper getDiagnostic() { return getSingleton()->getWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC); }

    static int getReturnCode(SlangResult res);

protected:

    ComPtr<ISlangWriter> m_writers[SLANG_WRITER_CHANNEL_COUNT_OF]; 
    int m_replaceWriterFlags;                                               ///< Bit for each writer

    static AppContext* s_singleton;
};

}

#endif
