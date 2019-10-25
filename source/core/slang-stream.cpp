#include "slang-stream.h"
#ifdef _WIN32
#include <share.h>
#endif
#include "slang-io.h"

namespace Slang
{

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FileStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

FileStream::FileStream(const String& fileName, FileMode fileMode)
{
	_init(fileName, fileMode, fileMode==FileMode::Open?FileAccess::Read:FileAccess::Write, FileShare::None);
}

FileStream::FileStream(const String& fileName, FileMode fileMode, FileAccess access, FileShare share)
{
	_init(fileName, fileMode, access, share);
}

void FileStream::_init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share)
{
	const wchar_t * mode = L"rt";
	const char* modeMBCS = "rt";
	switch (fileMode)
	{
	case FileMode::Create:
		if (access == FileAccess::Read)
			throw ArgumentException("Read-only access is incompatible with Create mode.");
		else if (access == FileAccess::ReadWrite)
		{
			mode = L"w+b";
			modeMBCS = "w+b";
			this->m_fileAccess = FileAccess::ReadWrite;
		}
		else
		{
			mode = L"wb";
			modeMBCS = "wb";
			this->m_fileAccess = FileAccess::Write;
		}
		break;
	case FileMode::Open:
		if (access == FileAccess::Read)
		{
			mode = L"rb";
			modeMBCS = "rb";
			this->m_fileAccess = FileAccess::Read;
		}
		else if (access == FileAccess::ReadWrite)
		{
			mode = L"r+b";
			modeMBCS = "r+b";
			this->m_fileAccess = FileAccess::ReadWrite;
		}
		else
		{
			mode = L"wb";
			modeMBCS = "wb";
			this->m_fileAccess = FileAccess::Write;
		}
		break;
	case FileMode::CreateNew:
		if (File::exists(fileName))
		{
			throw IOException("Failed opening '" + fileName + "', file already exists.");
		}
		if (access == FileAccess::Read)
			throw ArgumentException("Read-only access is incompatible with Create mode.");
		else if (access == FileAccess::ReadWrite)
		{
			mode = L"w+b";
			this->m_fileAccess = FileAccess::ReadWrite;
		}
		else
		{
			mode = L"wb";
			this->m_fileAccess = FileAccess::Write;
		}
		break;
	case FileMode::Append:
		if (access == FileAccess::Read)
			throw ArgumentException("Read-only access is incompatible with Append mode.");
		else if (access == FileAccess::ReadWrite)
		{
			mode = L"a+b";
			this->m_fileAccess = FileAccess::ReadWrite;
		}
		else
		{
			mode = L"ab";
			this->m_fileAccess = FileAccess::Write;
		}
		break;
	default:
		break;
	}
#ifdef _WIN32
    int shFlag = _SH_DENYRW;
    switch (share)
	{
	case FileShare::None:
		shFlag = _SH_DENYRW;
		break;
	case FileShare::ReadOnly:
		shFlag = _SH_DENYWR;
		break;
	case FileShare::WriteOnly:
		shFlag = _SH_DENYRD;
		break;
	case FileShare::ReadWrite:
		shFlag = _SH_DENYNO;
		break;
	default:
		throw ArgumentException("Invalid file share mode.");
		break;
	}
    if (share == FileShare::None)
#pragma warning(suppress:4996)
        m_handle = _wfopen(fileName.toWString(), mode);
    else
		m_handle = _wfsopen(fileName.toWString(), mode, shFlag);
#else
	m_handle = fopen(fileName.getBuffer(), modeMBCS);
#endif
	if (!m_handle)
	{
		throw IOException("Cannot open file '" + fileName + "'");
	}
}

FileStream::~FileStream()
{
	close();
}

