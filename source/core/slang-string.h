#ifndef SLANG_CORE_STRING_H
#define SLANG_CORE_STRING_H

#include <string.h>
#include <cstdlib>
#include <stdio.h>

#include "slang-smart-pointer.h"
#include "slang-common.h"
#include "slang-hash.h"
#include "slang-secure-crt.h"

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
            : m_begin(nullptr)
            , m_end(nullptr)
        {}

        explicit UnownedStringSlice(char const* a) :
            m_begin(a),
            m_end(a + strlen(a))
        {}
        UnownedStringSlice(char const* b, char const* e)
            : m_begin(b)
            , m_end(e)
        {}
        UnownedStringSlice(char const* b, size_t len)
            : m_begin(b)
            , m_end(b + len)
        {}

        char const* begin() const
        {
            return m_begin;
        }

        char const* end() const
        {
            return m_end;
        }

            /// True if slice is strictly contained in memory.
        bool isMemoryContained(const UnownedStringSlice& slice) const
        {
            return slice.m_begin >= m_begin && slice.m_end <= m_end; 
        }

        Index getLength() const
        {
            return Index(m_end - m_begin);
        }

            /// Finds first index of char 'c'. If not found returns -1.
        Index indexOf(char c) const;
            /// Find first index of slice. If not found returns -1
        Index indexOf(const UnownedStringSlice& slice) const;

        Index lastIndexOf(char c) const
        {
            const Index size = Index(m_end - m_begin);
            for (Index i = size - 1; i >= 0; --i)
            {
                if (m_begin[i] == c)
                {
                    return i;
                }
            }
            return -1;
        }

        const char& operator[](Index i) const
        {
            assert(i >= 0 && i < Index(m_end - m_begin));
            return m_begin[i];
        }

        bool operator==(UnownedStringSlice const& other) const
        {
            // Note that memcmp is undefined when passed in null ptrs, so if we want to handle
            // we need to cover that case.
            // Can only be nullptr if size is 0.
            auto thisSize = getLength();
            auto otherSize = other.getLength();

            if (thisSize != otherSize)
            {
                return false;
            }

            const char*const thisChars = begin();
            const char*const otherChars = other.begin();
            if (thisChars == otherChars || thisSize == 0)
            {
                return true;
            }
            SLANG_ASSERT(thisChars && otherChars);
            return memcmp(thisChars, otherChars, thisSize) == 0;
        }

        bool operator==(char const* str) const
        {
            return (*this) == UnownedStringSlice(str, str + ::strlen(str));
        }

        bool operator!=(UnownedStringSlice const& other) const
        {
            return !(*this == other);
        }

        bool startsWith(UnownedStringSlice const& other) const;
        bool startsWith(char const* str) const;

        bool endsWith(UnownedStringSlice const& other) const;
        bool endsWith(char const* str) const;


        UnownedStringSlice trim() const;

        HashCode getHashCode() const
        {
            return Slang::getHashCode(m_begin, size_t(m_end - m_begin)); 
        }

        template <size_t SIZE> 
        SLANG_FORCE_INLINE static UnownedStringSlice fromLiteral(const char (&in)[SIZE]) { return UnownedStringSlice(in, SIZE - 1); }

    private:
        char const* m_begin;
        char const* m_end;
    };

    // A `StringRepresentation` provides the backing storage for
    // all reference-counted string-related types.
    class StringRepresentation : public RefObject
    {
    public:
        Index length;
        Index capacity;

        SLANG_FORCE_INLINE Index getLength() const
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

        static StringRepresentation* createWithCapacityAndLength(Index capacity, Index length)
        {
            SLANG_ASSERT(capacity >= length);
            void* allocation = operator new(sizeof(StringRepresentation) + capacity + 1);
            StringRepresentation* obj = new(allocation) StringRepresentation();
            obj->capacity = capacity;
            obj->length = length;
            obj->getData()[length] = 0;
            return obj;
        }

        static StringRepresentation* createWithCapacity(Index capacity)
        {
            return createWithCapacityAndLength(capacity, 0);
        }

        static StringRepresentation* createWithLength(Index length)
        {
            return createWithCapacityAndLength(length, length);
        }

        StringRepresentation* cloneWithCapacity(Index newCapacity)
        {
            StringRepresentation* newObj = createWithCapacityAndLength(newCapacity, length);
            memcpy(getData(), newObj->getData(), length + 1);
            return newObj;
        }

        StringRepresentation* clone()
        {
            return cloneWithCapacity(length);
        }

        StringRepresentation* ensureCapacity(Index required)
        {
            if (capacity >= required) return this;

            Index newCapacity = capacity;
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
            /// Default
        OSString();
            /// NOTE! This assumes that begin is a new wchar_t[] buffer, and it will
            /// now be owned by the OSString
        OSString(wchar_t* begin, wchar_t* end);
            /// Move Ctor
        OSString(OSString&& rhs):
            m_begin(rhs.m_begin),
            m_end(rhs.m_end)
        {
            rhs.m_begin = nullptr;
            rhs.m_end = nullptr;
        }
            // Copy Ctor
        OSString(const OSString& rhs) :
            m_begin(nullptr),
            m_end(nullptr)
        {
            set(rhs.m_begin, rhs.m_end);
        }

            /// =
        void operator=(const OSString& rhs) { set(rhs.m_begin, rhs.m_end); }
        void operator=(OSString&& rhs)
        {
            auto begin = m_begin;
            auto end = m_end;
            m_begin = rhs.m_begin;
            m_end = rhs.m_end;
            rhs.m_begin = begin;
            rhs.m_end = end;
        }

        ~OSString() { _releaseBuffer(); }

        size_t getLength() const { return (m_end - m_begin); }
        void set(const wchar_t* begin, const wchar_t* end);

        operator wchar_t const*() const
        {
            return begin();
        }

        wchar_t const* begin() const;
        wchar_t const* end() const;

    private:

        void _releaseBuffer();

        wchar_t* m_begin;           ///< First character. This is a new wchar_t[] buffer
        wchar_t* m_end;             ///< Points to terminating 0
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
            return m_buffer ? m_buffer->getData() : (char*)"";
        }

     
        void ensureUniqueStorageWithCapacity(Index capacity);
     
        RefPtr<StringRepresentation> m_buffer;

    public:

        explicit String(StringRepresentation* buffer)
            : m_buffer(buffer)
        {}

		static String fromWString(const wchar_t * wstr);
		static String fromWString(const wchar_t * wstr, const wchar_t * wend);
		static String fromWChar(const wchar_t ch);
		static String fromUnicodePoint(unsigned int codePoint);
		String()
		{
		}

            /// Returns a buffer which can hold at least count chars
        char* prepareForAppend(Index count);
            /// Append data written to buffer output via 'prepareForAppend' directly written 'inplace'
        void appendInPlace(const char* chars, Index count);

        SLANG_FORCE_INLINE StringRepresentation* getStringRepresentation() const { return m_buffer; }

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

            /// Append a character (to remove ambiguity with other integral types)
        void appendChar(char chr);

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
            m_buffer = str.m_buffer;
#if 0
			this->operator=(str);
#endif
		}
		String(String&& other)
		{
            m_buffer = _Move(other.m_buffer);
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
            m_buffer.setNull(); 
		}

		String & operator=(const String & str)
		{
            m_buffer = str.m_buffer;
			return *this;
		}
		String & operator=(String&& other)
		{
            m_buffer = _Move(other.m_buffer);
            return *this;
		}
		char operator[](Index id) const
		{
            SLANG_ASSERT(id >= 0 && id < getLength());
			return begin()[id];
		}

        Index getLength() const
        {
            return m_buffer ? m_buffer->getLength() : 0;
        }

		friend String operator+(const char*op1, const String & op2);
		friend String operator+(const String & op1, const char * op2);
		friend String operator+(const String & op1, const String & op2);

		StringSlice trimStart() const
		{
			if (!m_buffer)
				return StringSlice();
			Index startIndex = 0;
            const char*const data = getData();
			while (startIndex < getLength() &&
				(data[startIndex] == ' ' || data[startIndex] == '\t' || data[startIndex] == '\r' || data[startIndex] == '\n'))
				startIndex++;
            return StringSlice(m_buffer, startIndex, getLength());
		}

		StringSlice trimEnd() const
		{
			if (!m_buffer)
				return StringSlice();

			Index endIndex = getLength();
            const char*const data = getData();
			while (endIndex > 0 &&
				(data[endIndex-1] == ' ' || data[endIndex-1] == '\t' || data[endIndex-1] == '\r' || data[endIndex-1] == '\n'))
				endIndex--;

            return StringSlice(m_buffer, 0, endIndex);
		}

		StringSlice trim() const
		{
			if (!m_buffer)
				return StringSlice();

			Index startIndex = 0;
            const char*const data = getData();
			while (startIndex < getLength() &&
				(data[startIndex] == ' ' || data[startIndex] == '\t'))
				startIndex++;
            Index endIndex = getLength();
			while (endIndex > startIndex &&
				(data[endIndex-1] == ' ' || data[endIndex-1] == '\t'))
				endIndex--;

            return StringSlice(m_buffer, startIndex, endIndex);
		}

		StringSlice subString(Index id, Index len) const
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
            return StringSlice(m_buffer, id, id + len);
		}

		char const* getBuffer() const
		{
            return getData();
		}

        OSString toWString(Index* len = 0) const;

		bool equals(const String & str, bool caseSensitive = true)
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

		String toUpper() const
		{
            String result;
            for (auto c : *this)
            {
                char d = (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
                result.append(d);
            }
            return result;
		}

		String toLower() const
		{
            String result;
            for (auto c : *this)
            {
                char d = (c >= 'A' && c <= 'Z') ? (c - ('A' - 'a')) : c;
                result.append(d);
            }
            return result;
		}

        Index indexOf(const char * str, Index id) const // String str
		{
			if (id >= getLength())
				return Index(-1);
			auto findRs = strstr(begin() + id, str);
			Index res = findRs ? findRs - begin() : Index(-1);
			return res;
		}

        Index indexOf(const String & str, Index id) const
		{
			return indexOf(str.begin(), id);
		}

        Index indexOf(const char * str) const
		{
			return indexOf(str, 0);
		}

        Index indexOf(const String & str) const
		{
			return indexOf(str.begin(), 0);
		}

        Index indexOf(char ch, Index id) const
		{
            const Index length = getLength();
            SLANG_ASSERT(id >= 0 && id <= length);

			if (!m_buffer)
				return Index(-1);

            const char* data = getData();
			for (Index i = id; i < length; i++)
				if (data[i] == ch)
					return i;
			return Index(-1);
		}

        Index indexOf(char ch) const
		{
			return indexOf(ch, 0);
		}

        Index lastIndexOf(char ch) const
		{            
            const Index length = getLength();
            const char* data = getData();

            // TODO(JS): If we know Index is signed we can do this a bit more simply

            for (Index i = length; i > 0; i--)
				if (data[i - 1] == ch)
					return i - 1;
			return Index(-1);
		}

		bool startsWith(const char * str) const // String str
		{
			if (!m_buffer)
				return false;
            Index strLen = Index(::strlen(str));
			if (strLen > getLength())
				return false;

            const char*const data = getData();

			for (Index i = 0; i < strLen; i++)
				if (str[i] != data[i])
					return false;
			return true;
		}

		bool startsWith(const String& str) const
		{
			return startsWith(str.begin());
		}

		bool endsWith(char const * str)  const // String str
		{
			if (!m_buffer)
				return false;

			const Index strLen = Index(::strlen(str));
            const Index len = getLength();

			if (strLen > len)
				return false;
            const char* data = getData();
			for (Index i = strLen; i > 0; i--)
				if (str[i-1] != data[len - strLen + i-1])
					return false;
			return true;
		}

		bool endsWith(const String & str) const
		{
			return endsWith(str.begin());
		}

		bool contains(const char * str) const // String str
		{
			return m_buffer && indexOf(str) != Index(-1); 
		}

		bool contains(const String & str) const
		{
			return contains(str.begin());
		}

		HashCode getHashCode() const
		{
			return Slang::getHashCode(StringRepresentation::asSlice(m_buffer));
		}

        UnownedStringSlice getUnownedSlice() const
        {
            return StringRepresentation::asSlice(m_buffer);
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
			Append(str.getBuffer(), str.getLength());
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
            m_buffer.setNull();
		}
	};

	int StringToInt(const String & str, int radix = 10);
	unsigned int StringToUInt(const String & str, int radix = 10);
	double StringToDouble(const String & str);
	float StringToFloat(const String & str);
}

#endif
