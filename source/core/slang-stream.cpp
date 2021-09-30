#include "slang-stream.h"
#ifdef _WIN32
#include <share.h>
#endif
#include "slang-io.h"

namespace Slang
{

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FileStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult Stream::readExactly(void* buffer, size_t length)
{
    size_t readBytes;
    SLANG_RETURN_ON_FAIL(read(buffer, length, readBytes));
    return (readBytes == length) ? SLANG_OK : SLANG_FAIL;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FileStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

FileStream::FileStream() :
    m_handle(nullptr),
    m_fileAccess(FileAccess::None),
    m_endReached(false)
{
}

SlangResult FileStream::init(const String& fileName, FileMode fileMode)
{
    const FileAccess access = (fileMode == FileMode::Open) ? FileAccess::Read : FileAccess::Write;
    return _init(fileName, fileMode, access, FileShare::None);
}

SlangResult FileStream::init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share)
{
    return _init(fileName, fileMode, access, share);
}

SlangResult FileStream::_init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share)
{
    // Make sure it's closed to start with
    close();

    if (access == FileAccess::None)
    {
        SLANG_ASSERT(!"FileAccess::None not valid to create a FileStream.");
        return SLANG_E_INVALID_ARG;
    }

    const char* mode = "rt";
    switch (fileMode)
    {
    case FileMode::Create:
        if (access == FileAccess::Read)
        {
            SLANG_ASSERT(!"Read-only access is incompatible with Create mode.");
            return SLANG_E_INVALID_ARG;
        }
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
            return SLANG_E_CANNOT_OPEN;
        }
        if (access == FileAccess::Read)
        {
            SLANG_ASSERT(!"Read-only access is incompatible with Create mode.");
            return SLANG_E_INVALID_ARG;
        }
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
        {
            SLANG_ASSERT(!"Read-only access is incompatible with Append mode.");
            return SLANG_E_INVALID_ARG;
        }
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
        SLANG_ASSERT(!"Invalid file share mode.");
        return SLANG_FAIL;
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
        return SLANG_E_CANNOT_OPEN;
    }

    // Just set the access specified
    m_fileAccess = access;
    return SLANG_OK;
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

SlangResult FileStream::seek(SeekOrigin seekOrigin, Int64 offset)
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
        SLANG_ASSERT(!"Unsupported seek origin.");
        return SLANG_FAIL;
    }

    // If endReached is intended to be like feof - then doing a seek will reset it
    m_endReached = false;

#ifdef _WIN32
    int rs = _fseeki64(m_handle, offset, fseekOrigin);
#else
    int rs = fseek(m_handle, (long int)offset, fseekOrigin);
#endif

    // If rs != 0 then the the seek failed
    SLANG_ASSERT(rs == 0);

    return (rs == 0) ? SLANG_OK : SLANG_FAIL;
}

SlangResult FileStream::read(void* buffer, size_t length, size_t& outBytesRead)
{
    auto bytesRead = fread_s(buffer, length, 1, length, m_handle);

    outBytesRead = bytesRead;
    if (bytesRead == 0 && length > 0)
    {
        // If we have reached the end, then reading nothing is ok.
        if (!m_endReached)
        {
            // If we are not at the end of the file we should be able to read some bytes
            if (!feof(m_handle))
            {
                return SLANG_FAIL;
            }
            m_endReached = true;
        }
    }
    return SLANG_OK;
}

SlangResult FileStream::write(const void* buffer, size_t length)
{
    auto bytesWritten = fwrite(buffer, 1, length, m_handle);
    return (bytesWritten == length) ? SLANG_OK : SLANG_FAIL;
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
        m_handle = nullptr;

        // If closed, can neither read or write
        m_fileAccess = FileAccess::None;
    }
}

bool FileStream::isEnd()
{
    return m_endReached;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MemoryStreamBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult MemoryStreamBase::seek(SeekOrigin origin, Int64 offset)
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
            SLANG_ASSERT(!"Unsupported seek origin.");
            return SLANG_E_NOT_IMPLEMENTED;
    }

    m_atEnd = false;

    // Clamp to the valid range
    pos = (pos < 0) ? 0 : pos;
    pos = (pos > Int64(m_contentsSize)) ? Int64(m_contentsSize) : pos;

    m_position = ptrdiff_t(pos);
    return SLANG_OK;
}

SlangResult MemoryStreamBase::read(void* buffer, size_t length, size_t& outReadBytes)
{
    outReadBytes = 0;
    if (!canRead())
    {
        SLANG_ASSERT(!"Cannot read this stream.");
        return SLANG_FAIL;
    }

    const size_t maxRead = size_t(m_contentsSize - m_position);
    if (maxRead == 0 && length > 0)
    {
        // At end of stream
        m_atEnd = true;        
        return SLANG_OK;
    }

    length = length > maxRead ? maxRead : length;

    ::memcpy(buffer, m_contents + m_position, length);
    m_position += ptrdiff_t(length);
    outReadBytes = length;

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! OwnedMemoryStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult OwnedMemoryStream::write(const void * buffer, size_t length)
{
    if (!canWrite())
    {
        SLANG_ASSERT(!"Cannot write this stream.");
        return SLANG_FAIL;
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
    return SLANG_OK;
}

} // namespace Slang
