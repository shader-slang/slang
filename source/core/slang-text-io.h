#ifndef SLANG_CORE_TEXT_IO_H
#define SLANG_CORE_TEXT_IO_H

#include "slang-secure-crt.h"
#include "slang-stream.h"
#include "slang-char-encode.h"

namespace Slang
{
using Slang::List;
using Slang::_EndLine;

class TextReader
{
public:	
	virtual void close(){}
	virtual String readToEnd()=0;
	virtual bool isEnd() = 0;

	int read(char* buffer, int count);

	char read()
	{
		if (m_decodedCharIndex == m_decodedCharSize)
			readChar();
		if (m_decodedCharIndex < m_decodedCharSize)
			return m_decodedChar[m_decodedCharIndex++];
		else
			return 0;
	}
	char peek()
	{
		if (m_decodedCharIndex == m_decodedCharSize)
			readChar();
		if (m_decodedCharIndex < m_decodedCharSize)
			return m_decodedChar[m_decodedCharIndex];
		else
			return 0;
	}

    virtual ~TextReader() { close(); }

protected:
    char m_decodedChar[5];
    Index m_decodedCharIndex = 0;
    Index m_decodedCharSize = 0;

    virtual void readChar() = 0;
};


class StreamReader : public TextReader
{
public:
    virtual String readToEnd() SLANG_OVERRIDE;
    virtual bool isEnd() SLANG_OVERRIDE { return m_index == m_buffer.getCount() && m_stream->isEnd(); }
    virtual void close() { m_stream->close(); }

    void releaseStream() { m_stream.setNull(); }

    StreamReader(const String& path);
    StreamReader(RefPtr<Stream> stream, CharEncoding* encoding = nullptr);

protected:
    virtual void readChar() SLANG_OVERRIDE
    {
        m_decodedCharIndex = 0;

        Char32 codePoint = 0;
        if (m_encoding == CharEncoding::UTF8)
            codePoint = getUnicodePointFromUTF8([&](Index) {return readBufferChar(); });
        else if (m_encoding == CharEncoding::UTF16)
            codePoint = getUnicodePointFromUTF16([&](Index) {return readBufferChar(); });
        else if (m_encoding == CharEncoding::UTF16Reversed)
            codePoint = getUnicodePointFromUTF16Reversed([&](Index) {return readBufferChar(); });
        else if (m_encoding == CharEncoding::UTF32)
            codePoint = getUnicodePointFromUTF32([&](Index) {return readBufferChar(); });

        m_decodedCharSize = encodeUnicodePointToUTF8(codePoint, m_decodedChar);
    }

private:
    char readBufferChar();
    void readBuffer();
    CharEncoding* determineEncoding();

    RefPtr<Stream> m_stream;
    List<char> m_buffer;
    CharEncoding* m_encoding;
    Index m_index;                  ///< Index into buffer
};

class TextWriter
{
public:
		
	virtual void writeSlice(const UnownedStringSlice& slice) = 0;
	virtual void close(){}

    void write(const UnownedStringSlice& slice) { writeSlice(slice); }
    void write(const char* str) { writeSlice(UnownedStringSlice(str)); }
    void write(const String& str) { writeSlice(str.getUnownedSlice()); }

    virtual ~TextWriter() { close(); }

	template<typename T>
	TextWriter& operator << (const T& val)
	{
		write(val.ToString());
		return *this;
	}
	TextWriter& operator << (int value)
	{
		write(String(value));
		return *this;
	}
	TextWriter& operator << (float value)
	{
		write(String(value));
		return *this;
	}
	TextWriter& operator << (double value)
	{
		write(String(value));
		return *this;
	}
	TextWriter& operator << (const char* value)
	{
		writeSlice(UnownedStringSlice(value));
		return *this;
	}
	TextWriter& operator << (const String & val)
	{
		writeSlice(val.getUnownedSlice());
		return *this;
	}
	TextWriter& operator << (const _EndLine &)
	{
#ifdef _WIN32
		writeSlice(UnownedStringSlice::fromLiteral("\r\n"));
#else
		writeSlice(UnownedStringSlice::fromLiteral("\n"));
#endif
		return *this;
	}
};
 
class StreamWriter : public TextWriter
{
public:
    // TextWriter		
    virtual void writeSlice(const UnownedStringSlice& slice) SLANG_OVERRIDE;
    virtual void close() SLANG_OVERRIDE { m_stream->close(); }

    void releaseStream() { m_stream.setNull();  }

    StreamWriter(const String& path, CharEncoding* encoding = CharEncoding::UTF8);
    StreamWriter(RefPtr<Stream> stream, CharEncoding* encoding = CharEncoding::UTF8);

private:
    List<Byte> m_encodingBuffer;
    RefPtr<Stream> m_stream;
    CharEncoding* m_encoding;
};

}

#endif
