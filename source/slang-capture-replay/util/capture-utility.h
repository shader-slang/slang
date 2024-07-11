#ifndef CAPTURE_UTILITY_H
#define CAPTURE_UTILITY_H

// in gcc and clang, __PRETTY_FUNCTION__ is the function signature,
// while MSVC uses __FUNCSIG__
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace SlangCapture
{
    enum LogLevel: unsigned int
    {
        Silent = 0,
        Error = 1,
        Debug = 2,
        Verbose = 3,
    };

    bool isCaptureLayerEnabled();
    void slangCaptureLog(LogLevel logLevel, const char* fmt, ...);
    void setLogLevel();
}

#define SLANG_CAPTURE_ASSERT(VALUE)         \
    do {                                    \
        if (!(VALUE)) {                     \
            SlangCapture::slangCaptureLog(SlangCapture::LogLevel::Error, "Assertion failed: %s, %s, %d\n", #VALUE, __FILE__, __LINE__);\
            std::abort();                   \
        }                                   \
    } while(0)

#define SLANG_CAPTURE_CHECK(VALUE)          \
    do {                                    \
        SLANG_CAPTURE_ASSERT((VALUE) == SLANG_OK);      \
    } while(0)
#endif // CAPTURE_UTILITY_H
