#ifndef CORE_LIB_STREAM_H
#define CORE_LIB_STREAM_H

#include "basic.h"

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

    class MemoryStream : public Stream
    {
    public:
        virtual Int64 GetPosition() SLANG_OVERRIDE { return m_position;  }
        virtual void Seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE;
        virtual Int64 Read(void * buffer, Int64 length) SLANG_OVERRIDE;
        virtual Int64 Write(const void * buffer, Int64 length) SLANG_OVERRIDE;
        virtual bool IsEnd() SLANG_OVERRIDE { return m_atEnd; }
        virtual bool CanRead() SLANG_OVERRIDE { return (int(m_access) & int(FileAccess::Read)) != 0;  }
        virtual bool CanWrite() SLANG_OVERRIDE { return (int(m_access) & int(FileAccess::Write)) != 0; }
        virtual void Close() SLANG_OVERRIDE { m_access = FileAccess::None;  }

        MemoryStream(FileAccess access) :
            m_access(access),
            m_position(0),
            m_atEnd(false)
        {}

        UInt m_position;

        bool m_atEnd;           ///< Happens when a read is done and nothing can be returned because already at end

        FileAccess m_access;
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
