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
		virtual ~TextReader()
		{
			Close();
		}

		virtual void Close(){}
		virtual String ReadLine()=0;
		virtual String ReadToEnd()=0;
		virtual bool IsEnd() = 0;

		int Read(char* buffer, int count);

		char Read()
		{
			if (m_decodedCharIndex == m_decodedCharSize)
				ReadChar();
			if (m_decodedCharIndex < m_decodedCharSize)
				return m_decodedChar[m_decodedCharIndex++];
			else
				return 0;
		}
		char peek()
		{
			if (m_decodedCharIndex == m_decodedCharSize)
				ReadChar();
			if (m_decodedCharIndex < m_decodedCharSize)
				return m_decodedChar[m_decodedCharIndex];
			else
				return 0;
		}

    protected:
        char m_decodedChar[5];
        Index m_decodedCharIndex = 0;
        Index m_decodedCharSize = 0;
        virtual void ReadChar() = 0;
	};

	class TextWriter
	{
	public:
		virtual ~TextWriter()
		{
			Close();
		}
		virtual void Write(const String & str)=0;
		virtual void Write(const char* str)=0;
		virtual void Close(){}

		template<typename T>
		TextWriter& operator << (const T& val)
		{
			Write(val.ToString());
			return *this;
		}
		TextWriter& operator << (int value)
		{
			Write(String(value));
			return *this;
		}
		TextWriter& operator << (float value)
		{
			Write(String(value));
			return *this;
		}
		TextWriter& operator << (double value)
		{
			Write(String(value));
			return *this;
		}
		TextWriter& operator << (const char* value)
		{
			Write(value);
			return *this;
		}
		TextWriter& operator << (const String & val)
		{
			Write(val);
			return *this;
		}
		TextWriter& operator << (const _EndLine &)
		{
#ifdef _WIN32
			Write("\r\n");
#else
			Write("\n");
#endif
			return *this;
		}
	};
 
	class StreamWriter : public TextWriter
	{
	private:
		List<char> encodingBuffer;
		RefPtr<Stream> stream;
		CharEncoding * encoding;
	public:
		StreamWriter(const String & path, CharEncoding* encoding = CharEncoding::UTF8);
		StreamWriter(RefPtr<Stream> stream, CharEncoding* encoding = CharEncoding::UTF8);
		virtual void Write(const String & str);
		virtual void Write(const char * str);
		virtual void Close()
		{
			stream->close();
		}
        void ReleaseStream()
        {
            stream = 0;
        }
	};

    class StreamReader : public TextReader
	{
	private:
		RefPtr<Stream> stream;
		List<char> buffer;
		CharEncoding * encoding;
		Index ptr;
		char ReadBufferChar();
		void ReadBuffer();
			
		CharEncoding* DetermineEncoding();
	protected:
		virtual void ReadChar()
		{
			m_decodedCharIndex = 0;
			int codePoint = 0;
			if (encoding == CharEncoding::UTF8)
				codePoint = GetUnicodePointFromUTF8([&](int) {return ReadBufferChar(); });
			else if (encoding == CharEncoding::UTF16)
				codePoint = GetUnicodePointFromUTF16([&](int) {return ReadBufferChar(); });
			else if (encoding == CharEncoding::UTF16Reversed)
				codePoint = GetUnicodePointFromUTF16Reversed([&](int) {return ReadBufferChar(); });
			else if (encoding == CharEncoding::UTF32)
				codePoint = GetUnicodePointFromUTF32([&](int) {return ReadBufferChar(); });
			m_decodedCharSize = EncodeUnicodePointToUTF8(m_decodedChar, codePoint);
		}
	public:
		StreamReader(const String & path);
		StreamReader(RefPtr<Stream> stream, CharEncoding* encoding = nullptr);
		virtual String ReadLine();
		virtual String ReadToEnd();
		virtual bool IsEnd()
		{
			return ptr == buffer.getCount() && stream->isEnd();
		}
		virtual void Close()
		{
			stream->close();
		}
        void ReleaseStream()
        {
            stream = 0;
        }
	};
}

#endif
