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
/// An append buffer that the current thread has begun on `writer` and not yet ended.
struct ActiveAppendBuffer
{
    const AppendBufferWriter* writer = nullptr;
    List<char> buffer;
};

struct ThreadAppendBuffers
{
    /// The append buffers this thread has begun and not yet ended. Each writer allows one active
    /// append buffer, so there is at most one entry per writer. Each entry's characters live in
    /// their own allocation, so adding an entry for another writer does not move a buffer that
    /// has already been returned to a caller.
    List<ActiveAppendBuffer> active;

    /// The storage of the most recently ended append buffer, which the next begin reuses so that
    /// repeated begin/end pairs do not allocate.
    List<char> spare;

    Index find(const AppendBufferWriter* writer) const
    {
        return active.findFirstIndex([&](const ActiveAppendBuffer& activeAppendBuffer)
                                     { return activeAppendBuffer.writer == writer; });
    }
};
} // namespace

static ThreadAppendBuffers& _getThreadAppendBuffers()
{
    thread_local ThreadAppendBuffers threadAppendBuffers;
    return threadAppendBuffers;
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

    List<char>& buffer = appendBuffers.active[index].buffer;
    buffer.setCount(maxNumChars);
    return buffer.getBuffer();
}

SLANG_NO_THROW SlangResult SLANG_MCALL
AppendBufferWriter::endAppendBuffer(char* buffer, size_t numChars)
{
    auto& appendBuffers = _getThreadAppendBuffers();
    const Index index = appendBuffers.find(this);
    if (index < 0)
    {
        // The append buffer belongs to the thread that began it, so an end without a begin on
        // this thread is reported rather than writing from memory this thread does not own.
        return SLANG_E_INVALID_ARG;
    }

    // The buffer is detached before calling write, so that a write that appends to this same
    // writer begins a new buffer instead of overwriting the one being written.
    List<char> appendBuffer;
    appendBuffer.swapWith(appendBuffers.active[index].buffer);
    appendBuffers.active.fastRemoveAt(index);

    SLANG_ASSERT(appendBuffer.getBuffer() == buffer && buffer + numChars <= appendBuffer.end());
    const SlangResult res = write(buffer, numChars);

    appendBuffer.clear();
    appendBuffers.spare.swapWith(appendBuffer);
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
