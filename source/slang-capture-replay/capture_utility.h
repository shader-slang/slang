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
#endif // CAPTURE_UTILITY_H
