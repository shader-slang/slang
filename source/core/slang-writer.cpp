#include "slang-writer.h"

#include "platform.h"

// Includes to allow us to control console
// output when writing assembly dumps.
#include <fcntl.h>
#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#endif

namespace Slang
{
static const Guid IID_ISlangWriter = SLANG_UUID_ISlangWriter;
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;

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
    if (m_flags & WriterFlag::IsOwned)
    {
        fclose(m_file);
    }
}

SlangResult FileWriter::write(const char* text, size_t numChars)
{
    const size_t numWritten = ::fwrite(text, sizeof(char), numChars, m_file);
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

SlangResult StringWriter::write(const char* chars, size_t numChars)
{
    if (numChars > 0)
    {
        m_builder->Append(chars, numChars);
    }
    return SLANG_OK;
}

}

