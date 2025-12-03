// slang-test-diagnostics.h
// Diagnostic control for slang-test debugging
// Enabled via SLANG_TEST_DIAGNOSTICS environment variable

#ifndef SLANG_TEST_DIAGNOSTICS_H
#define SLANG_TEST_DIAGNOSTICS_H

#include <cstdlib>
#include <cstring>

namespace Slang
{

// Check if a diagnostic category is enabled via SLANG_TEST_DIAGNOSTICS env var
// Categories: timing, timing-phases, rpc, fd, pipe, all
inline bool isDiagnosticEnabled(const char* category)
{
#ifdef _WIN32
    static const char* diagnostics = []() -> const char*
    {
        static char buffer[256];
        char* buf = nullptr;
        size_t len = 0;
        if (_dupenv_s(&buf, &len, "SLANG_TEST_DIAGNOSTICS") == 0 && buf != nullptr)
        {
            strncpy_s(buffer, sizeof(buffer), buf, _TRUNCATE);
            free(buf);
            return buffer;
        }
        return nullptr;
    }();
#else
    static const char* diagnostics = getenv("SLANG_TEST_DIAGNOSTICS");
#endif
    if (!diagnostics)
        return false;
    if (strcmp(diagnostics, "all") == 0 || strcmp(diagnostics, "1") == 0)
        return true;
    return strstr(diagnostics, category) != nullptr;
}

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTICS_H