Int64 FileStream::getPosition()
{
#if defined(_WIN32) || defined(__CYGWIN__)
	fpos_t pos;
	fgetpos(m_handle, &pos);
	return pos;
#elif defined(__APPLE__)
	return ftell(m_handle);
#else 
	fpos64_t pos;
	fgetpos64(m_handle, &pos);
	return *(Int64*)(&pos);
#endif
}
void FileStream::seek(SeekOrigin origin, Int64 offset)
{
	int _origin;
	switch (origin)
	{
	case SeekOrigin::Start:
		_origin = SEEK_SET;
		m_endReached = false;
		break;
	case SeekOrigin::End:
		_origin = SEEK_END;
        // JS TODO: This doesn't seem right, the offset can mean it's not at the end
		m_endReached = true;
		break;
	case SeekOrigin::Current:
		_origin = SEEK_CUR;
		m_endReached = false;
		break;
	default:
		throw NotSupportedException("Unsupported seek origin.");
		break;
	}
#ifdef _WIN32
	int rs = _fseeki64(m_handle, offset, _origin);
#else
	int rs = fseek(m_handle, (int)offset, _origin);
#endif
	if (rs != 0)
	{
		throw IOException("FileStream seek failed.");
	}
}

size_t FileStream::read(void* buffer, size_t length)
{
	auto bytes = fread_s(buffer, length, 1, length, m_handle);
	if (bytes == 0 && length > 0)
	{
		if (!feof(m_handle))
			throw IOException("FileStream read failed.");
		else if (m_endReached)
			throw EndOfStreamException("End of file is reached.");
		m_endReached = true;
	}
	return bytes;
}

size_t FileStream::write(const void* buffer, size_t length)
{
	auto bytes = fwrite(buffer, 1, length, m_handle);
	if (bytes < length)
	{
		throw IOException("FileStream write failed.");
	}
	return bytes;
}

bool FileStream::canRead()
{
	return ((int)m_fileAccess & (int)FileAccess::Read) != 0;
}

bool FileStream::canWrite()
{
	return ((int)m_fileAccess & (int)FileAccess::Write) != 0;
}

void FileStream::close()
{
	if (m_handle)
	{
		fclose(m_handle);
		m_handle = 0;
	}
}

bool FileStream::isEnd()
{
	return m_endReached;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MemoryStreamBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void MemoryStreamBase::seek(SeekOrigin origin, Int64 offset)
{
    Int64 pos = 0;
    switch (origin)
    {
        case SeekOrigin::Start:
            pos = offset;
            break;
        case SeekOrigin::End:
            pos = Int64(m_contentsSize) + offset;
            break;
        case SeekOrigin::Current:
            pos = Int64(m_position) + offset;
            break;
        default:
            throw NotSupportedException("Unsupported seek origin.");
            break;
    }

    m_atEnd = false;

    // Clamp to the valid range
    pos = (pos < 0) ? 0 : pos;
    pos = (pos > Int64(m_contentsSize)) ? Int64(m_contentsSize) : pos;

    m_position = ptrdiff_t(pos);
}

size_t MemoryStreamBase::read(void* buffer, size_t length)
{
    if (!canRead())
    {
        throw IOException("Cannot read this stream.");
    }

    const size_t maxRead = size_t(m_contentsSize - m_position);
    if (maxRead == 0 && length > 0)
    {
        m_atEnd = true;
        throw EndOfStreamException("End of file is reached.");
    }

    length = length > maxRead ? maxRead : length;

    ::memcpy(buffer, m_contents + m_position, length);
    m_position += ptrdiff_t(length);
    return maxRead;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! OwnedMemoryStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

size_t OwnedMemoryStream::write(const void * buffer, size_t length)
{
    if (!canWrite())
    {
        throw IOException("Cannot write this stream.");
    }

    if (m_position == m_ownedContents.getCount())
    {
        m_ownedContents.addRange((const uint8_t*)buffer, Index(length));
    }
    else
    {
        m_ownedContents.insertRange(m_position, (const uint8_t*)buffer, Index(length));
    }

    m_contents = m_ownedContents.getBuffer();
    m_contentsSize = ptrdiff_t(m_ownedContents.getCount());

    m_atEnd = false;

    m_position += ptrdiff_t(length);
    return length;
}

} // namespace Slang
