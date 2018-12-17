#include "slang-writer.h"

#include "platform.h"
#include "slang-string-util.h"

// Includes to allow us to control console
// output when writing assembly dumps.
#include <fcntl.h>
#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#endif

#include <stdarg.h>

namespace Slang
{
static const Guid IID_ISlangWriter = SLANG_UUID_ISlangWriter;
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;

/* !!!!!!!!!!!!!!!!!!!!!!!!! WriterHelper !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult WriterHelper::print(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SlangResult res = SLANG_OK;

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
        char* appendBuffer = m_writer->beginAppendBuffer(numChars);
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

/* !!!!!!!!!!!!!!!!!!!!!!!!! BaseWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

ISlangUnknown* BaseWriter::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangWriter) ? static_cast<ISlangWriter*>(this) : nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! AppendBufferWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SLANG_NO_THROW char* SLANG_MCALL AppendBufferWriter::beginAppendBuffer(size_t maxNumChars)
{
    m_appendBuffer.SetSize(maxNumChars);
    return m_appendBuffer.Buffer();
}

SLANG_NO_THROW SlangResult SLANG_MCALL AppendBufferWriter::endAppendBuffer(char* buffer, size_t numChars)
{
    SLANG_ASSERT(m_appendBuffer.Buffer() == buffer && buffer + numChars <= m_appendBuffer.end());
    // Do the actual write
    SlangResult res = write(buffer, numChars);
    // Clear so that buffer can't be written from again without assert
    m_appendBuffer.Clear();
    return res;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! CallbackWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult CallbackWriter::write(const char* chars, size_t numChars)
{
    if (numChars > 0)
    {
        // Make sure zero terminated
        StringBuilder builder;
        builder.Append(chars, numChars);

        m_callback(builder.Buffer(), (void*)m_data);
    }

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! FileWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

FileWriter::~FileWriter()
{
    if ((m_flags & WriterFlag::IsUnowned) == 0)
    {
        fclose(m_file);
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

/* static */bool FileWriter::isConsole(FILE* file)
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
    default: break;
    }
    return SLANG_FAIL;
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
        m_builder->Append(chars, numChars);
    }
    return SLANG_OK;
}

}

