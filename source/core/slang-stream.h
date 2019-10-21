#ifndef SLANG_CORE_STREAM_H
#define SLANG_CORE_STREAM_H

#include "slang-basic.h"

namespace Slang
{
	class IOException : public Exception
	{
	public:
		IOException()
		{}
		IOException(const String & message)
			: Slang::Exception(message)
		{
		}
	};

	class EndOfStreamException : public IOException
	{
	public:
		EndOfStreamException()
		{}
		EndOfStreamException(const String & message)
			: IOException(message)
		{
		}
	};

	enum class SeekOrigin
	{
		Start, End, Current
	};

	class Stream : public RefObject
	{
	public:
        virtual ~Stream() {}
		virtual Int64 GetPosition()=0;
		virtual void Seek(SeekOrigin origin, Int64 offset)=0;
		virtual Int64 Read(void * buffer, Int64 length) = 0;
		virtual Int64 Write(const void * buffer, Int64 length) = 0;
		virtual bool IsEnd() = 0;
		virtual bool CanRead() = 0;
		virtual bool CanWrite() = 0;
		virtual void Close() = 0;
	};

	enum class FileMode
	{
		Create, Open, CreateNew, Append
	};

	enum class FileAccess
	{
		None = 0, Read = 1, Write = 2, ReadWrite = 3
	};

	enum class FileShare
	{
		None, ReadOnly, WriteOnly, ReadWrite
	};

    /// Base class for memory streams. Only supports reading and does NOT own contained data.
    class MemoryStreamBase : public Stream
    {
    public:
        typedef Stream Super;

        virtual Int64 GetPosition() SLANG_OVERRIDE { return m_position; }
        virtual void Seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE;
        virtual Int64 Read(void * buffer, Int64 length) SLANG_OVERRIDE;
        virtual Int64 Write(const void * buffer, Int64 length) SLANG_OVERRIDE { SLANG_UNUSED(buffer); SLANG_UNUSED(length); return 0; }
        virtual bool IsEnd() SLANG_OVERRIDE { return m_atEnd; }
        virtual bool CanRead() SLANG_OVERRIDE { return (int(m_access) & int(FileAccess::Read)) != 0; }
        virtual bool CanWrite() SLANG_OVERRIDE { return (int(m_access) & int(FileAccess::Write)) != 0; }
        virtual void Close() SLANG_OVERRIDE { m_access = FileAccess::None; }

        MemoryStreamBase(FileAccess access = FileAccess::Read, const void* data = nullptr, size_t dataSize = 0):
            m_data((const uint8_t*)data),
            m_dataSize(dataSize),
            m_position(0),
            m_atEnd(false),
            m_access(access)
        {
        }

    protected:
        const uint8_t* m_data;

        // Use ptrdiff_t (as opposed to size_t) as makes maths simpler
        ptrdiff_t m_dataSize;
        ptrdiff_t m_position;

        bool m_atEnd;           ///< Happens when a read is done and nothing can be returned because already at end

        FileAccess m_access;
    };

    /// Memory stream that owns it's contents
    class MemoryStream : public MemoryStreamBase
    {
    public:
        typedef MemoryStreamBase Super;

        virtual Int64 Write(const void * buffer, Int64 length) SLANG_OVERRIDE;

            /// Set the contents
        void setContents(const void* data, size_t size)
        {
            m_contents.setCount(size);
            ::memcpy(m_contents.getBuffer(), data, size);
             _update();
        }

        void swapContents(List<uint8_t>& rhs)
        {
            rhs.swapWith(m_contents);
            _update();
        }

        MemoryStream(FileAccess access) :
            Super(access)
        {}

    protected:
        void _update()
        {
            m_data = m_contents.getBuffer();
            m_dataSize = ptrdiff_t(m_contents.getCount());
            // Make sure position is in range
            m_position = (m_position > m_dataSize) ? m_dataSize : m_position;
            // We only have at end when a read has happened past the end. So here we reset, as seems most sensible. 
            m_atEnd = false;
        }

        List<uint8_t> m_contents;
    };

	class FileStream : public Stream
	{
	private:
		FILE * handle;
		FileAccess fileAccess;
		bool endReached = false;
		void Init(const Slang::String & fileName, FileMode fileMode, FileAccess access, FileShare share);
	public:
		FileStream(const Slang::String & fileName, FileMode fileMode = FileMode::Open);
		FileStream(const Slang::String & fileName, FileMode fileMode, FileAccess access, FileShare share);
		~FileStream();
	public:
		virtual Int64 GetPosition();
		virtual void Seek(SeekOrigin origin, Int64 offset);
		virtual Int64 Read(void * buffer, Int64 length);
		virtual Int64 Write(const void * buffer, Int64 length);
		virtual bool CanRead();
		virtual bool CanWrite();
		virtual void Close();
		virtual bool IsEnd();
	};
}

#endif
