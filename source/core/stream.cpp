#include "stream.h"
#ifdef _WIN32
#include <share.h>
#endif
#include "slang-io.h"

namespace Slang
{
	FileStream::FileStream(const Slang::String & fileName, FileMode fileMode)
	{
		Init(fileName, fileMode, fileMode==FileMode::Open?FileAccess::Read:FileAccess::Write, FileShare::None);
	}
	FileStream::FileStream(const Slang::String & fileName, FileMode fileMode, FileAccess access, FileShare share)
	{
		Init(fileName, fileMode, access, share);
	}
	void FileStream::Init(const Slang::String & fileName, FileMode fileMode, FileAccess access, FileShare share)
	{
		const wchar_t * mode = L"rt";
		const char* modeMBCS = "rt";
		switch (fileMode)
		{
		case Slang::FileMode::Create:
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
		case Slang::FileMode::Open:
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
		case Slang::FileMode::CreateNew:
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
		case Slang::FileMode::Append:
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
#ifdef _WIN32
        int shFlag = _SH_DENYRW;
        switch (share)
		{
		case Slang::FileShare::None:
			shFlag = _SH_DENYRW;
			break;
		case Slang::FileShare::ReadOnly:
			shFlag = _SH_DENYWR;
			break;
		case Slang::FileShare::WriteOnly:
			shFlag = _SH_DENYRD;
			break;
		case Slang::FileShare::ReadWrite:
			shFlag = _SH_DENYNO;
			break;
		default:
			throw ArgumentException("Invalid file share mode.");
			break;
		}
        if (share == Slang::FileShare::None)
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
#if defined(_WIN32) || defined(__CYGWIN__)
		fpos_t pos;
		fgetpos(handle, &pos);
		return pos;
#elif defined(__APPLE__)
		return ftell(handle);
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
		case Slang::SeekOrigin::Start:
			_origin = SEEK_SET;
			endReached = false;
			break;
		case Slang::SeekOrigin::End:
			_origin = SEEK_END;
            // JS TODO: This doesn't seem right, the offset can mean it's not at the end
			endReached = true;
			break;
		case Slang::SeekOrigin::Current:
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

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MemoryStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    void MemoryStream::Seek(SeekOrigin origin, Int64 offset)
    {
        Int64 pos = 0;
        switch (origin)
        {
        case Slang::SeekOrigin::Start:
            pos = offset;
            break;
        case Slang::SeekOrigin::End:
            pos = Int64(m_contents.Count()) + offset;
            break;
        case Slang::SeekOrigin::Current:
            pos = Int64(m_position) + offset;
            break;
        default:
            throw NotSupportedException("Unsupported seek origin.");
            break;
        }

        m_atEnd = false;

        // Clamp to the valid range
        pos = (pos < 0) ? 0 : pos;
        pos = (pos > Int64(m_contents.Count())) ? Int64(m_contents.Count()) : pos;

        m_position = UInt(pos);
    }

    Int64 MemoryStream::Read(void * buffer, Int64 length)
    {
        if (!CanRead())
        {
            throw IOException("Cannot read this stream.");
        }

        const Int64 maxRead = Int64(m_contents.Count() - m_position);
        
        if (maxRead == 0 && length > 0)
        {
            m_atEnd = true;
            throw EndOfStreamException("End of file is reached.");
        }

        length = length > maxRead ? maxRead : length;

        ::memcpy(buffer, m_contents.begin() + m_position, size_t(length));
        m_position += UInt(length);
        return maxRead;
    }
    
    Int64 MemoryStream::Write(const void * buffer, Int64 length)
    {
        if (!CanWrite())
        {
            throw IOException("Cannot write this stream.");
        }

        if (m_position == m_contents.Count())
        {
            m_contents.AddRange((const uint8_t*)buffer, UInt(length));
        }
        else
        {
            m_contents.InsertRange(m_position, (const uint8_t*)buffer, UInt(length));
        }

        m_atEnd = false;

        m_position += UInt(length);
        return length;
    }

}
