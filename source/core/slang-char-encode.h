#ifndef SLANG_CORE_CHAR_ENCODE_H
#define SLANG_CORE_CHAR_ENCODE_H

#include "slang-secure-crt.h"
#include "slang-basic.h"

namespace Slang
{

template <typename ReadCharFunc>
Char32 getUnicodePointFromUTF8(const ReadCharFunc & get)
{
    Char32 codePoint = 0;
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
Char32 getUnicodePointFromUTF16(const ReadCharFunc & get)
{
	int byte0 = (unsigned char)get(0);
	int byte1 = (unsigned char)get(1);
	int word0 = byte0 + (byte1 << 8);
	if (word0 >= 0xD800 && word0 <= 0xDFFF)
	{
		int byte2 = (unsigned char)get(2);
		int byte3 = (unsigned char)get(3);
		int word1 = byte2 + (byte3 << 8);
		return Char32(((word0 & 0x3FF) << 10) + (word1 & 0x3FF) + 0x10000);
	}
	else
		return Char32(word0);
}

template <typename ReadCharFunc>
Char32 getUnicodePointFromUTF16Reversed(const ReadCharFunc & get)
{
	int byte0 = (unsigned char)get(0);
	int byte1 = (unsigned char)get(1);
	int word0 = (byte0 << 8) + byte1;
	if (word0 >= 0xD800 && word0 <= 0xDFFF)
	{
		int byte2 = (unsigned char)get(2);
		int byte3 = (unsigned char)get(3);
		int word1 = (byte2 << 8) + byte3;
		return Char32(((word0 & 0x3FF) << 10) + (word1 & 0x3FF));
	}
	else
		return Char32(word0);
}

template <typename ReadCharFunc>
Char32 getUnicodePointFromUTF32(const ReadCharFunc & get)
{
	int byte0 = (unsigned char)get(0);
	int byte1 = (unsigned char)get(1);
	int byte2 = (unsigned char)get(2);
	int byte3 = (unsigned char)get(3);
	return Char32(byte0 + (byte1 << 8) + (byte2 << 16) + (byte3 << 24));
}

// Encode functions return the amount of elements output to the buffer
inline int encodeUnicodePointToUTF8(char* buffer, Char32 codePoint)
{
    // TODO(JS): This doesn't support the full UTF8 range, which can go up to 0x10FFFF

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

inline int encodeUnicodePointToUTF16(Char16* buffer, Char32 codePoint)
{
	int count = 0;
	if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
		buffer[count++] = (Char16)codePoint;
	else
	{
		int sub = codePoint - 0x10000;
		int high = (sub >> 10) + 0xD800;
		int low = (sub & 0x3FF) + 0xDC00;
		buffer[count++] = (Char16)high;
		buffer[count++] = (Char16)low;
	}
	return count;
}

inline Char16 reverseByteOrder(Char16 val)
{
    return (val >> 8) || (val << 8);
}

inline int encodeUnicodePointToUTF16Reversed(Char16* buffer, Char32 codePoint)
{
	int count = 0;
	if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
		buffer[count++] = reverseByteOrder((Char16)codePoint);
	else
	{
		int sub = codePoint - 0x10000;
		int high = (sub >> 10) + 0xD800;
		int low = (sub & 0x3FF) + 0xDC00;
		buffer[count++] = reverseByteOrder((Char16)high);
		buffer[count++] = reverseByteOrder((Char16)low);
	}
	return count;
}

static const Char16 kUTF16Header = 0xFEFF;
static const Char16 kUTF16ReversedHeader = 0xFFFE;

class CharEncoding
{
public:
	static CharEncoding* UTF8,* UTF16,* UTF16Reversed,* UTF32;

	virtual void encode(const UnownedStringSlice& str, List<char>& ioBuffer) = 0;
	virtual void decode(const char* buffer, int length, List<char>& ioBuffer) = 0;

	virtual ~CharEncoding()	{}
};

}

#endif
