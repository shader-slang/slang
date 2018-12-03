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
    
    ISlangWriter * getWriter(SlangWriterTargetType type) const { return m_streams[type]; }
    void setWriter(SlangWriterTargetType type, ISlangWriter* writer) { m_streams[type] = writer; }

        /// Initialize a default context
    static AppContext* initDefault();

    static AppContext* getSingleton() { return s_singleton; }
    static void setSingleton(AppContext* context) { s_singleton = context;  }

    static WriterHelper getStdError() { return getSingleton()->getWriter(SLANG_WRITER_TARGET_TYPE_STD_ERROR); }
    static WriterHelper getStdOut() { return getSingleton()->getWriter(SLANG_WRITER_TARGET_TYPE_STD_OUTPUT); }
    static WriterHelper getDiagnostic() { return getSingleton()->getWriter(SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC); }

    static int getReturnCode(SlangResult res);

protected:

    ComPtr<ISlangWriter> m_streams[SLANG_WRITER_TARGET_TYPE_COUNT_OF]; 
    
    static AppContext* s_singleton;
};

}

#endif
