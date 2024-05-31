#ifndef OUTPUT_STREAM_H
#define OUTPUT_STREAM_H
namespace SlangCapture
{
    class OutputStream
    {
    public:
        virtual ~OutputStream() {}

        virtual size_t Write(const void* data, size_t len) = 0;

        virtual void Flush() {}
    };
} // namespace SlangCapture
#endif // OUTPUT_STREAM_H
