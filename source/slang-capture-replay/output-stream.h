#ifndef OUTPUT_STREAM_H
#define OUTPUT_STREAM_H

#include <string>
#include "../core/slang-stream.h"

namespace SlangCapture
{
    class OutputStream
    {
    public:
        virtual ~OutputStream() {}
        virtual void write(const void* data, size_t len) = 0;
        virtual void flush() {}
    };

    class FileOutputStream : public OutputStream
    {
    public:
        FileOutputStream(const std::string& filename, bool append = false);
        virtual ~FileOutputStream() override;
        virtual void write(const void* data, size_t len) override;
        virtual void flush() override;

    private:
        Slang::FileStream m_fileStream;
    };

    // The reason we inherit from OwnedMemoryStream instead of declaring it
    // as a member is because OwnedMemoryStream lacks some of the functionality
    // of operating on the underlying buffer directly.
    class MemoryStream : public OutputStream
    {
    public:
        MemoryStream();
        virtual ~MemoryStream() { }
        virtual void write(const void* data, size_t len) override;
        virtual void flush() override;
        const void* getData() { return m_memoryStream.getContents().getBuffer(); }
        size_t getSizeInBytes() { return m_memoryStream.getContents().getCount(); }

    private:
        Slang::OwnedMemoryStream    m_memoryStream;
    };
} // namespace SlangCapture
#endif // OUTPUT_STREAM_H
