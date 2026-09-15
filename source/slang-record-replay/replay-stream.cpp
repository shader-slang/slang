#include "slang-record-replay/replay-stream.h"

#include <limits>

namespace SlangRecord
{

ReplayStream::ReplayStream(const void* data, size_t size)
    : m_isReading(true)
{
    SLANG_RELEASE_ASSERT(data || size == 0);
    if (size > 0)
    {
        m_buffer.setCount(size);
        std::memcpy(m_buffer.getBuffer(), data, size);
    }
}

ReplayStream ReplayStream::loadFromFile(const char* path)
{
    ReplayStream stream;
    stream.m_isReading = true;

    List<unsigned char> contents;
    SlangResult result = File::readAllBytes(String(path), contents);
    if (SLANG_FAILED(result))
    {
        // Communicate the IO failure through the returned stream's failed state; the caller checks
        // isFailed() rather than catching.
        stream.setError(String("Failed to open file for reading: ") + path);
        return stream;
    }

    stream.m_buffer = Slang::_Move(contents);
    return stream;
}

ReplayStream::ReplayStream(ReplayStream&& other)
{
    *this = Slang::_Move(other);
}

ReplayStream& ReplayStream::operator=(ReplayStream&& other)
{
    if (this == &other)
    {
        return *this;
    }

    closeMirrorFile();
    m_buffer = Slang::_Move(other.m_buffer);
    m_position = other.m_position;
    m_isReading = other.m_isReading;
    m_mirrorFile = Slang::_Move(other.m_mirrorFile);
    // Move the failed state too: loadFromFile reports IO failure through the returned stream, so a
    // move must not silently drop it.
    m_failed = other.m_failed;
    m_errorMessage = Slang::_Move(other.m_errorMessage);

    other.m_position = 0;
    other.m_isReading = false;
    other.m_failed = false;
    other.m_errorMessage = String();

    return *this;
}

ReplayStream::~ReplayStream()
{
    closeMirrorFile();
}

void ReplayStream::write(const void* data, size_t size)
{
    if (m_failed)
        return;

    if (m_isReading)
    {
        setError("Cannot write to a reading stream");
        return;
    }

    if (size == 0)
        return;

    SLANG_RELEASE_ASSERT(data);

    if (size > (std::numeric_limits<size_t>::max)() - m_position)
    {
        setError("Write past maximum stream size");
        return;
    }

    size_t newSize = m_position + size;
    const size_t maxListCount = size_t((std::numeric_limits<Slang::Index>::max)());
    if (newSize > maxListCount)
    {
        setError("Write past maximum stream size");
        return;
    }

    if (newSize > size_t(m_buffer.getCapacity()))
    {
        size_t reserveSize = newSize;
        if (reserveSize <= maxListCount / 2)
            reserveSize *= 2;
        else
            reserveSize = maxListCount;
        m_buffer.reserve(Slang::Index(reserveSize));
    }
    if (newSize > size_t(m_buffer.getCount()))
    {
        m_buffer.setCount(Slang::Index(newSize));
    }

    const size_t writeOffset = m_position;
    std::memcpy(m_buffer.getBuffer() + writeOffset, data, size);
    m_position += size;

    if (m_mirrorFile)
    {
        m_mirrorFile->seek(SeekOrigin::Start, Slang::Int64(writeOffset));
        m_mirrorFile->write(data, size);
        m_mirrorFile->flush();
    }
}

void ReplayStream::read(void* data, size_t size)
{
    // A read on an already-failed stream is a no-op; `data` is left as the caller initialized it.
    if (m_failed)
        return;

    if (!m_isReading)
    {
        setError("Cannot read from a writing stream");
        return;
    }

    if (size == 0)
        return;

    SLANG_RELEASE_ASSERT(data);

    const size_t bufferSize = size_t(m_buffer.getCount());
    if (m_position > bufferSize || size > bufferSize - m_position)
    {
        // Leave `data` untouched — `size` may be enormous (a truncated stream can request a huge
        // count), so writing to `data` here would overrun the caller's buffer.
        setError("Read past end of stream");
        return;
    }

    std::memcpy(data, m_buffer.getBuffer() + m_position, size);
    m_position += size;
}

void ReplayStream::reset()
{
    m_buffer.clear();
    m_position = 0;
    m_isReading = false;
    clearError();
}

SlangResult ReplayStream::setMirrorFile(const char* path)
{
    closeMirrorFile();

    m_mirrorFile = new FileStream();
    SlangResult result =
        m_mirrorFile->init(String(path), FileMode::Create, FileAccess::Write, FileShare::ReadWrite);
    if (SLANG_FAILED(result))
    {
        m_mirrorFile = nullptr;
        return result;
    }

    if (m_buffer.getCount() > 0)
    {
        m_mirrorFile->write(m_buffer.getBuffer(), m_buffer.getCount());
        m_mirrorFile->flush();
    }
    return SLANG_OK;
}

SlangResult ReplayStream::saveToFile(const char* path) const
{
    return File::writeAllBytes(String(path), m_buffer.getBuffer(), m_buffer.getCount());
}

void ReplayStream::closeMirrorFile()
{
    if (m_mirrorFile)
    {
        m_mirrorFile->close();
        m_mirrorFile = nullptr;
    }
}

ReplayStream ReplayStream::createReader() const
{
    return ReplayStream(m_buffer.getBuffer(), m_buffer.getCount());
}

void ReplayStream::clear()
{
    m_buffer.clear();
    m_position = 0;
    m_isReading = false;
    clearError();
}

} // namespace SlangRecord
