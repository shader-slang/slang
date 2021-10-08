#ifndef SLANG_CORE_TEXT_IO_H
#define SLANG_CORE_TEXT_IO_H

#include "slang-secure-crt.h"
#include "slang-stream.h"

namespace Slang
{
	using Slang::List;
	using Slang::_EndLine;

	class TextReader
	{
	protected:
		char decodedChar[5];
		int decodedCharPtr = 0, decodedCharSize = 0;
		virtual void ReadChar() = 0;
	public:
		virtual ~TextReader()
		{
			Close();
		}
		virtual void Close(){}
		virtual String ReadLine()=0;
		virtual String ReadToEnd()=0;
		virtual bool IsEnd() = 0;
		int Read(char * buffer, int count);
		char Read()
		{
			if (decodedCharPtr == decodedCharSize)
				ReadChar();
			if (decodedCharPtr < decodedCharSize)
				return decodedChar[decodedCharPtr++];
			else
				return 0;
		}
		char Peak()
		{
			if (decodedCharPtr == decodedCharSize)
				ReadChar();
			if (decodedCharPtr < decodedCharSize)
				return decodedChar[decodedCharPtr];
			else
				return 0;
		}
	};

	class TextWriter
	{
	public:
		virtual ~TextWriter()
		{
			Close();
		}
		virtual void Write(const String & str)=0;
		virtual void Write(const char * str)=0;
		virtual void Close(){}
		template<typename T>
		TextWriter & operator << (const T& val)
		{
			Write(val.ToString());
			return *this;
		}
		TextWriter & operator << (int value)
		{
			Write(String(value));
			return *this;
		}
		TextWriter & operator << (float value)
		{
			Write(String(value));
			return *this;
		}
		TextWriter & operator << (double value)
		{
			Write(String(value));
			return *this;
		}
		TextWriter & operator << (const char* value)
		{
			Write(value);
			return *this;
		}
		TextWriter & operator << (const String & val)
		{
			Write(val);
			return *this;
		}
		TextWriter & operator << (const _EndLine &)
		{
#ifdef _WIN32
			Write("\r\n");
#else
			Write("\n");
#endif
			return *this;
		}
	};

	template <typename ReadCharFunc>
	int GetUnicodePointFromUTF8(const ReadCharFunc & get)
	{
		int codePoint = 0;
		int leading = get(0);
		int mask = 0x80;
		int count = 0;
		while (leading & mask)
		{
			count++;
			mask >>= 1;
		}
		codePoint = (leading & (mask - 1));
		for (int i = 1; i <= count - 1; i++)
		{
			codePoint <<= 6;
			codePoint += (get(i) & 0x3F);
		}
		return codePoint;
	}

	template <typename ReadCharFunc>
	int GetUnicodePointFromUTF16(const ReadCharFunc & get)
	{
		int byte0 = (unsigned char)get(0);
		int byte1 = (unsigned char)get(1);
		int word0 = byte0 + (byte1 << 8);
		if (word0 >= 0xD800 && word0 <= 0xDFFF)
		{
			int byte2 = (unsigned char)get(2);
			int byte3 = (unsigned char)get(3);
			int word1 = byte2 + (byte3 << 8);
			return ((word0 & 0x3FF) << 10) + (word1 & 0x3FF) + 0x10000;
		}
		else
			return word0;
	}

	template <typename ReadCharFunc>
	int GetUnicodePointFromUTF16Reversed(const ReadCharFunc & get)
	{
		int byte0 = (unsigned char)get(0);
		int byte1 = (unsigned char)get(1);
		int word0 = (byte0 << 8) + byte1;
		if (word0 >= 0xD800 && word0 <= 0xDFFF)
		{
			int byte2 = (unsigned char)get(2);
			int byte3 = (unsigned char)get(3);
			int word1 = (byte2 << 8) + byte3;
			return ((word0 & 0x3FF) << 10) + (word1 & 0x3FF);
		}
		else
			return word0;
	}

	template <typename ReadCharFunc>
	int GetUnicodePointFromUTF32(const ReadCharFunc & get)
	{
		int byte0 = (unsigned char)get(0);
		int byte1 = (unsigned char)get(1);
		int byte2 = (unsigned char)get(2);
		int byte3 = (unsigned char)get(3);
		return byte0 + (byte1 << 8) + (byte2 << 16) + (byte3 << 24);
	}

