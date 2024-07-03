#include "output-stream.h"
#include "../capture-utility.h"

namespace SlangCapture
{
    FileOutputStream::FileOutputStream(const std::string& filename, bool append)
    {
        Slang::String path(filename.c_str());
        Slang::FileMode fileMode = append ? Slang::FileMode::Append : Slang::FileMode::Create;
        Slang::FileAccess fileAccess = Slang::FileAccess::Write;
        Slang::FileShare fileShare = Slang::FileShare::None;

        SlangResult res = m_fileStream.init(path, fileMode, fileAccess, fileShare);

        if (res != SLANG_OK)
        {
            SlangCapture::slangCaptureLog(SlangCapture::LogLevel::Error, "Failed to open file %s\n", filename.c_str());
            std::abort();
        }
    }

    FileOutputStream::~FileOutputStream()
    {
        m_fileStream.close();
    }

    void FileOutputStream::write(const void* data, size_t len)
    {
        SLANG_CAPTURE_CHECK(m_fileStream.write(data, len));
    }

    MemoryStream::MemoryStream()
        : m_memoryStream(Slang::FileAccess::Write)
    { }

    void FileOutputStream::flush()
    {
        SLANG_CAPTURE_CHECK(m_fileStream.flush());
    }

    void MemoryStream::write(const void* data, size_t len)
    {
        SLANG_CAPTURE_CHECK(m_memoryStream.write(data, len));
    }

    void MemoryStream::flush()
    {
        // This call will reset the underlying buffer to size 0,
        // and reset the write position to 0.
        m_memoryStream.setContent(nullptr, 0);
    }
}
