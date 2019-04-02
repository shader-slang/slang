#ifndef SLANG_STD_WRITERS_H
#define SLANG_STD_WRITERS_H

#include "slang-writer.h"
#include "../../slang-com-ptr.h"

namespace Slang
{

/* Holds standard writers for the channels */
class StdWriters: public RefObject
{
public:

    ISlangWriter* getWriter(SlangWriterChannel chan) const { return m_writers[chan]; }
    void setWriter(SlangWriterChannel chan, ISlangWriter* writer) { m_writers[chan] = writer; }

    WriterHelper getError() { return getWriter(SLANG_WRITER_CHANNEL_STD_ERROR); }
    WriterHelper getOut() { return getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT); }
    WriterHelper getDiagnostic() { return getWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC); }

        /// Set the writers on the SlangCompileRequest
    void setRequestWriters(SlangCompileRequest* request);
        /// Create default std writers
    static RefPtr<StdWriters> createDefault();

        /// All throw away
    static RefPtr<StdWriters> createNull();

        /// Ctor
    StdWriters() {}

protected:

    ComPtr<ISlangWriter> m_writers[SLANG_WRITER_CHANNEL_COUNT_OF]; 
};

class GlobalWriters
{
public:
    /// Initialize a default context
    static RefPtr<StdWriters> initDefaultSingleton();

    static WriterHelper getError() { return getSingleton()->getError(); }
    static WriterHelper getOut() { return getSingleton()->getOut(); }
    static WriterHelper getDiagnostic() { return getSingleton()->getDiagnostic(); }

    static StdWriters* getSingleton() { return s_singleton; }
    static void setSingleton(StdWriters* context) { s_singleton = context; }

private:
    static StdWriters* s_singleton;
};

}

#endif