	inline int EncodeUnicodePointToUTF8(char * buffer, int codePoint)
	{
		int count = 0;
		if (codePoint <= 0x7F)
			buffer[count++] = ((char)codePoint);
		else if (codePoint <= 0x7FF)
		{
			unsigned char byte = (unsigned char)(0xC0 + (codePoint >> 6));
			buffer[count++] = ((char)byte);
			byte = 0x80 + (codePoint & 0x3F);
			buffer[count++] = ((char)byte);
		}
		else if (codePoint <= 0xFFFF)
		{
			unsigned char byte = (unsigned char)(0xE0 + (codePoint >> 12));
			buffer[count++] = ((char)byte);
			byte = (unsigned char)(0x80 + ((codePoint >> 6) & (0x3F)));
			buffer[count++] = ((char)byte);
			byte = (unsigned char)(0x80 + (codePoint & 0x3F));
			buffer[count++] = ((char)byte);
		}
		else
		{
			unsigned char byte = (unsigned char)(0xF0 + (codePoint >> 18));
			buffer[count++] = ((char)byte);
			byte = (unsigned char)(0x80 + ((codePoint >> 12) & 0x3F));
			buffer[count++] = ((char)byte);
			byte = (unsigned char)(0x80 + ((codePoint >> 6) & 0x3F));
			buffer[count++] = ((char)byte);
			byte = (unsigned char)(0x80 + (codePoint & 0x3F));
			buffer[count++] = ((char)byte);
		}
		return count;
	}

	inline int EncodeUnicodePointToUTF16(unsigned short * buffer, int codePoint)
	{
		int count = 0;
		if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
			buffer[count++] = (unsigned short)codePoint;
		else
		{
			int sub = codePoint - 0x10000;
			int high = (sub >> 10) + 0xD800;
			int low = (sub & 0x3FF) + 0xDC00;
			buffer[count++] = (unsigned short)high;
			buffer[count++] = (unsigned short)low;
		}
		return count;
	}

	inline unsigned short ReverseBitOrder(unsigned short val)
	{
		int byte0 = val & 0xFF;
		int byte1 = val >> 8;
		return (unsigned short)(byte1 + (byte0 << 8));
	}

	inline int EncodeUnicodePointToUTF16Reversed(unsigned short * buffer, int codePoint)
	{
		int count = 0;
		if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
			buffer[count++] = ReverseBitOrder((unsigned short)codePoint);
		else
		{
			int sub = codePoint - 0x10000;
			int high = (sub >> 10) + 0xD800;
			int low = (sub & 0x3FF) + 0xDC00;
			buffer[count++] = ReverseBitOrder((unsigned short)high);
			buffer[count++] = ReverseBitOrder((unsigned short)low);
		}
		return count;
	}

	class Encoding
	{
	public:
		static Encoding * UTF8, * UTF16, *UTF16Reversed, * UTF32;
		virtual void GetBytes(List<char>& buffer, const String & str) = 0;
		virtual String ToString(const char * buffer, int length) = 0;
		virtual ~Encoding()
		{}
	};

	class StreamWriter : public TextWriter
	{
	private:
		List<char> encodingBuffer;
		RefPtr<Stream> stream;
		Encoding * encoding;
	public:
		StreamWriter(const String & path, Encoding * encoding = Encoding::UTF8);
		StreamWriter(RefPtr<Stream> stream, Encoding * encoding = Encoding::UTF8);
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
		Encoding * encoding;
		Index ptr;
		char ReadBufferChar();
		void ReadBuffer();
			
		Encoding * DetermineEncoding();
	protected:
		virtual void ReadChar()
		{
			decodedCharPtr = 0;
			int codePoint = 0;
			if (encoding == Encoding::UTF8)
				codePoint = GetUnicodePointFromUTF8([&](int) {return ReadBufferChar(); });
			else if (encoding == Encoding::UTF16)
				codePoint = GetUnicodePointFromUTF16([&](int) {return ReadBufferChar(); });
			else if (encoding == Encoding::UTF16Reversed)
				codePoint = GetUnicodePointFromUTF16Reversed([&](int) {return ReadBufferChar(); });
			else if (encoding == Encoding::UTF32)
				codePoint = GetUnicodePointFromUTF32([&](int) {return ReadBufferChar(); });
			decodedCharSize = EncodeUnicodePointToUTF8(decodedChar, codePoint);
		}
	public:
		StreamReader(const String & path);
		StreamReader(RefPtr<Stream> stream, Encoding * encoding = nullptr);
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
