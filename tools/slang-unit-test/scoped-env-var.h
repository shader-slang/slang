#pragma once

#include "core/slang-string.h"

#include <stdlib.h>

namespace SlangUnitTest
{

inline int writeEnvironmentVariable(const char* key, const char* val)
{
#ifdef _WIN32
    return _putenv_s(key, val);
#else
    return setenv(key, val, 1);
#endif
}

inline int unsetEnvironmentVariable(const char* key)
{
#ifdef _WIN32
    return _putenv_s(key, "");
#else
    return unsetenv(key);
#endif
}

struct ScopedEnvVar
{
    const char* key;
    bool hadOldValue = false;
    Slang::String oldValue;

    ScopedEnvVar(const char* inKey, const char* inVal)
        : key(inKey)
    {
#ifdef _WIN32
        char* value = nullptr;
        size_t valueLength = 0;
        const errno_t result = _dupenv_s(&value, &valueLength, key);
        SLANG_RELEASE_ASSERT(result == 0);
        if (value)
        {
            hadOldValue = true;
            oldValue = value;
            free(value);
        }
#else
        if (const char* value = getenv(key))
        {
            hadOldValue = true;
            oldValue = value;
        }
#endif
        writeEnvironmentVariable(key, inVal);
    }

    ScopedEnvVar(const ScopedEnvVar&) = delete;
    ScopedEnvVar& operator=(const ScopedEnvVar&) = delete;

    ~ScopedEnvVar()
    {
        if (hadOldValue)
        {
            writeEnvironmentVariable(key, oldValue.getBuffer());
        }
        else
        {
            unsetEnvironmentVariable(key);
        }
    }
};

} // namespace SlangUnitTest
