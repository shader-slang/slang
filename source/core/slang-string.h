#ifndef FUNDAMENTAL_LIB_STRING_H
#define FUNDAMENTAL_LIB_STRING_H
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include "smart-pointer.h"
#include "common.h"
#include "hash.h"
#include "secure-crt.h"

namespace Slang
{
	class _EndLine
	{};
	extern _EndLine EndLine;

	// in-place reversion, works only for ascii string
	inline void ReverseInternalAscii(char * buffer, int length)
	{
		int i, j;
		char c;
		for (i = 0, j = length - 1; i<j; i++, j--)
		{
			c = buffer[i];
			buffer[i] = buffer[j];
			buffer[j] = c;
		}
	}
	template<typename IntType>
	inline int IntToAscii(char * buffer, IntType val, int radix)
	{
		int i = 0;
		IntType sign;
		sign = val;
		if (sign < 0)
			val = (IntType)(0 - val);
		do
		{
			int digit = (val % radix);
			if (digit <= 9)
				buffer[i++] = (char)(digit + '0');
			else
				buffer[i++] = (char)(digit - 10 + 'A');
		} while ((val /= radix) > 0);
		if (sign < 0)
			buffer[i++] = '-';
		buffer[i] = '\0';
		return i;
	}

	inline bool IsUtf8LeadingByte(char ch)
	{
		return (((unsigned char)ch) & 0xC0) == 0xC0;
	}

	inline bool IsUtf8ContinuationByte(char ch)
	{
		return (((unsigned char)ch) & 0xC0) == 0x80;
	}

	/*!
	@brief Represents a UTF-8 encoded string.
	*/

	class String
	{
		friend class StringBuilder;
	private:
		RefPtr<char, RefPtrArrayDestructor> buffer;
		wchar_t * wcharBuffer = nullptr;
		int length = 0;
		void Free()
		{
			if (buffer)
				buffer = 0;
			if (wcharBuffer)
				delete[] wcharBuffer;
			buffer = 0;
			wcharBuffer = 0;
			length = 0;
		}
	public:
		static String FromBuffer(RefPtr<char, RefPtrArrayDestructor> buffer, int len)
		{
			String rs;
			rs.buffer = buffer;
			rs.length = len;
			return rs;
		}
		static String FromWString(const wchar_t * wstr);
		static String FromWString(const wchar_t * wstr, const wchar_t * wend);
		static String FromWChar(const wchar_t ch);
		static String FromUnicodePoint(unsigned int codePoint);
		String()
		{
		}
		const char * begin() const
		{
			return buffer.Ptr();
		}
		const char * end() const
		{
			return buffer.Ptr() + length;
		}
		String(int val, int radix = 10)
		{
			buffer = new char[33];
			length = IntToAscii(buffer.Ptr(), val, radix);
			ReverseInternalAscii(buffer.Ptr(), length);
		}
		String(unsigned int val, int radix = 10)
		{
			buffer = new char[33];
			length = IntToAscii(buffer.Ptr(), val, radix);
			ReverseInternalAscii(buffer.Ptr(), length);
		}
		String(long long val, int radix = 10)
		{
			buffer = new char[65];
			length = IntToAscii(buffer.Ptr(), val, radix);
			ReverseInternalAscii(buffer.Ptr(), length);
		}
		String(float val, const char * format = "%g")
		{
			buffer = new char[128];
			sprintf_s(buffer.Ptr(), 128, format, val);
			length = (int)strnlen_s(buffer.Ptr(), 128);
		}
		String(double val, const char * format = "%g")
		{
			buffer = new char[128];
			sprintf_s(buffer.Ptr(), 128, format, val);
			length = (int)strnlen_s(buffer.Ptr(), 128);
		}
		String(const char * str)
		{
			if (str)
			{
				length = (int)strlen(str);
				buffer = new char[length + 1];
				memcpy(buffer.Ptr(), str, length + 1);
			}
		}
		String(char chr)
		{
			if (chr)
			{
				length = 1;
				buffer = new char[2];
				buffer[0] = chr;
				buffer[1] = '\0';
			}
		}
		String(const String & str)
		{
			this->operator=(str);
		}
		String(String&& other)
		{
			this->operator=(static_cast<String&&>(other));
		}
		~String()
		{
			Free();
		}
		String & operator=(const String & str)
		{
			if (str.buffer == buffer)
				return *this;
			Free();
			if (str.buffer)
			{
				length = str.length;
				buffer = str.buffer;
				wcharBuffer = 0;
			}
			return *this;
		}
		String & operator=(String&& other)
		{
			if (this != &other)
			{
				Free();
				buffer = _Move(other.buffer);
				length = other.length;
				wcharBuffer = other.wcharBuffer;
				other.buffer = 0;
				other.length = 0;
				other.wcharBuffer = 0;
			}
			return *this;
		}
		char operator[](int id) const
		{
#if _DEBUG
			if (id < 0 || id >= length)
				throw "Operator[]: index out of range.";
#endif
			return buffer.Ptr()[id];
		}

