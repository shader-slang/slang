#define _CRT_SECURE_NO_WARNINGS

#include "slang-writer.h"

#include "slang-platform.h"
#include "slang-string-util.h"

// Includes to allow us to control console
// output when writing assembly dumps.
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdarg.h>

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!! WriterHelper !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult WriterHelper::print(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SlangResult res = SLANG_OK;

    // numChars is the amount of characters needed *not* including terminating 0
    size_t numChars;
    {
        // Create a copy of args, as will be consumed by calcFormattedSize
        va_list argsCopy;
        va_copy(argsCopy, args);
        numChars = StringUtil::calcFormattedSize(format, argsCopy);
        va_end(argsCopy);
    }

    if (numChars > 0)
    {
        // We need to add 1 here, because calcFormatted, *requires* space for terminating 0
        char* appendBuffer = m_writer->beginAppendBuffer(numChars + 1);
        StringUtil::calcFormatted(format, args, numChars, appendBuffer);
        res = m_writer->endAppendBuffer(appendBuffer, numChars);
    }

    va_end(args);
    return res;
}

SlangResult WriterHelper::put(const char* text)
{
    return m_writer->write(text, ::strlen(text));
}

SlangResult WriterHelper::put(const UnownedStringSlice& text)
{
    return m_writer->write(text.begin(), text.getLength());
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! BaseWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

ISlangUnknown* BaseWriter::getInterface(const Guid& guid)
{
    return (guid == ISlangUnknown::getTypeGuid() || guid == ISlangWriter::getTypeGuid())
               ? static_cast<ISlangWriter*>(this)
               : nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! AppendBufferWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

namespace
{
/// An append buffer that the current thread has begun on `writer` and not yet ended. `writer`
/// is only compared, never dereferenced. A writer's destructor removes the destroying thread's
/// entry for it. An entry abandoned on another thread stays until that thread exits, and a later
/// writer at the same address takes it over harmlessly, because its next begin resizes it.
struct ActiveAppendBuffer
{
    const AppendBufferWriter* writer = nullptr;
    List<char> buffer;
};

/// The append buffers of one thread. `_getThreadAppendBuffers` returns the calling thread's
/// instance, including for calls made after its thread_local storage has been destroyed.
struct ThreadAppendBuffers
{
    /// The append buffers this thread has begun and not yet ended. Each writer allows one active
    /// append buffer, so there is at most one entry per writer. Each entry's characters live in
    /// their own allocation, so adding an entry for another writer does not move a buffer that
    /// has already been returned to a caller.
    List<ActiveAppendBuffer> active;

    /// The storage of the most recently ended append buffer, if it is small, which the next begin
    /// reuses so that repeated begin/end pairs do not allocate.
    List<char> spare;

    /// Returns the index in `active` of the entry for `writer`, or -1 if there is none.
    Index find(const AppendBufferWriter* writer) const
    {
        return active.findFirstIndex([&](const ActiveAppendBuffer& activeAppendBuffer)
                                     { return activeAppendBuffer.writer == writer; });
    }
};

// A thread's append buffers live in a thread_local holder from its first use until thread exit.
// A destructor that runs after that on the same thread (a static object's at process exit, or
// another thread_local's) can still print to a writer, so it gets a separate heap instance
// instead. These variables are trivially destructible, so they stay usable that late.

/// Set when this thread's holder has been constructed, so a writer's destructor can find the
/// buffers without creating them during teardown.
thread_local bool gIsThreadAppendBuffersCreated = false;
thread_local bool gIsThreadAppendBuffersDestroyed = false;
/// The heap instance used after the holder has been destroyed, or null.
thread_local ThreadAppendBuffers* gLateThreadAppendBuffers = nullptr;

struct ThreadAppendBuffersHolder
{
    ThreadAppendBuffers buffers;
    ThreadAppendBuffersHolder() { gIsThreadAppendBuffersCreated = true; }
    ~ThreadAppendBuffersHolder() { gIsThreadAppendBuffersDestroyed = true; }
};
} // namespace

static ThreadAppendBuffers& _getThreadAppendBuffers()
{
    if (!gIsThreadAppendBuffersDestroyed)
    {
        thread_local ThreadAppendBuffersHolder holder;
        return holder.buffers;
    }
    // The instance is freed once it holds no append buffer: by the end of its last buffer, by an
    // end without a begin, or by a writer's destructor. A late begin that is never ended leaves
    // it allocated, because no later point on this thread could free it.
    if (!gLateThreadAppendBuffers)
        gLateThreadAppendBuffers = new ThreadAppendBuffers();
    return *gLateThreadAppendBuffers;
}

/// Returns the calling thread's buffers if it has any, without creating them.
static ThreadAppendBuffers* _findThreadAppendBuffers()
{
    if (gIsThreadAppendBuffersDestroyed)
        return gLateThreadAppendBuffers;
    return gIsThreadAppendBuffersCreated ? &_getThreadAppendBuffers() : nullptr;
}

static void _releaseLateThreadAppendBuffersIfIdle()
{
    if (gLateThreadAppendBuffers && gLateThreadAppendBuffers->active.getCount() == 0)
    {
        delete gLateThreadAppendBuffers;
        gLateThreadAppendBuffers = nullptr;
    }
}

AppendBufferWriter::~AppendBufferWriter()
{
    // A buffer this thread began and abandoned on this writer is freed with the writer. Buffers
    // abandoned on other threads stay until those threads exit.
    if (auto appendBuffers = _findThreadAppendBuffers())
    {
        const Index index = appendBuffers->find(this);
        if (index >= 0)
        {
            appendBuffers->active[index].buffer.clearAndDeallocate();
            appendBuffers->active.fastRemoveAt(index);
        }
        _releaseLateThreadAppendBuffersIfIdle();
    }
}

SLANG_NO_THROW char* SLANG_MCALL AppendBufferWriter::beginAppendBuffer(size_t maxNumChars)
{
    auto& appendBuffers = _getThreadAppendBuffers();

    // If an earlier append buffer on this writer was never ended, which `ISlangWriter` allows when
    // there are no characters to write, its entry is reused.
    Index index = appendBuffers.find(this);
    if (index < 0)
    {
        index = appendBuffers.active.getCount();
        appendBuffers.active.add(ActiveAppendBuffer{this, List<char>()});
        appendBuffers.active[index].buffer.swapWith(appendBuffers.spare);
    }

    List<char>& appendBuffer = appendBuffers.active[index].buffer;
    appendBuffer.setCount(maxNumChars);
    return appendBuffer.getBuffer();
}

SLANG_NO_THROW SlangResult SLANG_MCALL
AppendBufferWriter::endAppendBuffer(char* buffer, size_t numChars)
{
    auto& appendBuffers = _getThreadAppendBuffers();
    const Index index = appendBuffers.find(this);

    // The append buffer belongs to the thread that began it. An end on another thread, with a
    // pointer other than the one begin returned, or with more characters than were reserved, is
    // reported rather than writing from memory this thread does not own. The other thread may
    // have an abandoned buffer of its own on this writer, so finding an entry is not enough.
    if (index < 0 || appendBuffers.active[index].buffer.getBuffer() != buffer ||
        numChars > size_t(appendBuffers.active[index].buffer.getCount()))
    {
        _releaseLateThreadAppendBuffersIfIdle();
        return SLANG_E_INVALID_ARG;
    }

    // The buffer is detached before calling write, so that a write that appends to this same
    // writer begins a new buffer instead of overwriting the one being written.
    List<char> appendBuffer;
    appendBuffer.swapWith(appendBuffers.active[index].buffer);
    appendBuffers.active.fastRemoveAt(index);

    const SlangResult res = write(buffer, numChars);

    // Only a small buffer is kept for reuse, so one large append does not hold its memory for
    // the rest of the thread's life.
    const Index kMaxSpareCapacity = 4096;
    if (appendBuffer.getCapacity() <= kMaxSpareCapacity)
    {
        appendBuffer.clear();
        // During teardown, an append to any writer from inside `write` can end with the late
        // instance freed, so the buffers are looked up again instead of reusing `appendBuffers`.
        _getThreadAppendBuffers().spare.swapWith(appendBuffer);
    }
    _releaseLateThreadAppendBuffersIfIdle();
    return res;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! CallbackWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SLANG_NO_THROW char* SLANG_MCALL CallbackWriter::beginAppendBuffer(size_t maxNumChars)
{
    // Add one so there is always space for end termination, we need for the callback.
    m_appendBuffer.setCount(maxNumChars + 1);
    return m_appendBuffer.getBuffer();
}

SLANG_NO_THROW SlangResult SLANG_MCALL
CallbackWriter::endAppendBuffer(char* buffer, size_t numChars)
{
    SLANG_ASSERT(m_appendBuffer.getBuffer() == buffer && buffer + numChars <= m_appendBuffer.end());
    SlangResult res = write(buffer, numChars);
    // Clear so that buffer can't be written from again without assert
    m_appendBuffer.clear();
    return res;
}

SlangResult CallbackWriter::write(const char* chars, size_t numChars)
{
    if (numChars > 0)
    {
        char* appendBuffer = m_appendBuffer.getBuffer();
        // See if it's from an append buffer
        if (chars >= appendBuffer &&
            (chars + numChars) < (appendBuffer + m_appendBuffer.getCount()))
        {
            // Set terminating 0
            appendBuffer[(chars + numChars) - appendBuffer] = 0;

            m_callback(chars, (void*)m_data);
        }
        else
        {
            // Use the append buffer to add the terminating 0
            m_appendBuffer.setCount(numChars + 1);
            ::memcpy(m_appendBuffer.getBuffer(), chars, numChars);
            m_appendBuffer[numChars] = 0;

            m_callback(m_appendBuffer.getBuffer(), (void*)m_data);
        }
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! FileWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

FileWriter::~FileWriter()
{
    if (m_file)
    {
        ::fflush(m_file);

        if ((m_flags & WriterFlag::IsUnowned) == 0)
        {
            ::fclose(m_file);
        }
    }
}

// A single fwrite per call makes this safe to call from several threads, because the C library
// locks the stream for each call.
SlangResult FileWriter::write(const char* text, size_t numChars)
{
    const size_t numWritten = ::fwrite(text, sizeof(char), numChars, m_file);
    if (m_flags & WriterFlag::AutoFlush)
    {
        ::fflush(m_file);
    }
    return numChars == numWritten ? SLANG_OK : SLANG_FAIL;
}

void FileWriter::flush()
{
    ::fflush(m_file);
}

/* static */ bool FileWriter::isFileConsole(FILE* file)
{
    const int stdoutFileDesc = _fileno(file);
    return _isatty(stdoutFileDesc) != 0;
}

SlangResult FileWriter::setMode(SlangWriterMode mode)
{
    switch (mode)
    {
    case SLANG_WRITER_MODE_BINARY:
        {
#ifdef _WIN32
            int stdoutFileDesc = _fileno(m_file);
            _setmode(stdoutFileDesc, _O_BINARY);
            return SLANG_OK;
#else
            break;
#endif
        }
    default:
        break;
    }
    return SLANG_FAIL;
}

/* static */ SlangResult FileWriter::create(
    const char* filePath,
    const char* writeOptions,
    WriterFlags flags,
    ComPtr<ISlangWriter>& outWriter)
{
    flags &= ~WriterFlag::IsUnowned;

    FILE* file = nullptr;
    if (fopen_s(&file, filePath, writeOptions) != 0 || !file)
    {
        return SLANG_E_CANNOT_OPEN;
    }

    outWriter = new FileWriter(file, flags);
    return SLANG_OK;
}


/* !!!!!!!!!!!!!!!!!!!!!!!!! StringWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SLANG_NO_THROW char* SLANG_MCALL StringWriter::beginAppendBuffer(size_t maxNumChars)
{
    return m_builder->prepareForAppend(maxNumChars);
}

SLANG_NO_THROW SlangResult SLANG_MCALL StringWriter::endAppendBuffer(char* buffer, size_t numChars)
{
    m_builder->appendInPlace(buffer, numChars);
    return SLANG_OK;
}

SlangResult StringWriter::write(const char* chars, size_t numChars)
{
    if (numChars > 0)
    {
        m_builder->append(chars, numChars);
    }
    return SLANG_OK;
}

} // namespace Slang
