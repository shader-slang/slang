#ifndef FUNDAMENTAL_LIB_STRING_H
#define FUNDAMENTAL_LIB_STRING_H

#include <string.h>
#include <cstdlib>
#include <stdio.h>

#include "smart-pointer.h"
#include "common.h"
#include "hash.h"
#include "secure-crt.h"

#include <new>

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

    struct UnownedStringSlice
    {
    public:
        UnownedStringSlice()
            : beginData(nullptr)
            , endData(nullptr)
        {}

        explicit UnownedStringSlice(char const* a) :
            beginData(a),
            endData(a + strlen(a))
        {}
        UnownedStringSlice(char const* b, char const* e)
            : beginData(b)
            , endData(e)
        {}
        UnownedStringSlice(char const* b, size_t len)
            : beginData(b)
            , endData(b + len)
        {}

        char const* begin() const
        {
            return beginData;
        }

        char const* end() const
        {
            return endData;
        }

        UInt size() const
        {
            return endData - beginData;
        }

        int indexOf(char c) const
        {
            const int size = int(endData - beginData);
            for (int i = 0; i < size; ++i)
            {
                if (beginData[i] == c)
                {
                    return i;
                }
            }
            return -1;
        }

        const char& operator[](UInt i) const
        {
            assert(i < UInt(endData - beginData));
            return beginData[i];
        }

        bool operator==(UnownedStringSlice const& other) const
        {
            return size() == other.size()
                && memcmp(begin(), other.begin(), size()) == 0;
        }

        bool operator==(char const* str) const
        {
            return (*this) == UnownedStringSlice(str, str + strlen(str));
        }

        bool operator!=(UnownedStringSlice const& other) const
        {
            return !(*this == other);
        }

        bool startsWith(UnownedStringSlice const& other) const;
        bool startsWith(char const* str) const;

        bool endsWith(UnownedStringSlice const& other) const;
        bool endsWith(char const* str) const;

        int GetHashCode() const
        {
            return Slang::GetHashCode(beginData, size_t(endData - beginData)); 
        }

        template <size_t SIZE> 
        SLANG_FORCE_INLINE static UnownedStringSlice fromLiteral(const char (&in)[SIZE]) { return UnownedStringSlice(in, SIZE - 1); }

    private:
        char const* beginData;
        char const* endData;
    };

    // A `StringRepresentation` provides the backing storage for
    // all reference-counted string-related types.
    class StringRepresentation : public RefObject
    {
    public:
        UInt length;
        UInt capacity;

        SLANG_FORCE_INLINE UInt getLength() const
        {
            return length;
        }

        SLANG_FORCE_INLINE char* getData()
        {
            return (char*) (this + 1);
        }
        SLANG_FORCE_INLINE const char* getData() const
        {
            return (const char*)(this + 1);
        }

        static const char* getData(const StringRepresentation* stringRep)
        {
            return stringRep ? stringRep->getData() : "";
        }

        static UnownedStringSlice asSlice(const StringRepresentation* rep)
        {
            return rep ? UnownedStringSlice(rep->getData(), rep->getLength()) : UnownedStringSlice();
        }

        static bool equal(const StringRepresentation* a, const StringRepresentation* b)
        {
            return (a == b) || asSlice(a) == asSlice(b);
        }

        static StringRepresentation* createWithCapacityAndLength(UInt capacity, UInt length)
        {
            SLANG_ASSERT(capacity >= length);
            void* allocation = operator new(sizeof(StringRepresentation) + capacity + 1);
            StringRepresentation* obj = new(allocation) StringRepresentation();
            obj->capacity = capacity;
            obj->length = length;
            obj->getData()[length] = 0;
            return obj;
        }

        static StringRepresentation* createWithCapacity(UInt capacity)
        {
            return createWithCapacityAndLength(capacity, 0);
        }

        static StringRepresentation* createWithLength(UInt length)
        {
            return createWithCapacityAndLength(length, length);
        }

        StringRepresentation* cloneWithCapacity(UInt newCapacity)
        {
            StringRepresentation* newObj = createWithCapacityAndLength(newCapacity, length);
            memcpy(getData(), newObj->getData(), length + 1);
            return newObj;
        }

        StringRepresentation* clone()
        {
            return cloneWithCapacity(length);
        }

        StringRepresentation* ensureCapacity(UInt required)
        {
            if (capacity >= required) return this;

            UInt newCapacity = capacity;
            if (!newCapacity) newCapacity = 16; // TODO: figure out good value for minimum capacity

            while (newCapacity < required)
            {
                newCapacity = 2 * newCapacity;
            }

            return cloneWithCapacity(newCapacity);
        }
    };

    class String;



    struct UnownedTerminatedStringSlice : public UnownedStringSlice
    {
    public:
        UnownedTerminatedStringSlice(char const* b)
            : UnownedStringSlice(b, b + strlen(b))
        {}
    };

    struct StringSlice
    {
    public:
        StringSlice();

        StringSlice(String const& str);

        StringSlice(String const& str, UInt beginIndex, UInt endIndex);

        UInt getLength() const
        {
            return endIndex - beginIndex;
        }

        char const* begin() const
        {
            return representation ? representation->getData() + beginIndex : "";
        }

        char const* end() const
        {
            return begin() + getLength();
        }

    private:
        RefPtr<StringRepresentation> representation;
        UInt beginIndex;
        UInt endIndex;

        friend class String;

        StringSlice(RefPtr<StringRepresentation> const& representation, UInt beginIndex, UInt endIndex)
            : representation(representation)
            , beginIndex(beginIndex)
            , endIndex(endIndex)
        {}
    };

    /// String as expected by underlying platform APIs
    class OSString
    {
    public:
        OSString();
        OSString(wchar_t* begin, wchar_t* end);
        ~OSString();

        operator wchar_t const*() const
        {
            return begin();
        }

        wchar_t const* begin() const;
        wchar_t const* end() const;

    private:
        wchar_t* beginData;
        wchar_t* endData;
    };

	/*!
	@brief Represents a UTF-8 encoded string.
	*/

	class String
	{
        friend struct StringSlice;
		friend class StringBuilder;
	private:


        char* getData() const
        {
            return buffer ? buffer->getData() : (char*)"";
        }

        UInt getLength() const
        {
            return buffer ? buffer->getLength() : 0;
        }

        void ensureUniqueStorageWithCapacity(UInt capacity);
     
        RefPtr<StringRepresentation> buffer;

    public:

        explicit String(StringRepresentation* buffer)
            : buffer(buffer)
        {}

		static String FromWString(const wchar_t * wstr);
		static String FromWString(const wchar_t * wstr, const wchar_t * wend);
		static String FromWChar(const wchar_t ch);
		static String FromUnicodePoint(unsigned int codePoint);
		String()
		{
		}

            /// Returns a buffer which can hold at least count chars
        char* prepareForAppend(UInt count);
            /// Append data written to buffer output via 'prepareForAppend' directly written 'inplace'
        void appendInPlace(const char* chars, UInt count);

        SLANG_FORCE_INLINE StringRepresentation* getStringRepresentation() const { return buffer; }

		const char * begin() const
		{
			return getData();
		}
		const char * end() const
		{
			return getData() + getLength();
		}

        void append(int32_t value, int radix = 10);
        void append(uint32_t value, int radix = 10);
        void append(int64_t value, int radix = 10);
        void append(uint64_t value, int radix = 10);
        void append(float val, const char * format = "%g");
        void append(double val, const char * format = "%g");

        void append(char const* str);
        void append(const char* textBegin, char const* textEnd);
        void append(char chr);
        void append(String const& str);
        void append(StringSlice const& slice);
        void append(UnownedStringSlice const& slice);

		String(int32_t val, int radix = 10)
		{
            append(val, radix);
		}
		String(uint32_t val, int radix = 10)
		{
            append(val, radix);
		}
		String(int64_t val, int radix = 10)
		{
            append(val, radix);
		}
		String(uint64_t val, int radix = 10)
		{
            append(val, radix);
		}
		String(float val, const char * format = "%g")
		{
            append(val, format);
		}
		String(double val, const char * format = "%g")
		{
            append(val, format);
		}
		String(const char * str)
		{
            append(str);
#if 0
			if (str)
			{
                buffer = StringRepresentation::createWithLength(strlen(str));
				memcpy(buffer.Ptr(), str, getLength() + 1);
			}
#endif
		}
        String(const char* textBegin, char const* textEnd)
		{
            append(textBegin, textEnd);
#if 0
			if (textBegin != textEnd)
			{
                buffer = StringRepresentation::createWithLength(textEnd - textBegin);
				memcpy(buffer.Ptr(), textBegin, getLength());
                buffer->getData()[getLength()] = 0;
			}
#endif
		}
		String(char chr)
		{
            append(chr);
#if 0
			if (chr)
			{
                buffer = StringRepresentation::createWithLength(1);
                buffer->getData()[0] = chr;
                buffer->getData()[1] = 0;
			}
#endif
		}
		String(String const& str)
		{
            buffer = str.buffer;
#if 0
			this->operator=(str);
#endif
		}
		String(String&& other)
		{
            buffer = _Move(other.buffer);
		}

        String(StringSlice const& slice)
        {
            append(slice);
        }

        String(UnownedStringSlice const& slice)
        {
            append(slice);
        }

		~String()
		{
            buffer = 0;
		}

		String & operator=(const String & str)
		{
            buffer = str.buffer;
			return *this;
		}
		String & operator=(String&& other)
		{
            buffer = _Move(other.buffer);
            return *this;
		}
		char operator[](UInt id) const
		{
#if _DEBUG
			if (id < 0 || id >= getLength())
				throw "Operator[]: index out of range.";
#endif
			return begin()[id];
		}

		friend String operator+(const char*op1, const String & op2);
		friend String operator+(const String & op1, const char * op2);
		friend String operator+(const String & op1, const String & op2);

		StringSlice TrimStart() const
		{
			if (!buffer)
				return StringSlice();
			UInt startIndex = 0;
			while (startIndex < getLength() &&
				(getData()[startIndex] == ' ' || getData()[startIndex] == '\t' || getData()[startIndex] == '\r' || getData()[startIndex] == '\n'))
				startIndex++;
            return StringSlice(buffer, startIndex, getLength());
		}

		StringSlice TrimEnd() const
		{
			if (!buffer)
				return StringSlice();

			UInt endIndex = getLength();
			while (endIndex > 0 &&
				(getData()[endIndex-1] == ' ' || getData()[endIndex-1] == '\t' || getData()[endIndex-1] == '\r' || getData()[endIndex-1] == '\n'))
				endIndex--;

            return StringSlice(buffer, 0, endIndex);
		}

		StringSlice Trim() const
		{
			if (!buffer)
				return StringSlice();

			UInt startIndex = 0;
			while (startIndex < getLength() &&
				(getData()[startIndex] == ' ' || getData()[startIndex] == '\t'))
				startIndex++;
			UInt endIndex = getLength();
			while (endIndex > startIndex &&
				(getData()[endIndex-1] == ' ' || getData()[endIndex-1] == '\t'))
				endIndex--;

            return StringSlice(buffer, startIndex, endIndex);
		}

		StringSlice SubString(UInt id, UInt len) const
		{
			if (len == 0)
				return StringSlice();

            if (id + len > getLength())
				len = getLength() - id;
#if _DEBUG
			if (id < 0 || id >= getLength() || (id + len) > getLength())
				throw "SubString: index out of range.";
			if (len < 0)
				throw "SubString: length less than zero.";
#endif
            return StringSlice(buffer, id, id + len);
		}

		char const* Buffer() const
		{
            return getData();
		}

        OSString ToWString(UInt* len = 0) const;

		bool Equals(const String & str, bool caseSensitive = true)
		{
			if (caseSensitive)
				return (strcmp(begin(), str.begin()) == 0);
			else
			{
#ifdef _MSC_VER
				return (_stricmp(begin(), str.begin()) == 0);
#else
				return (strcasecmp(begin(), str.begin()) == 0);
#endif
			}
		}
		bool operator==(const char * strbuffer) const
		{
			return (strcmp(begin(), strbuffer) == 0);
		}

		bool operator==(const String & str) const
		{
			return (strcmp(begin(), str.begin()) == 0);
		}
		bool operator!=(const char * strbuffer) const
		{
			return (strcmp(begin(), strbuffer) != 0);
		}
		bool operator!=(const String & str) const
		{
			return (strcmp(begin(), str.begin()) != 0);
		}
		bool operator>(const String & str) const
		{
			return (strcmp(begin(), str.begin()) > 0);
		}
		bool operator<(const String & str) const
		{
			return (strcmp(begin(), str.begin()) < 0);
		}
		bool operator>=(const String & str) const
		{
			return (strcmp(begin(), str.begin()) >= 0);
		}
		bool operator<=(const String & str) const
		{
			return (strcmp(begin(), str.begin()) <= 0);
		}

		String ToUpper() const
		{
            String result;
            for (auto c : *this)
            {
                char d = (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
                result.append(d);
            }
            return result;
		}

		String ToLower() const
		{
            String result;
            for (auto c : *this)
            {
                char d = (c >= 'A' && c <= 'Z') ? (c - ('A' - 'a')) : c;
                result.append(d);
            }
            return result;
		}

		UInt Length() const
		{
			return getLength();
		}

		UInt IndexOf(const char * str, UInt id) const // String str
		{
			if (id >= getLength())
				return UInt(-1);
			auto findRs = strstr(begin() + id, str);
			UInt res = findRs ? findRs - begin() : -1;
			return res;
		}

		UInt IndexOf(const String & str, UInt id) const
		{
			return IndexOf(str.begin(), id);
		}

		UInt IndexOf(const char * str) const
		{
			return IndexOf(str, 0);
		}

		UInt IndexOf(const String & str) const
		{
			return IndexOf(str.begin(), 0);
		}

		UInt IndexOf(char ch, UInt id) const
		{
#if _DEBUG
			if (id < 0 || id >= getLength())
				throw "SubString: index out of range.";
#endif
			if (!buffer)
				return UInt(-1);
			for (UInt i = id; i < getLength(); i++)
				if (getData()[i] == ch)
					return i;
			return UInt(-1);
		}

		UInt IndexOf(char ch) const
		{
			return IndexOf(ch, 0);
		}

		UInt LastIndexOf(char ch) const
		{
			for (UInt i = getLength(); i > 0; i--)
				if (getData()[i-1] == ch)
					return i-1;
			return UInt(-1);
		}

		bool StartsWith(const char * str) const // String str
		{
			if (!buffer)
				return false;
			UInt strLen = strlen(str);
			if (strLen > getLength())
				return false;
			for (UInt i = 0; i < strLen; i++)
				if (str[i] != getData()[i])
					return false;
			return true;
		}

		bool StartsWith(const String & str) const
		{
			return StartsWith(str.begin());
		}

		bool EndsWith(char const * str)  const // String str
		{
			if (!buffer)
				return false;
			UInt strLen = strlen(str);
			if (strLen > getLength())
				return false;
			for (UInt i = strLen; i > 0; i--)
				if (str[i-1] != getData()[getLength() - strLen + i-1])
					return false;
			return true;
		}

		bool EndsWith(const String & str) const
		{
			return EndsWith(str.begin());
		}

		bool Contains(const char * str) const // String str
		{
			if (!buffer)
				return false;
			return (IndexOf(str) != UInt(-1)) ? true : false;
		}

		bool Contains(const String & str) const
		{
			return Contains(str.begin());
		}

		int GetHashCode() const
		{
			return Slang::GetHashCode((const char*)begin());
		}

        UnownedStringSlice getUnownedSlice() const
        {
            return StringRepresentation::asSlice(buffer);
        }
	};

	class StringBuilder : public String
	{
	private:
        enum { InitialSize = 1024 };
	public:
		explicit StringBuilder(UInt bufferSize = InitialSize)
		{
            ensureUniqueStorageWithCapacity(bufferSize);
		}
		void EnsureCapacity(UInt size)
		{
            ensureUniqueStorageWithCapacity(size);
		}
		StringBuilder & operator << (char ch)
		{
			Append(&ch, 1);
			return *this;
		}
		StringBuilder & operator << (Int32 val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (UInt32 val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (Int64 val)
		{
			Append(val);
			return *this;
		}
		StringBuilder & operator << (UInt64 val)
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
			Append(str, strlen(str));
			return *this;
		}
		StringBuilder & operator << (const String & str)
		{
			Append(str);
			return *this;
		}
		StringBuilder & operator << (UnownedStringSlice const& str)
		{
			append(str);
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
		void Append(Int32 value, int radix = 10)
		{
			char vBuffer[33];
			int len = IntToAscii(vBuffer, value, radix);
			ReverseInternalAscii(vBuffer, len);
			Append(vBuffer);
		}
		void Append(UInt32 value, int radix = 10)
		{
			char vBuffer[33];
			int len = IntToAscii(vBuffer, value, radix);
			ReverseInternalAscii(vBuffer, len);
			Append(vBuffer);
		}
		void Append(Int64 value, int radix = 10)
		{
			char vBuffer[65];
			int len = IntToAscii(vBuffer, value, radix);
			ReverseInternalAscii(vBuffer, len);
			Append(vBuffer);
		}
		void Append(UInt64 value, int radix = 10)
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
			Append(str, strlen(str));
		}
		void Append(const char * str, UInt strLen)
		{
            append(str, str + strLen);
		}

#if 0
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
#endif

		String ToString()
		{
            return *this;
		}

		String ProduceString()
		{
            return *this;
		}

#if 0
		String GetSubString(int start, int count)
		{
			String rs;
			rs.buffer = new char[count + 1];
			rs.length = count;
			strncpy_s(rs.buffer.Ptr(), count + 1, buffer + start, count);
			rs.buffer[count] = 0;
			return rs;
		}
#endif

#if 0
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
#endif

		void Clear()
		{
            buffer = 0;
		}
	};

	int StringToInt(const String & str, int radix = 10);
	unsigned int StringToUInt(const String & str, int radix = 10);
	double StringToDouble(const String & str);
	float StringToFloat(const String & str);
}

#endif