		friend String StringConcat(const char * lhs, int leftLen, const char * rhs, int rightLen);
		friend String operator+(const char*op1, const String & op2);
		friend String operator+(const String & op1, const char * op2);
		friend String operator+(const String & op1, const String & op2);

		String TrimStart() const
		{
			if (!buffer)
				return *this;
			int startIndex = 0;
			while (startIndex < length &&
				(buffer[startIndex] == ' ' || buffer[startIndex] == '\t' || buffer[startIndex] == '\r' || buffer[startIndex] == '\n'))
				startIndex++;
			return String(buffer + startIndex);
		}

		String TrimEnd() const
		{
			if (!buffer)
				return *this;

			int endIndex = length - 1;
			while (endIndex >= 0 &&
				(buffer[endIndex] == ' ' || buffer[endIndex] == '\t' || buffer[endIndex] == '\r' || buffer[endIndex] == '\n'))
				endIndex--;
			String res;
			res.length = endIndex + 1;
			res.buffer = new char[endIndex + 2];
			strncpy_s(res.buffer.Ptr(), endIndex + 2, buffer.Ptr(), endIndex + 1);
			return res;
		}

		String Trim() const
		{
			if (!buffer)
				return *this;

			int startIndex = 0;
			while (startIndex < length &&
				(buffer[startIndex] == ' ' || buffer[startIndex] == '\t'))
				startIndex++;
			int endIndex = length - 1;
			while (endIndex >= startIndex &&
				(buffer[endIndex] == ' ' || buffer[endIndex] == '\t'))
				endIndex--;

			String res;
			res.length = endIndex - startIndex + 1;
			res.buffer = new char[res.length + 1];
			memcpy(res.buffer.Ptr(), buffer + startIndex, res.length);
			res.buffer[res.length] = '\0';
			return res;
		}

		String SubString(int id, int len) const
		{
			if (len == 0)
				return "";
			if (id + len > length)
				len = length - id;
#if _DEBUG
			if (id < 0 || id >= length || (id + len) > length)
				throw "SubString: index out of range.";
			if (len < 0)
				throw "SubString: length less than zero.";
#endif
			String res;
			res.buffer = new char[len + 1];
			res.length = len;
			strncpy_s(res.buffer.Ptr(), len + 1, buffer + id, len);
			res.buffer[len] = 0;
			return res;
		}

		const char * Buffer() const
		{
			if (buffer)
				return buffer.Ptr();
			else
				return "";
		}

		const wchar_t * ToWString(int * len = 0) const;

		bool Equals(const String & str, bool caseSensitive = true)
		{
			if (!buffer)
				return (str.buffer == 0);
			if (caseSensitive)
				return (strcmp(buffer.Ptr(), str.buffer.Ptr()) == 0);
			else
			{
#ifdef _MSC_VER
				return (_stricmp(buffer.Ptr(), str.buffer.Ptr()) == 0);
#else
				return (strcasecmp(buffer.Ptr(), str.buffer.Ptr()) == 0);
#endif
			}
		}
		bool operator==(const char * strbuffer) const
		{
			if (!buffer)
				return (strbuffer == 0 || strcmp(strbuffer, "") == 0);
			if (!strbuffer)
				return buffer == nullptr || strcmp(buffer.Ptr(), "") == 0;
			return (strcmp(buffer.Ptr(), strbuffer) == 0);
		}

