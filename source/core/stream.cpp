#include "Stream.h"
#ifdef _WIN32
#include <share.h>
#endif
#include "slang-io.h"

namespace CoreLib
{
	namespace IO
	{
		using namespace CoreLib::Basic;
		FileStream::FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode)
		{
			Init(fileName, fileMode, fileMode==FileMode::Open?FileAccess::Read:FileAccess::Write, FileShare::None);
		}
		FileStream::FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share)
		{
			Init(fileName, fileMode, access, share);
		}
		void FileStream::Init(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share)
		{
			const wchar_t * mode = L"rt";
			const char* modeMBCS = "rt";
			switch (fileMode)
			{
			case CoreLib::IO::FileMode::Create:
				if (access == FileAccess::Read)
					throw ArgumentException("Read-only access is incompatible with Create mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"w+b";
					modeMBCS = "w+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"wb";
					modeMBCS = "wb";
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::Open:
				if (access == FileAccess::Read)
				{
					mode = L"rb";
					modeMBCS = "rb";
					this->fileAccess = FileAccess::Read;
				}
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"r+b";
					modeMBCS = "r+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"wb";
					modeMBCS = "wb";
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::CreateNew:
				if (File::Exists(fileName))
				{
					throw IOException("Failed opening '" + fileName + "', file already exists.");
				}
				if (access == FileAccess::Read)
					throw ArgumentException("Read-only access is incompatible with Create mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"w+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"wb";
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::Append:
				if (access == FileAccess::Read)
					throw ArgumentException("Read-only access is incompatible with Append mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"a+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"ab";
					this->fileAccess = FileAccess::Write;
				}
				break;
			default:
				break;
			}
			int shFlag;
#ifdef _WIN32
			switch (share)
			{
			case CoreLib::IO::FileShare::None:
				shFlag = _SH_DENYRW;
				break;
			case CoreLib::IO::FileShare::ReadOnly:
				shFlag = _SH_DENYWR;
				break;
			case CoreLib::IO::FileShare::WriteOnly:
				shFlag = _SH_DENYRD;
				break;
			case CoreLib::IO::FileShare::ReadWrite:
				shFlag = _SH_DENYNO;
				break;
			default:
				throw ArgumentException("Invalid file share mode.");
				break;
			}
            if (share == CoreLib::IO::FileShare::None)
#pragma warning(suppress:4996)
                handle = _wfopen(fileName.ToWString(), mode);
            else
			    handle = _wfsopen(fileName.ToWString(), mode, shFlag);
#else
			handle = fopen(fileName.Buffer(), modeMBCS);
#endif
			if (!handle)
			{
				throw IOException("Cannot open file '" + fileName + "'");
			}
		}
		FileStream::~FileStream()
		{
			Close();
		}
		Int64 FileStream::GetPosition()
		{
#ifdef _WIN32
			fpos_t pos;
			fgetpos(handle, &pos);
			return pos;
#else
			fpos64_t pos;
			fgetpos64(handle, &pos);
			return *(Int64*)(&pos);
#endif
		}
		void FileStream::Seek(SeekOrigin origin, Int64 offset)
		{
			int _origin;
			switch (origin)
			{
			case CoreLib::IO::SeekOrigin::Start:
				_origin = SEEK_SET;
				endReached = false;
				break;
			case CoreLib::IO::SeekOrigin::End:
				_origin = SEEK_END;
				endReached = true;
				break;
			case CoreLib::IO::SeekOrigin::Current:
				_origin = SEEK_CUR;
				endReached = false;
				break;
			default:
				throw NotSupportedException("Unsupported seek origin.");
				break;
			}
#ifdef _WIN32
			int rs = _fseeki64(handle, offset, _origin);
#else
			int rs = fseek(handle, (int)offset, _origin);
#endif
			if (rs != 0)
			{
				throw IOException("FileStream seek failed.");
			}
		}
		Int64 FileStream::Read(void * buffer, Int64 length)
		{
			auto bytes = fread_s(buffer, (size_t)length, 1, (size_t)length, handle);
			if (bytes == 0 && length > 0)
			{
				if (!feof(handle))
					throw IOException("FileStream read failed.");
				else if (endReached)
					throw EndOfStreamException("End of file is reached.");
				endReached = true;
			}
			return (int)bytes;
		}
		Int64 FileStream::Write(const void * buffer, Int64 length)
		{
			auto bytes = (Int64)fwrite(buffer, 1, (size_t)length, handle);
			if (bytes < length)
			{
				throw IOException("FileStream write failed.");
			}
			return bytes;
		}
		bool FileStream::CanRead()
		{
			return ((int)fileAccess & (int)FileAccess::Read) != 0;
		}
		bool FileStream::CanWrite()
		{
			return ((int)fileAccess & (int)FileAccess::Write) != 0;
		}
		void FileStream::Close()
		{
			if (handle)
			{
				fclose(handle);
				handle = 0;
			}
		}
		bool FileStream::IsEnd()
		{
			return endReached;
		}
	}
}
