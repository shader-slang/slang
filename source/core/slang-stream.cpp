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
    const FileAccess access = (fileMode == FileMode::Open) ? FileAccess::Read : FileAccess::Write;
    _init(fileName, fileMode, access, FileShare::None);
}

FileStream::FileStream(const String& fileName, FileMode fileMode, FileAccess access, FileShare share)
{
    _init(fileName, fileMode, access, share);
}

void FileStream::_init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share)
{
    if (access == FileAccess::None)
    {
        throw ArgumentException("FileAccess::None not valid to create a FileStream.");
    }

    // Default to no access, until stream is fully constructed
    m_fileAccess = FileAccess::None;

    const char* mode = "rt";
    switch (fileMode)
    {
    case FileMode::Create:
        if (access == FileAccess::Read)
            throw ArgumentException("Read-only access is incompatible with Create mode.");
        else if (access == FileAccess::ReadWrite)
        {
            mode = "w+b";
        }
        else
        {
            mode = "wb";
        }
        break;
    case FileMode::Open:
        if (access == FileAccess::Read)
        {
            mode = "rb";
        }
        else if (access == FileAccess::ReadWrite)
        {
            mode = "r+b";
        }
        else
        {
            mode = "wb";
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
            mode = "w+b";
        }
        else
        {
            mode = "wb";
        }
        break;
    case FileMode::Append:
        if (access == FileAccess::Read)
            throw ArgumentException("Read-only access is incompatible with Append mode.");
        else if (access == FileAccess::ReadWrite)
        {
            mode = "a+b";
        }
        else
        {
            mode = "ab";
        }
        break;
    default:
        break;
    }
#ifdef _WIN32

    // NOTE! This works because we know all the characters in the mode
    // are encoded directly as the same value in a wchar_t.
    // 
    // Work out the length *including* terminating 0
    const Index modeLength = Index(::strlen(mode)) + 1;
    wchar_t wideMode[8];
    SLANG_ASSERT(modeLength <= SLANG_COUNT_OF(wideMode));

    // Copy to wchar_t 
    for (Index i = 0; i < modeLength ; ++i)
    {
        wideMode[i] = wchar_t(mode[i]);
    }
   
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
        m_handle = _wfopen(fileName.toWString(), wideMode);
    else
        m_handle = _wfsopen(fileName.toWString(), wideMode, shFlag);
#else
    m_handle = fopen(fileName.getBuffer(), mode);
#endif
    if (!m_handle)
    {
        throw IOException("Cannot open file '" + fileName + "'");
    }

    // Just set the access specified
    m_fileAccess = access;
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

void FileStream::seek(SeekOrigin seekOrigin, Int64 offset)
{
    int fseekOrigin;
    switch (seekOrigin)
    {
    case SeekOrigin::Start:
        fseekOrigin = SEEK_SET;
        break;
    case SeekOrigin::End:
        fseekOrigin = SEEK_END;
        break;
    case SeekOrigin::Current:
        fseekOrigin = SEEK_CUR;
        break;
    default:
        throw NotSupportedException("Unsupported seek origin.");
        break;
    }

    // If endReached is intended to be like feof - then doing a seek will reset it
    m_endReached = false;

#ifdef _WIN32
    int rs = _fseeki64(m_handle, offset, fseekOrigin);
#else
    int rs = fseek(m_handle, (long int)offset, fseekOrigin);
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

        // If closed, can neither read or write
        m_fileAccess = FileAccess::None;
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