		bool operator==(const String & str) const
		{
			if (!buffer)
				return (str.buffer == 0 || strcmp(str.buffer.Ptr(), "") == 0);
			if (!str.buffer)
				return buffer == nullptr || strcmp(buffer.Ptr(), "") == 0;
			return (strcmp(buffer.Ptr(), str.buffer.Ptr()) == 0);
		}
		bool operator!=(const char * strbuffer) const
		{
			if (!buffer)
				return (strbuffer != 0 && strcmp(strbuffer, "") != 0);
			if (strbuffer == 0)
				return length != 0;
			return (strcmp(buffer.Ptr(), strbuffer) != 0);
		}
		bool operator!=(const String & str) const
		{
			if (!buffer)
				return (str.buffer != 0 && strcmp(str.buffer.Ptr(), "") != 0);
			if (str.buffer.Ptr() == 0)
				return length != 0;
			return (strcmp(buffer.Ptr(), str.buffer.Ptr()) != 0);
		}
		bool operator>(const String & str) const
		{
			if (!buffer)
				return false;
			if (!str.buffer)
				return buffer.Ptr() != nullptr && length != 0;
			return (strcmp(buffer.Ptr(), str.buffer.Ptr()) > 0);
		}
		bool operator<(const String & str) const
		{
			if (!buffer)
				return (str.buffer != 0);
			if (!str.buffer)
				return false;
			return (strcmp(buffer.Ptr(), str.buffer.Ptr()) < 0);
		}
		bool operator>=(const String & str) const
		{
			if (!buffer)
				return (str.buffer == 0);
			if (!str.buffer)
				return length == 0;
			int res = strcmp(buffer.Ptr(), str.buffer.Ptr());
			return (res > 0 || res == 0);
		}
		bool operator<=(const String & str) const
		{
			if (!buffer)
				return true;
			if (!str.buffer)
				return length > 0;
			int res = strcmp(buffer.Ptr(), str.buffer.Ptr());
			return (res < 0 || res == 0);
		}

		String ToUpper() const
		{
			if (!buffer)
				return *this;
			String res;
			res.length = length;
			res.buffer = new char[length + 1];
			for (int i = 0; i <= length; i++)
				res.buffer[i] = (buffer[i] >= 'a' && buffer[i] <= 'z') ?
				(buffer[i] - 'a' + 'A') : buffer[i];
			return res;
		}

		String ToLower() const
		{
			if (!buffer)
				return *this;
			String res;
			res.length = length;
			res.buffer = new char[length + 1];
			for (int i = 0; i <= length; i++)
				res.buffer[i] = (buffer[i] >= 'A' && buffer[i] <= 'Z') ?
				(buffer[i] - 'A' + 'a') : buffer[i];
			return res;
		}

		int Length() const
		{
			return length;
		}

		int IndexOf(const char * str, int id) const // String str
		{
			if (!buffer)
				return -1;
			if (id < 0 || id >= length)
				return -1;
			auto findRs = strstr(buffer + id, str);
			int res = findRs ? (int)(findRs - buffer.Ptr()) : -1;
			if (res >= 0)
				return res;
			else
				return -1;
		}

		int IndexOf(const String & str, int id) const
		{
			return IndexOf(str.buffer.Ptr(), id);
		}

		int IndexOf(const char * str) const
		{
			return IndexOf(str, 0);
		}

		int IndexOf(const String & str) const
		{
			return IndexOf(str.buffer.Ptr(), 0);
		}

		int IndexOf(char ch, int id) const
		{
#if _DEBUG
			if (id < 0 || id >= length)
				throw "SubString: index out of range.";
#endif
			if (!buffer)
				return -1;
			for (int i = id; i < length; i++)
				if (buffer[i] == ch)
					return i;
			return -1;
		}

		int IndexOf(char ch) const
		{
			return IndexOf(ch, 0);
		}

		int LastIndexOf(char ch) const
		{
			for (int i = length - 1; i >= 0; i--)
				if (buffer[i] == ch)
					return i;
			return -1;
		}

		bool StartsWith(const char * str) const // String str
		{
			if (!buffer)
				return false;
			int strLen = (int)strlen(str);
			if (strLen > length)
				return false;
			for (int i = 0; i < strLen; i++)
				if (str[i] != buffer[i])
					return false;
			return true;
		}

		bool StartsWith(const String & str) const
		{
			return StartsWith(str.buffer.Ptr());
		}

		bool EndsWith(char * str)  const // String str
		{
			if (!buffer)
				return false;
			int strLen = (int)strlen(str);
			if (strLen > length)
				return false;
			for (int i = strLen - 1; i >= 0; i--)
				if (str[i] != buffer[length - strLen + i])
					return false;
			return true;
		}

		bool EndsWith(const String & str) const
		{
			return EndsWith(str.buffer.Ptr());
		}

		bool Contains(const char * str) const // String str
		{
			if (!buffer)
				return false;
			return (IndexOf(str) >= 0) ? true : false;
		}

		bool Contains(const String & str) const
		{
			return Contains(str.buffer.Ptr());
		}

		int GetHashCode() const
		{
			return Slang::GetHashCode((const char*)buffer.Ptr());
		}
		String PadLeft(char ch, int length);
		String PadRight(char ch, int length);
		String ReplaceAll(String src, String dst) const;
	};

