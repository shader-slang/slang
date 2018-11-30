#include "slang-write-stream.h"

#include "slang-string-util.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!! WriteStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

void WriteStream::print(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    printVaList(format, args);
    va_end(args);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! FileWriteStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

FileWriteStream::~FileWriteStream()
{
    if (m_isOwned)
    {
        fclose(m_file);
    }
}

void FileWriteStream::printVaList(const char* format, va_list args)
{
    // http://www.cplusplus.com/reference/cstdio/vprintf/
    vprintf(format, args);
}

void FileWriteStream::put(const char* text, size_t numChars)
{
    ::fwrite(text, sizeof(char), numChars, m_file);
}

void FileWriteStream::flush()
{
    ::fflush(m_file);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! MemoryWriteStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

void StringWriteStream::printVaList(const char* format, va_list args)
{
    StringUtil::append(format, args, *m_builder);
}

void StringWriteStream::put(const char* chars, size_t numChars)
{
    if (numChars > 0)
    {
        m_builder->Append(chars, numChars);
    }
}

void StringWriteStream::flush()
{
}

}

