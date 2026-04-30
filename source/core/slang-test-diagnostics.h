// slang-test-diagnostics.h
#ifndef SLANG_TEST_DIAGNOSTICS_H
#define SLANG_TEST_DIAGNOSTICS_H

#include <stdlib.h>
#include <string.h>

namespace Slang
{

inline bool isTestDiagnosticTokenEnabled(const char* diagnostics, const char* category)
{
    if (!diagnostics || !category)
        return false;

    if (strcmp(diagnostics, "all") == 0 || strcmp(diagnostics, "1") == 0)
        return true;

    const size_t categoryLength = strlen(category);
    const char* cursor = diagnostics;
    while (*cursor)
    {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == ',')
            cursor++;

        const char* end = cursor;
        while (*end && *end != ',')
            end++;

        const char* tokenEnd = end;
        while (tokenEnd > cursor && (tokenEnd[-1] == ' ' || tokenEnd[-1] == '\t'))
            tokenEnd--;

        if (size_t(tokenEnd - cursor) == categoryLength &&
            strncmp(cursor, category, categoryLength) == 0)
        {
            return true;
        }

        cursor = end;
    }

    return false;
}

inline bool isTestDiagnosticEnabled(const char* category)
{
#ifdef _WIN32
    static const char* diagnostics = []() -> const char*
    {
        static char buffer[256];
        char* value = nullptr;
        size_t valueLength = 0;
        if (_dupenv_s(&value, &valueLength, "SLANG_TEST_DIAGNOSTICS") == 0 && value)
        {
            strncpy_s(buffer, sizeof(buffer), value, _TRUNCATE);
            free(value);
            return buffer;
        }
        return nullptr;
    }();
#else
    static const char* diagnostics = getenv("SLANG_TEST_DIAGNOSTICS");
#endif
    return isTestDiagnosticTokenEnabled(diagnostics, category);
}

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTICS_H
