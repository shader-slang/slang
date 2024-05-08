#ifndef CAPTURE_UTILITY_H
#define CAPTURE_UTILITY_H

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
