#ifndef SLANG_CORE_STD_WRITERS_H
#define SLANG_CORE_STD_WRITERS_H

#include "slang-com-ptr.h"
#include "slang-writer.h"

namespace Slang
{

enum class DebugMessageType
{
    Info,
    Warning,
    Error
};

enum class DebugMessageSource
{
    Layer,
    Driver,
    Slang
};

class IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
    handleMessage(DebugMessageType type, DebugMessageSource source, const char* message) = 0;
};

/* Holds standard writers for the channels */
class StdWriters : public RefObject
{
public:
    ISlangWriter* getWriter(SlangWriterChannel chan) const { return m_writers[chan]; }
    void setWriter(SlangWriterChannel chan, ISlangWriter* writer) { m_writers[chan] = writer; }

    IDebugCallback* getDebugCallback() const { return m_debugCallback; }
    void setDebugCallback(IDebugCallback* callback) { m_debugCallback = callback; }

    /// Flush all the set writers
    void flushWriters();

    /// Ctor
    StdWriters() {}

    /// Initialize a default context
    static RefPtr<StdWriters> createDefault();
    static RefPtr<StdWriters> initDefaultSingleton();

    static StdWriters* getSingleton() { return s_singleton; }
    static void setSingleton(StdWriters* context) { s_singleton = context; }

    static WriterHelper getError()
    {
        return getSingleton()->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR);
    }
    static WriterHelper getOut()
    {
        return getSingleton()->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT);
    }
    static WriterHelper getDiagnostic()
    {
        return getSingleton()->getWriter(SLANG_WRITER_CHANNEL_DIAGNOSTIC);
    }

protected:
    ComPtr<ISlangWriter> m_writers[SLANG_WRITER_CHANNEL_COUNT_OF];
    IDebugCallback* m_debugCallback = nullptr;

    static StdWriters* s_singleton;
};

// --------------------------------------------------------------------------
inline void StdWriters::flushWriters()
{
    for (Index i = 0; i < Count(SLANG_WRITER_CHANNEL_COUNT_OF); ++i)
    {
        auto writer = m_writers[i];
        if (writer)
        {
            writer->flush();
        }
    }
}

} // namespace Slang

#endif
