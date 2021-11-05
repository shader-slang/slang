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

SlangResult FileStream::flush()
{
    if (m_handle && canWrite())
    {
        fflush(m_handle);
        return SLANG_OK;
    }
    return SLANG_E_NOT_AVAILABLE;
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! BufferedReadStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void BufferedReadStream::_advanceStartIndex(Index byteCount)
{
    SLANG_ASSERT(m_count >= byteCount && byteCount >= 0);

    m_startIndex += byteCount;
    m_count -= byteCount;

    if (m_count == 0)
    {
        m_startIndex = 0;
        return;
    }

    const Index bufferCount = m_buffer.getCount();
    
    // Wrap around if necessary.
    // NOTE! It's important if m_startIndex is pointing to end index (ie bufferCount),
    // it is wrapped around to 0.
    m_startIndex = (m_startIndex >= bufferCount) ? (m_startIndex - bufferCount) : m_startIndex;
}

Int64 BufferedReadStream::getPosition()
{
    return m_stream ? (m_stream->getPosition() - m_count) : 0;
}

SlangResult BufferedReadStream::seek(SeekOrigin origin, Int64 offset)
{
    if (!m_stream)
    {
        return SLANG_FAIL;
    }

    if (origin == SeekOrigin::End || origin == SeekOrigin::Start || offset < 0 || offset >= m_count)
    {
        m_startIndex = 0;
        m_count = 0;

        return m_stream->seek(origin, offset);
    }

    _advanceStartIndex(Index(offset));
}

SlangResult BufferedReadStream::read(void* inBuffer, size_t length, size_t& outReadBytes)
{
    Byte* buffer = (Byte*)inBuffer;

    size_t totalReadBytes = 0;
    outReadBytes = 0;

    update();

    const Index bufferCount = m_buffer.getCount();

    while (length > 0)
    {
        if (m_count > 0)
        {
            // If it wraps around, deal with bytes up to end of buffer initially
            if (m_startIndex + m_count > bufferCount)
            {
                const size_t maxRead = bufferCount - m_startIndex;
                const size_t readCount = std::min(maxRead, length);

                // Copy down
                ::memcpy(buffer, m_buffer.getBuffer() + m_startIndex, readCount);

                _advanceStartIndex(Index(readCount));
                buffer += readCount;
                length -= readCount;
            }
            else
            {
                const size_t readCount = std::min(length, size_t(m_count));

                ::memcpy(buffer, m_buffer.getBuffer() + m_startIndex, readCount);

                _advanceStartIndex(Index(readCount));
                buffer += readCount;
                length -= readCount;
            }
        }
        else
        {
            if (m_stream == nullptr)
            {
                break;
            }

            // Read from underlying buffer
            size_t readBytes;
            SlangResult res = m_stream->read(buffer, length, readBytes);

            outReadBytes = totalReadBytes + readBytes;
            return res;
        }
    }

    outReadBytes = totalReadBytes;
    return SLANG_OK;
}

SlangResult BufferedReadStream::write(const void* buffer, size_t length)
{
    SLANG_UNUSED(buffer);
    SLANG_UNUSED(length);

    return SLANG_E_NOT_AVAILABLE;
}

bool BufferedReadStream::canRead()
{
    return m_count > 0 || (m_stream && m_stream->canRead());
}

bool BufferedReadStream::canWrite()
{
    return false;
}

void BufferedReadStream::close()
{
    if (m_stream)
    {
        m_stream->close();
        m_stream.setNull();
    }
}

bool BufferedReadStream::isEnd()
{
    return m_count == 0 && (m_stream == nullptr || m_stream->isEnd());
}

SlangResult BufferedReadStream::flush()
{
    return SLANG_E_NOT_AVAILABLE;
}

Byte* BufferedReadStream::getCanonicalBuffer()
{
    Byte* data = m_buffer.getBuffer();
    if (m_startIndex == 0)
    {
        return data;
    }

    const Index bufferCount = m_buffer.getCount();
    if (m_count == 0)
    {
        // Don't need to do anything in terms of shifting
    }
    else if (m_startIndex + m_count <= bufferCount)
    {
        ::memmove(data, data + m_startIndex, m_count);
    }
    else
    {
        // We have to be wrapped around...
        const Index startSize = bufferCount - m_startIndex;
        const Index endSize = (m_startIndex + m_count) - bufferCount;

        const Index gapSize = bufferCount - m_count;

        // Lets rearrange such that the memory is linear, and whilst we are at it
        // lets always make 0 indexed so 'canonical'.
        // Ie
        // EEEEEEEGGGGGSSSS
        // Becomes
        // SSSSEEEEEEEEGGGG

        if (startSize <= gapSize)
        {
            // We can just shuffle around, in place
            ::memmove(data + startSize, data, endSize);
            ::memmove(data, data + m_startIndex, startSize);
        }
        else
        {
            // Copy the smallest section
            if (startSize < endSize)
            {
                List<Byte> work;
                work.setCount(startSize);
                ::memcpy(work.getBuffer(), data + m_startIndex, startSize);
                ::memmove(data + startSize, data, endSize);
                ::memcpy(data, work.getBuffer(), startSize);
            }
            else
            {
                List<Byte> work;
                work.setCount(endSize);
                ::memcpy(work.getBuffer(), data, endSize);
                ::memmove(data, data + m_startIndex, startSize);
                ::memcpy(data + startSize, work.getBuffer(), endSize);
            }
        }
    }

    m_startIndex = 0;
    return data;
}

Byte* BufferedReadStream::getLinearBuffer()
{
    const Index bufferCount = m_buffer.getCount();
    Byte* data = m_buffer.getBuffer();
    if (m_startIndex + m_count <= bufferCount)
    {
        return data + m_startIndex;
    }
    return getCanonicalBuffer();
}

void BufferedReadStream::update(size_t readSize)
{
    if (m_stream == nullptr)
    {
        return;
    }

    {
        const Index bufferSize = m_buffer.getCount();
        const Index gapSize = bufferSize - m_count;

        if (size_t(gapSize) < readSize)
        {
            // In order to read, we'll have to reallocate.
            getCanonicalBuffer();
            SLANG_ASSERT(m_startIndex == 0);

            m_buffer.setCount(m_count + readSize);
            m_buffer.setCount(m_buffer.getCapacity());
        }
        else
        {
            /// GGGGSSSSSGGGG
            if (m_startIndex + m_count <= bufferSize)
            {
                const Index endGapSize = bufferSize - (m_startIndex + m_count);
                if (size_t(endGapSize) < readSize)
                {
                    getCanonicalBuffer();
                }
            }
            else
            {
                // The gap is a contiguous run of the full gap size
                // EEEEGGGGGSSSS
            }
        }
    }

    Byte* dst = m_buffer.getBuffer() + m_startIndex + m_count;

    size_t readBytes = 0;
    m_stream->read(dst, readSize, readBytes);
    m_count += Index(readBytes);
}

} // namespace Slang
