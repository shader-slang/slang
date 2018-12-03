#ifndef SLANG_WRITE_STREAM_H
#define SLANG_WRITE_STREAM_H

#include "slang-string.h"
#include "stream.h"
#include "text-io.h"

#include <stdarg.h>

namespace Slang
{

// I could use an interface to specify the memory stream
class WriteStream
{
public:

    virtual void printVaList(const char* format, va_list args) = 0;
    virtual void put(const char* text, size_t numChars) = 0;
    virtual void flush() = 0;
    virtual bool isFile() = 0;

    void print(const char* format, ...);
    void put(const char* text) { put(text, ::strlen(text)); }
};

class FileWriteStream : public WriteStream
{
public:
        // WriteStream interface
    virtual void printVaList(const char* format, va_list args) SLANG_OVERRIDE;
    virtual void put(const char* text, size_t numChars) SLANG_OVERRIDE;
    virtual void flush() SLANG_OVERRIDE;
    virtual bool isFile() SLANG_OVERRIDE { return true;  }

        /// Ctor
    FileWriteStream(FILE* file, bool isOwned) :
        m_file(file),
        m_isOwned(isOwned)
    {}

        /// Dtor
    ~FileWriteStream();

    bool m_isOwned;
    FILE* m_file;
};


class StringWriteStream : public WriteStream
{
public:

    // WriteStream interface
    virtual void printVaList(const char* format, va_list args) SLANG_OVERRIDE;
    virtual void put(const char* text, size_t numChars) SLANG_OVERRIDE;
    virtual void flush() SLANG_OVERRIDE;
    virtual bool isFile() SLANG_OVERRIDE { return false; }

        /// Ctor
    StringWriteStream(StringBuilder* builder) :
        m_builder(builder)
    {}

protected:
    StringBuilder* m_builder;
};

}

#endif // SLANG_TEXT_WRITER_H
