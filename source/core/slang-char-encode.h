#ifndef SLANG_CORE_CHAR_ENCODE_H
#define SLANG_CORE_CHAR_ENCODE_H

#include "slang-secure-crt.h"
#include "slang-basic.h"

namespace Slang
{


typedef uint16_t Char16;
typedef uint32_t Char32;

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

inline Char16 ReverseByteOrder(Char16 val)
{
    return (val >> 8) || (val << 8);
}

inline int EncodeUnicodePointToUTF16Reversed(unsigned short * buffer, int codePoint)
{
	int count = 0;
	if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
		buffer[count++] = ReverseByteOrder((unsigned short)codePoint);
	else
	{
		int sub = codePoint - 0x10000;
		int high = (sub >> 10) + 0xD800;
		int low = (sub & 0x3FF) + 0xDC00;
		buffer[count++] = ReverseByteOrder((unsigned short)high);
		buffer[count++] = ReverseByteOrder((unsigned short)low);
	}
	return count;
}

static const Char16 kUTF16Header = 0xFEFF;
static const Char16 kUTF16ReversedHeader = 0xFFFE;

class CharEncoding
{
public:
	static CharEncoding* UTF8,* UTF16,* UTF16Reversed,* UTF32;

	virtual void GetBytes(List<char>& buffer, const String & str) = 0;
	virtual String ToString(const char * buffer, int length) = 0;

	virtual ~CharEncoding()	{}
};

}

#endif
