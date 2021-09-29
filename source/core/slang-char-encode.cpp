#include "slang-char-encode.h"

namespace Slang
{


class Utf8CharEncoding : public CharEncoding 
{
public:
	virtual void GetBytes(List<char> & result, const String & str) override
	{
		result.addRange(str.getBuffer(), str.getLength());
	}
	virtual String ToString(const char * bytes, int /*length*/) override
	{
		return String(bytes);
	}
};

class Utf32CharEncoding : public CharEncoding
{
public:
	virtual void GetBytes(List<char> & result, const String & str) override
	{
		Index ptr = 0;
		while (ptr < str.getLength())
		{
			int codePoint = GetUnicodePointFromUTF8([&](int)
			{
				if (ptr < str.getLength())
					return str[ptr++];
				else
					return '\0';
			});
			result.addRange((char*)&codePoint, 4);
		}
	}
	virtual String ToString(const char * bytes, int length) override
	{
		StringBuilder sb;
		int * content = (int*)bytes;
		for (int i = 0; i < (length >> 2); i++)
		{
			char buf[5];
			int count = EncodeUnicodePointToUTF8(buf, content[i]);
			for (int j = 0; j < count; j++)
				sb.Append(buf[j]);
		}
		return sb.ProduceString();
	}
};

class Utf16CharEncoding : public CharEncoding //UTF16
{
private:
	bool reverseOrder = false;
public:
	Utf16CharEncoding(bool pReverseOrder)
		: reverseOrder(pReverseOrder)
	{}
	virtual void GetBytes(List<char> & result, const String & str) override
	{
		Index ptr = 0;
		while (ptr < str.getLength())
		{
			int codePoint = GetUnicodePointFromUTF8([&](int)
			{
				if (ptr < str.getLength())
					return str[ptr++];
				else
					return '\0';
			});
			unsigned short buffer[2];
			int count;
			if (!reverseOrder)
				count = EncodeUnicodePointToUTF16(buffer, codePoint);
			else
				count = EncodeUnicodePointToUTF16Reversed(buffer, codePoint);
			result.addRange((char*)buffer, count * 2);
		}
	}
	virtual String ToString(const char * bytes, int length) override
	{
		int ptr = 0;
		StringBuilder sb;
		while (ptr < length)
		{
			int codePoint = GetUnicodePointFromUTF16([&](int)
			{
				if (ptr < length)
					return bytes[ptr++];
				else
					return '\0';
			});
			char buf[5];
			int count = EncodeUnicodePointToUTF8(buf, codePoint);
			for (int i = 0; i < count; i++)
				sb.Append(buf[i]);
		}
		return sb.ProduceString();
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
