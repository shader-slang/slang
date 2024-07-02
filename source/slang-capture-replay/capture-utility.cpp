
#include <cstring>
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <mutex>

#include "capture-utility.h"

constexpr const char* kCaptureLayerEnvVar = "SLANG_CAPTURE_LAYER";
constexpr const char* kCaptureLayerLogLevel = "SLANG_CAPTURE_LOG_LEVEL";

namespace SlangCapture
{
    static thread_local unsigned int g_logLevel = LogLevel::Silent;

    static bool getEnvironmentVariable(const char* name, std::string& out)
    {
#ifdef _WIN32
        char* envVar = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&envVar, &sz, name) == 0 && envVar != nullptr)
        {
            out = envVar;
            free(envVar);
        }
#else
        if (const char* envVar = std::getenv(name))
        {
            out = envVar;
        }
#endif
        return out.empty() == false;
    }

    bool isCaptureLayerEnabled()
    {
        std::string envVarStr;
        if(getEnvironmentVariable(kCaptureLayerEnvVar, envVarStr))
        {
            if (envVarStr == "1")
            {
                return true;
            }
        }
        return false;
    }

    void setLogLevel()
    {
        // We only want to set the log level once
        if (g_logLevel != LogLevel::Silent)
        {
            return;
        }

        std::string envVarStr;
        if (getEnvironmentVariable(kCaptureLayerLogLevel, envVarStr))
        {
            char* end = nullptr;
            unsigned int logLevel = std::strtol(envVarStr.c_str(), &end, 10);
            if (end && (*end == 0))
            {
                g_logLevel = std::min((unsigned int)(LogLevel::Verbose), logLevel);
                return;
            }
        }
    }

    void slangCaptureLog(LogLevel logLevel, const char* fmt, ...)
    {
        if (logLevel > g_logLevel)
        {
            return;
        }

        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
    }
}
