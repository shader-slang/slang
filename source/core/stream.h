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
		Read = 1, Write = 2, ReadWrite = 3
	};

	enum class FileShare
	{
		None, ReadOnly, WriteOnly, ReadWrite
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
