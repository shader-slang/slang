#include "slang-record-replay/replay-stream.h"

namespace SlangRecord
{

ReplayStream::ReplayStream() = default;

ReplayStream::ReplayStream(const void* data, size_t size)
    : m_isReading(true)
{
    if (data && size > 0)
    {
        m_buffer.setCount(size);
        std::memcpy(m_buffer.getBuffer(), data, size);
    }
}

ReplayStream ReplayStream::loadFromFile(const char* path)
{
    List<unsigned char> contents;
    SlangResult result = File::readAllBytes(String(path), contents);
    if (SLANG_FAILED(result))
        throw Slang::Exception(String("Failed to open file for reading: ") + path);

    ReplayStream stream;
    stream.m_isReading = true;
    stream.m_buffer = Slang::_Move(contents);

    return stream;
}

ReplayStream::ReplayStream(ReplayStream&&) = default;

ReplayStream& ReplayStream::operator=(ReplayStream&&) = default;

ReplayStream::~ReplayStream()
{
    closeMirrorFile();
}

void ReplayStream::write(const void* data, size_t size)
{
    if (m_isReading)
        throw Slang::Exception("Cannot write to a reading stream");

    size_t newSize = m_position + size;
    if (newSize > size_t(m_buffer.getCapacity()))
    {
        m_buffer.reserve(Slang::Index(newSize) * 2);
    }
    if (newSize > size_t(m_buffer.getCount()))
    {
        m_buffer.setCount(Slang::Index(newSize));
    }

    std::memcpy(m_buffer.getBuffer() + m_position, data, size);
    m_position += size;

    if (m_mirrorFile)
    {
        m_mirrorFile->write(data, size);
        m_mirrorFile->flush();
    }
}

void ReplayStream::read(void* data, size_t size)
{
    if (!m_isReading)
        throw Slang::Exception("Cannot read from a writing stream");

    if (m_position + size > size_t(m_buffer.getCount()))
        throw Slang::Exception("Read past end of stream");

    std::memcpy(data, m_buffer.getBuffer() + m_position, size);
    m_position += size;
}

void ReplayStream::reset()
{
    m_buffer.clear();
    m_position = 0;
    m_isReading = false;
}

void ReplayStream::setMirrorFile(const char* path)
{
    closeMirrorFile();

    m_mirrorFile = new FileStream();
    SlangResult result = m_mirrorFile->init(
        String(path),
        FileMode::Create,
        FileAccess::Write,
        FileShare::ReadWrite);
    if (SLANG_FAILED(result))
    {
        m_mirrorFile = nullptr;
        throw Slang::Exception(String("Failed to open mirror file: ") + path);
    }

    if (m_buffer.getCount() > 0)
    {
        m_mirrorFile->write(m_buffer.getBuffer(), m_buffer.getCount());
        m_mirrorFile->flush();
    }
}

void ReplayStream::saveToFile(const char* path) const
{
    SlangResult result =
        File::writeAllBytes(String(path), m_buffer.getBuffer(), m_buffer.getCount());
    if (SLANG_FAILED(result))
        throw Slang::Exception(String("Failed to write to file: ") + path);
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
}

} // namespace SlangRecord
