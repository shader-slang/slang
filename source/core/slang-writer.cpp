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
    SlangResult res = m_writer->writeVaList(format, args);

    if (res == SLANG_E_NOT_IMPLEMENTED)
    {
        StringBuilder builder;
        StringUtil::append(format, args, builder);

        // Write if there is anything to write
        res = (builder.Length()) ? m_writer->write(builder.Buffer(), builder.Length()) : SLANG_OK;
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

SlangResult FileWriter::writeVaList(const char* format, va_list args)
{
    // http://www.cplusplus.com/reference/cstdio/vfprintf/
    ::vfprintf(m_file, format, args);

    if (m_flags & WriterFlag::AutoFlush)
    {
        ::fflush(m_file);
    }

    return SLANG_OK;
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

/* static */const bool FileWriter::isConsole(FILE* file)
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
    }
#endif
    default: break;
    }
    return SLANG_FAIL;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! StringWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

SlangResult StringWriter::writeVaList(const char* format, va_list args)
{
    StringUtil::append(format, args, *m_builder);
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

