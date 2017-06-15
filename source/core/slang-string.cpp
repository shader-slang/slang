#include "slang-string.h"
#include "text-io.h"

namespace Slang
{
	_EndLine EndLine;
	String StringConcat(const char * lhs, int leftLen, const char * rhs, int rightLen)
	{
		String res;
		res.length = leftLen + rightLen;
		res.buffer = new char[res.length + 1];
		strcpy_s(res.buffer.Ptr(), res.length + 1, lhs);
		strcpy_s(res.buffer + leftLen, res.length + 1 - leftLen, rhs);
		return res;
	}
	String operator+(const char * op1, const String & op2)
	{
		if(!op2.buffer)
			return String(op1);

		return StringConcat(op1, (int)strlen(op1), op2.buffer.Ptr(), op2.length);
	}

	String operator+(const String & op1, const char * op2)
	{
		if(!op1.buffer)
			return String(op2);

		return StringConcat(op1.buffer.Ptr(), op1.length, op2, (int)strlen(op2));
	}

	String operator+(const String & op1, const String & op2)
	{
		if(!op1.buffer && !op2.buffer)
			return String();
		else if(!op1.buffer)
			return String(op2);
		else if(!op2.buffer)
			return String(op1);

		return StringConcat(op1.buffer.Ptr(), op1.length, op2.buffer.Ptr(), op2.length);
	}

	int StringToInt(const String & str, int radix)
	{
		if (str.StartsWith("0x"))
			return (int)strtoll(str.Buffer(), NULL, 16);
		else
			return (int)strtoll(str.Buffer(), NULL, radix);
	}
	unsigned int StringToUInt(const String & str, int radix)
	{
		if (str.StartsWith("0x"))
			return (unsigned int)strtoull(str.Buffer(), NULL, 16);
		else
			return (unsigned int)strtoull(str.Buffer(), NULL, radix);
	}
	double StringToDouble(const String & str)
	{
		return (double)strtod(str.Buffer(), NULL);
	}
	float StringToFloat(const String & str)
	{
		return strtof(str.Buffer(), NULL);
	}

	String String::ReplaceAll(String src, String dst) const
	{
		String rs = *this;
		int index = 0;
		int srcLen = src.length;
		int len = rs.length;
		while ((index = rs.IndexOf(src, index)) != -1)
		{
			rs = rs.SubString(0, index) + dst + rs.SubString(index + srcLen, len - index - srcLen);
			len = rs.length;
		}
		return rs;
	}

	String String::FromWString(const wchar_t * wstr)
	{
#ifdef _WIN32
		return Slang::Encoding::UTF16->ToString((const char*)wstr, (int)(wcslen(wstr) * sizeof(wchar_t)));
#else
		return Slang::Encoding::UTF32->ToString((const char*)wstr, (int)(wcslen(wstr) * sizeof(wchar_t)));
#endif
	}

	String String::FromWString(const wchar_t * wstr, const wchar_t * wend)
	{
#ifdef _WIN32
		return Slang::Encoding::UTF16->ToString((const char*)wstr, (int)((wend - wstr) * sizeof(wchar_t)));
#else
		return Slang::Encoding::UTF32->ToString((const char*)wstr, (int)((wend - wstr) * sizeof(wchar_t)));
#endif
	}

	String String::FromWChar(const wchar_t ch)
	{
#ifdef _WIN32
		return Slang::Encoding::UTF16->ToString((const char*)&ch, (int)(sizeof(wchar_t)));
#else
		return Slang::Encoding::UTF32->ToString((const char*)&ch, (int)(sizeof(wchar_t)));
#endif
	}

	String String::FromUnicodePoint(unsigned int codePoint)
	{
		char buf[6];
		int len = Slang::EncodeUnicodePointToUTF8(buf, (int)codePoint);
		buf[len] = 0;
		return String(buf);
	}

	const wchar_t * String::ToWString(int * len) const
	{
		if (!buffer)
		{
			if (len)
				*len = 0;
			return L"";
		}
		else
		{
			if (wcharBuffer)
			{
				if (len)
					*len = (int)wcslen(wcharBuffer);
				return wcharBuffer;
			}
			List<char> buf;
			Slang::Encoding::UTF16->GetBytes(buf, *this);
			if (len)
				*len = buf.Count() / sizeof(wchar_t);
			buf.Add(0);
			buf.Add(0);
			const_cast<String*>(this)->wcharBuffer = (wchar_t*)buf.Buffer();
			buf.ReleaseBuffer();
			return wcharBuffer;
		}
	}

	String String::PadLeft(char ch, int pLen)
	{
		StringBuilder sb;
		for (int i = 0; i < pLen - this->length; i++)
			sb << ch;
		for (int i = 0; i < this->length; i++)
			sb << buffer[i];
		return sb.ProduceString();
	}

	String String::PadRight(char ch, int pLen)
	{
		StringBuilder sb;
		for (int i = 0; i < this->length; i++)
			sb << buffer[i];
		for (int i = 0; i < pLen - this->length; i++)
			sb << ch;
		return sb.ProduceString();
	}
}