	class StringBuilder
	{
	private:
		char * buffer;
		int length;
		int bufferSize;
		static const int InitialSize = 512;
	public:
		StringBuilder(int bufferSize = 1024)
			:buffer(0), length(0), bufferSize(0)
		{
			buffer = new char[InitialSize]; // new a larger buffer 
			buffer[0] = '\0';
			length = 0;
			bufferSize = InitialSize;
		}
		~StringBuilder()
		{
			if (buffer)
				delete[] buffer;
		}
		void EnsureCapacity(int size)
		{
			if (bufferSize < size)
			{
				char * newBuffer = new char[size + 1];
				if (buffer)
				{
					strcpy_s(newBuffer, size + 1, buffer);
					delete[] buffer;
				}
				buffer = newBuffer;
				bufferSize = size;
			}
		}
		StringBuilder & operator << (char ch)
		{
			Append(&ch, 1);
			return *this;
		}
		StringBuilder & operator << (int val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (unsigned int val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (long long val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (float val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (double val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (const char * str)
		{
			Append(str, (int)strlen(str));
			return *this;
		}
		StringBuilder & operator << (const String & str)
		{
			Append(str);
			return *this;
		}
		StringBuilder & operator << (const _EndLine)
		{
			Append('\n');
			return *this;
		}
		void Append(char ch)
		{
			Append(&ch, 1);
		}
		void Append(float val)
		{
			char buf[128];
			sprintf_s(buf, 128, "%g", val);
			int len = (int)strnlen_s(buf, 128);
			Append(buf, len);
		}
		void Append(double val)
		{
			char buf[128];
			sprintf_s(buf, 128, "%g", val);
			int len = (int)strnlen_s(buf, 128);
			Append(buf, len);
		}
		void Append(unsigned int value, int radix = 10)
		{
			char vBuffer[33];
			int len = IntToAscii(vBuffer, value, radix);
			ReverseInternalAscii(vBuffer, len);
			Append(vBuffer);
		}
		void Append(int value, int radix = 10)
		{
			char vBuffer[33];
			int len = IntToAscii(vBuffer, value, radix);
			ReverseInternalAscii(vBuffer, len);
			Append(vBuffer);
		}
		void Append(long long value, int radix = 10)
		{
			char vBuffer[65];
			int len = IntToAscii(vBuffer, value, radix);
			ReverseInternalAscii(vBuffer, len);
			Append(vBuffer);
		}
		void Append(const String & str)
		{
			Append(str.Buffer(), str.Length());
		}
		void Append(const char * str)
		{
			Append(str, (int)strlen(str));
		}
		void Append(const char * str, int strLen)
		{
			int newLength = length + strLen;
			if (bufferSize < newLength + 1)
			{
				int newBufferSize = InitialSize;
				while (newBufferSize < newLength + 1)
					newBufferSize <<= 1;
				char * newBuffer = new char[newBufferSize];
				if (buffer)
				{
					memcpy(newBuffer, buffer, length);
					delete[] buffer;
				}
				memcpy(newBuffer + length, str, strLen);
				newBuffer[newLength] = '\0';
				buffer = newBuffer;
				bufferSize = newBufferSize;
			}
			else
			{
				memcpy(buffer + length, str, strLen);
				buffer[newLength] = '\0';
			}
			length = newLength;
		}

		int Capacity()
		{
			return bufferSize;
		}

		char * Buffer()
		{
			return buffer;
		}

		int Length()
		{
			return length;
		}

		String ToString()
		{
			return String(buffer);
		}

		String ProduceString()
		{
			String rs;
			rs.buffer = buffer;
			rs.length = length;
			buffer = 0;
			bufferSize = 0;
			length = 0;
			return rs;

		}

		String GetSubString(int start, int count)
		{
			String rs;
			rs.buffer = new char[count + 1];
			rs.length = count;
			strncpy_s(rs.buffer.Ptr(), count + 1, buffer + start, count);
			rs.buffer[count] = 0;
			return rs;
		}

		void Remove(int id, int len)
		{
#if _DEBUG
			if (id >= length || id < 0)
				throw "Remove: Index out of range.";
			if (len < 0)
				throw "Remove: remove length smaller than zero.";
#endif
			int actualDelLength = ((id + len) >= length) ? (length - id) : len;
			for (int i = id + actualDelLength; i <= length; i++)
				buffer[i - actualDelLength] = buffer[i];
			length -= actualDelLength;
		}

		void Clear()
		{
			length = 0;
			if (buffer)
				buffer[0] = 0;
		}
	};

	int StringToInt(const String & str, int radix = 10);
	unsigned int StringToUInt(const String & str, int radix = 10);
	double StringToDouble(const String & str);
	float StringToFloat(const String & str);
}

#endif
