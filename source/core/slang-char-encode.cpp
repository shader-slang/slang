#include "slang-char-encode.h"

namespace Slang
{


class Utf8CharEncoding : public CharEncoding 
{
public:
	virtual void encode(const UnownedStringSlice& slice, List<char>& ioBuffer) override
	{
        ioBuffer.addRange(slice.begin(), slice.getLength());
	}
	virtual void decode(const char* bytes, int length, List<char>& ioChars) override
	{
        ioChars.addRange(bytes, length);
	}
};

class Utf32CharEncoding : public CharEncoding
{
public:
	virtual void encode(const UnownedStringSlice& slice, List<char>& ioBuffer) override
	{
		Index ptr = 0;
		while (ptr < slice.getLength())
		{
            const Char32 codePoint = getUnicodePointFromUTF8([&](int)
			{
				if (ptr < slice.getLength())
					return slice[ptr++];
				else
					return '\0';
			});
            ioBuffer.addRange((const char*)&codePoint, 4);
		}
	}
	virtual void decode(const char* bytes, int length, List<char>& ioBuffer) override
	{
		const Char32* content = (const Char32*)bytes;
		for (int i = 0; i < (length >> 2); i++)
		{
			char buf[5];
			int count = encodeUnicodePointToUTF8(content[i], buf);
            for (int j = 0; j < count; j++)
                ioBuffer.addRange(buf, count);
		}
	}
};

class Utf16CharEncoding : public CharEncoding //UTF16
{
private:
	bool m_reverseOrder = false;
public:
	Utf16CharEncoding(bool reverseOrder)
		: m_reverseOrder(reverseOrder)
	{}
	virtual void encode(const UnownedStringSlice& slice, List<char>& ioBuffer) override
	{
		Index ptr = 0;
		while (ptr < slice.getLength())
		{
            Char32 codePoint = getUnicodePointFromUTF8([&](int)
			{
				if (ptr < slice.getLength())
					return slice[ptr++];
				else
					return '\0';
			});

			Char16 buffer[2];
			int count;
			if (!m_reverseOrder)
				count = encodeUnicodePointToUTF16(codePoint, buffer);
			else
				count = encodeUnicodePointToUTF16Reversed(codePoint, buffer);
            ioBuffer.addRange((char*)buffer, count * 2);
		}
	}
	virtual void decode(const char* bytes, int length, List<char>& ioBuffer) override
	{
		int ptr = 0;
		while (ptr < length)
		{
			Char32 codePoint = getUnicodePointFromUTF16([&](int)
			{
				if (ptr < length)
					return bytes[ptr++];
				else
					return '\0';
			});

			char buf[5];
			int count = encodeUnicodePointToUTF8(codePoint, buf);
            ioBuffer.addRange((const char*)buf, count);
		}
	}
};

static Utf8CharEncoding _utf8Encoding;
static Utf16CharEncoding _utf16Encoding(false);
static 	Utf16CharEncoding _utf16EncodingReversed(true);
static Utf32CharEncoding _utf32Encoding;

CharEncoding* CharEncoding::UTF8 = &_utf8Encoding;
CharEncoding* CharEncoding::UTF16 = &_utf16Encoding;
CharEncoding* CharEncoding::UTF16Reversed = &_utf16EncodingReversed;
CharEncoding* CharEncoding::UTF32 = &_utf32Encoding;

	
} // namespace Slang
