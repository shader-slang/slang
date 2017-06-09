#ifndef CORE_LIB_STREAM_H
#define CORE_LIB_STREAM_H

#include "Basic.h"

namespace CoreLib
{
	namespace IO
	{
		using CoreLib::Basic::Exception;
		using CoreLib::Basic::String;
		using CoreLib::Basic::RefPtr;

		class IOException : public Exception
		{
		public:
			IOException()
			{}
			IOException(const String & message)
				: CoreLib::Basic::Exception(message)
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

		class Stream : public CoreLib::Basic::Object
		{
		public:
			virtual Int64 GetPosition()=0;
			virtual void Seek(SeekOrigin origin, Int64 offset)=0;
			virtual Int64 Read(void * buffer, Int64 length) = 0;
			virtual Int64 Write(const void * buffer, Int64 length) = 0;
			virtual bool IsEnd() = 0;
			virtual bool CanRead() = 0;
			virtual bool CanWrite() = 0;
			virtual void Close() = 0;
		};

		class BinaryReader
		{
		private:
			RefPtr<Stream> stream;
			inline void Throw(Int64 val)
			{
				if (val == 0)
					throw IOException("read operation failed.");
			}
		public:
			BinaryReader(RefPtr<Stream> stream)
			{
				this->stream = stream;
			}
			Stream * GetStream()
			{
				return stream.Ptr();
			}
			void ReleaseStream()
			{
				stream.Release();
			}
			template<typename T>
			void Read(T * buffer, int count)
			{
				stream->Read(buffer, sizeof(T)*(Int64)count);
			}
			template<typename T>
			void Read(T & buffer)
			{
				Throw(stream->Read(&buffer, sizeof(T)));
			}
			template<typename T>
			void Read(List<T> & buffer)
			{
				int count = ReadInt32();
				buffer.SetSize(count);
				Read(buffer.Buffer(), count);
			}
			void Read(String & buffer)
			{
				buffer = ReadString();
			}
			int ReadInt32()
			{
				int rs;
				Throw(stream->Read(&rs, sizeof(int)));
				return rs;
			}
			short ReadInt16()
			{
				short rs;
				Throw(stream->Read(&rs, sizeof(short)));
				return rs;
			}
			Int64 ReadInt64()
			{
				Int64 rs;
				Throw(stream->Read(&rs, sizeof(Int64)));
				return rs;
			}
			float ReadFloat()
			{
				float rs;
				Throw(stream->Read(&rs, sizeof(float)));
				return rs;
			}
			double ReadDouble()
			{
				double rs;
				Throw(stream->Read(&rs, sizeof(double)));
				return rs;
			}
			char ReadChar()
			{
				char rs;
				Throw(stream->Read(&rs, sizeof(char)));
				return rs;
			}
			String ReadString()
			{
				int len = ReadInt32();
				char * buffer = new char[len+1];
				try
				{
					Throw(stream->Read(buffer, len));
				}
				catch(IOException & e)
				{
					delete [] buffer;
					throw e;
				}
				buffer[len] = 0;
				return String::FromBuffer(buffer, len);
			}
		};

		class BinaryWriter
		{
		private:
			RefPtr<Stream> stream;
		public:
			BinaryWriter(RefPtr<Stream> stream)
			{
				this->stream = stream;
			}
			Stream * GetStream()
			{
				return stream.Ptr();
			}
			template<typename T>
			void Write(const T& val)
			{
				stream->Write(&val, sizeof(T));
			}
			template<typename T>
			void Write(T * buffer, int count)
			{
				stream->Write(buffer, sizeof(T)*(Int64)count);
			}
			template<typename T>
			void Write(const List<T> & list)
			{
				Write(list.Count());
				stream->Write(list.Buffer(), sizeof(T)*list.Count());
			}
			void Write(const String & str)
			{
				Write(str.Length());
				Write(str.Buffer(), str.Length());
			}
			void ReleaseStream()
			{
				stream.Release();
			}
			void Close()
			{
				stream->Close();
			}
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
			void Init(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share);
		public:
			FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode = FileMode::Open);
			FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share);
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

		class MemoryStream : public Stream
		{
		private:
			CoreLib::List<unsigned char> writeBuffer;
			CoreLib::ArrayView<unsigned char> readBuffer;
			int ptr = 0;
			bool isReadStream;
		public:
			MemoryStream()
			{
				isReadStream = false;
			}
			MemoryStream(unsigned char * mem, int length)
			{
				isReadStream = true;
				readBuffer = MakeArrayView(mem, length);
			}
			MemoryStream(CoreLib::ArrayView<unsigned char> source)
			{
				isReadStream = true;
				readBuffer = source;
			}
			virtual Int64 GetPosition()
			{
				return ptr;
			}
			virtual void Seek(SeekOrigin origin, Int64 offset)
			{
				if (origin == SeekOrigin::Start)
					ptr = (int)offset;
				else if (origin == SeekOrigin::End)
				{
					if (isReadStream)
						ptr = readBuffer.Count() + (int)offset;
					else
						ptr = writeBuffer.Count() + (int)offset;
				}
			}
			virtual Int64 Read(void * pbuffer, Int64 length)
			{
				Int64 i;
				for (i = 0; i < length; i++)
				{
					if (ptr + i < readBuffer.Count())
					{
						((unsigned char*)pbuffer)[i] = readBuffer[(int)(ptr + i)];
					}
					else
						break;
				}
				return i;
			}
			virtual Int64 Write(const void * pbuffer, Int64 length)
			{
				writeBuffer.SetSize(ptr);
				if (pbuffer)
					writeBuffer.AddRange((unsigned char *)pbuffer, (int)length);
				else
					for (auto i = 0; i < length; i++)
						writeBuffer.Add(0);
				ptr = writeBuffer.Count();
				return length;
			}
			virtual bool CanRead()
			{
				return isReadStream;
			}
			virtual bool CanWrite()
			{
				return !isReadStream;
			}
			virtual void Close()
			{
				writeBuffer.SetSize(0);
				writeBuffer.Compress();
			}
			virtual bool IsEnd()
			{
				if (isReadStream)
					return ptr >= readBuffer.Count();
				else
					return ptr == writeBuffer.Count();
			}
			void * GetBuffer()
			{
				if (isReadStream)
					return readBuffer.Buffer();
				else
					return writeBuffer.Buffer();
			}
			int GetBufferSize()
			{
				if (isReadStream)
					return readBuffer.Count();
				else
					return writeBuffer.Count();
			}
		};
	}
}

#endif
