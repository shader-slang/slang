// unit-test-secure-crt.cpp
// Tests for the secure CRT fallback functions in source/core/slang-secure-crt.h.
// These inline functions are only compiled on non-Windows platforms (Linux/macOS).

#if !defined(_WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)

#include "../../source/core/slang-secure-crt.h"
#include "unit-test/slang-unit-test.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#if defined(__MINGW32__) && !defined(__CYGWIN__)
static const char* const kNullDevice = "NUL";
#else
static const char* const kNullDevice = "/dev/null";
#endif

SLANG_UNIT_TEST(secureCrtMemcpyS)
{
    // Happy path
    {
        char dst[16] = {};
        SLANG_CHECK(memcpy_s(dst, sizeof(dst), "hello", 5) == 0);
        SLANG_CHECK(memcmp(dst, "hello", 5) == 0);
    }
    // count == 0 returns success without touching dest
    {
        SLANG_CHECK(memcpy_s(nullptr, 0, nullptr, 0) == 0);
    }
    // null dest
    {
        errno = 0;
        SLANG_CHECK(memcpy_s(nullptr, 16, "hello", 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // null src — dest must be zeroed
    {
        char dst[8];
        memset(dst, 0xAA, sizeof(dst));
        errno = 0;
        SLANG_CHECK(memcpy_s(dst, sizeof(dst), nullptr, 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
        // buffer should be zeroed
        char zeros[8] = {};
        SLANG_CHECK(memcmp(dst, zeros, sizeof(dst)) == 0);
    }
    // destSize < count — dest must be zeroed, returns ERANGE
    {
        char dst[4];
        memset(dst, 0xAA, sizeof(dst));
        errno = 0;
        SLANG_CHECK(memcpy_s(dst, sizeof(dst), "toolong!", 8) == ERANGE);
        SLANG_CHECK(errno == ERANGE);
        char zeros[4] = {};
        SLANG_CHECK(memcmp(dst, zeros, sizeof(dst)) == 0);
    }
}

SLANG_UNIT_TEST(secureCrtFopenS)
{
    // Happy path — open a temp file
    {
        FILE* f = nullptr;
        SLANG_CHECK(fopen_s(&f, kNullDevice, "r") == 0);
        SLANG_CHECK(f != nullptr);
        if (f)
            fclose(f);
    }
    // null file pointer
    {
        errno = 0;
        SLANG_CHECK(fopen_s(nullptr, kNullDevice, "r") == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // null filename
    {
        FILE* f = nullptr;
        errno = 0;
        SLANG_CHECK(fopen_s(&f, nullptr, "r") == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // null mode
    {
        FILE* f = nullptr;
        errno = 0;
        SLANG_CHECK(fopen_s(&f, kNullDevice, nullptr) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // nonexistent file
    {
        FILE* f = nullptr;
        int result = fopen_s(&f, "/nonexistent/path/file.txt", "r");
        SLANG_CHECK(result != 0);
        SLANG_CHECK(f == nullptr);
    }
}

SLANG_UNIT_TEST(secureCrtFreadS)
{
    // Happy path — use tmpfile() for portability (no /dev/zero on MinGW)
    {
        FILE* f = tmpfile();
        SLANG_CHECK(f != nullptr);
        if (f)
        {
            const char data[4] = {};
            SLANG_CHECK(fwrite(data, 1, sizeof(data), f) == sizeof(data));
            rewind(f);

            char buf[8] = {};
            size_t n = fread_s(buf, sizeof(buf), 1, 4, f);
            SLANG_CHECK(n == 4);
            fclose(f);
        }
    }
    // elementSize == 0
    {
        char buf[4];
        SLANG_CHECK(fread_s(buf, sizeof(buf), 0, 10, stdin) == 0);
    }
    // count == 0
    {
        char buf[4];
        SLANG_CHECK(fread_s(buf, sizeof(buf), 1, 0, stdin) == 0);
    }
    // null buffer
    {
        errno = 0;
        SLANG_CHECK(fread_s(nullptr, 10, 1, 10, stdin) == 0);
        SLANG_CHECK(errno == EINVAL);
    }
    // null stream
    {
        char buf[4];
        errno = 0;
        SLANG_CHECK(fread_s(buf, sizeof(buf), 1, 4, nullptr) == 0);
        SLANG_CHECK(errno == EINVAL);
    }
    // count exceeds buffer capacity
    {
        char buf[4];
        errno = 0;
        SLANG_CHECK(fread_s(buf, sizeof(buf), 1, 100, stdin) == 0);
        SLANG_CHECK(errno == ERANGE);
    }
}

SLANG_UNIT_TEST(secureCrtStrnlenS)
{
    // Happy path
    {
        SLANG_CHECK(strnlen_s("hello", 10) == 5);
    }
    // String longer than limit
    {
        SLANG_CHECK(strnlen_s("hello", 3) == 3);
    }
    // Exact length
    {
        SLANG_CHECK(strnlen_s("hi", 2) == 2);
    }
    // null pointer
    {
        SLANG_CHECK(strnlen_s(nullptr, 10) == 0);
    }
    // Empty string
    {
        SLANG_CHECK(strnlen_s("", 10) == 0);
    }
}

SLANG_UNIT_TEST(secureCrtWcsnlenS)
{
    // Happy path
    {
        SLANG_CHECK(wcsnlen_s(L"hello", 10) == 5);
    }
    // String longer than limit
    {
        SLANG_CHECK(wcsnlen_s(L"hello", 3) == 3);
    }
    // null pointer
    {
        SLANG_CHECK(wcsnlen_s(nullptr, 10) == 0);
    }
    // Empty string
    {
        SLANG_CHECK(wcsnlen_s(L"", 10) == 0);
    }
}

SLANG_UNIT_TEST(secureCrtSprintfS)
{
    // Happy path
    {
        char buf[32];
        int n = sprintf_s(buf, sizeof(buf), "%s %d", "test", 42);
        SLANG_CHECK(n == 7);
        SLANG_CHECK(strcmp(buf, "test 42") == 0);
    }
    // null buffer
    {
        errno = 0;
        SLANG_CHECK(sprintf_s(nullptr, 10, "%s", "x") == -1);
        SLANG_CHECK(errno == EINVAL);
    }
    // null format
    {
        char buf[16];
        errno = 0;
        SLANG_CHECK(sprintf_s(buf, sizeof(buf), nullptr) == -1);
        SLANG_CHECK(errno == EINVAL);
    }
    // sizeOfBuffer == 0
    {
        char buf[4];
        errno = 0;
        SLANG_CHECK(sprintf_s(buf, 0, "%s", "x") == -1);
        SLANG_CHECK(errno == EINVAL);
    }
    // Overflow — output exceeds buffer
    {
        char buf[4];
        int n = sprintf_s(buf, sizeof(buf), "%s", "toolong");
        SLANG_CHECK(n == -1);
        SLANG_CHECK(buf[0] == '\0'); // buffer zeroed on overflow
    }
    // Exact fit (needs room for null terminator)
    {
        char buf[4];
        int n = sprintf_s(buf, sizeof(buf), "%s", "abc");
        SLANG_CHECK(n == 3);
        SLANG_CHECK(strcmp(buf, "abc") == 0);
    }
}

SLANG_UNIT_TEST(secureCrtSwprintfS)
{
    // Happy path
    {
        wchar_t buf[32];
        int n = swprintf_s(buf, 32, L"%ls %d", L"test", 42);
        SLANG_CHECK(n == 7);
        SLANG_CHECK(wcscmp(buf, L"test 42") == 0);
    }
    // null buffer
    {
        errno = 0;
        SLANG_CHECK(swprintf_s(nullptr, 10, L"%ls", L"x") == -1);
        SLANG_CHECK(errno == EINVAL);
    }
    // null format
    {
        wchar_t buf[16];
        errno = 0;
        SLANG_CHECK(swprintf_s(buf, 16, nullptr) == -1);
        SLANG_CHECK(errno == EINVAL);
    }
    // sizeOfBuffer == 0
    {
        wchar_t buf[4];
        errno = 0;
        SLANG_CHECK(swprintf_s(buf, 0, L"%ls", L"x") == -1);
        SLANG_CHECK(errno == EINVAL);
    }
    // Overflow
    {
        wchar_t buf[4];
        int n = swprintf_s(buf, 4, L"%ls", L"toolong");
        SLANG_CHECK(n == -1);
        SLANG_CHECK(buf[0] == L'\0');
    }
}

SLANG_UNIT_TEST(secureCrtStrcpyS)
{
    // Happy path
    {
        char dst[16];
        SLANG_CHECK(strcpy_s(dst, sizeof(dst), "hello") == 0);
        SLANG_CHECK(strcmp(dst, "hello") == 0);
    }
    // null dest
    {
        errno = 0;
        SLANG_CHECK(strcpy_s(nullptr, 16, "hello") == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // dest_size == 0
    {
        char dst[4];
        errno = 0;
        SLANG_CHECK(strcpy_s(dst, 0, "hello") == ERANGE);
        SLANG_CHECK(errno == ERANGE);
    }
    // null src — dest[0] should be zeroed
    {
        char dst[8];
        dst[0] = 'x';
        errno = 0;
        SLANG_CHECK(strcpy_s(dst, sizeof(dst), nullptr) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
        SLANG_CHECK(dst[0] == '\0');
    }
    // Overflow — src longer than dest
    {
        char dst[4];
        errno = 0;
        SLANG_CHECK(strcpy_s(dst, sizeof(dst), "toolong") == ERANGE);
        SLANG_CHECK(errno == ERANGE);
        SLANG_CHECK(dst[0] == '\0'); // dest[0] zeroed on overflow
    }
    // Exact fit
    {
        char dst[4];
        SLANG_CHECK(strcpy_s(dst, sizeof(dst), "abc") == 0);
        SLANG_CHECK(strcmp(dst, "abc") == 0);
    }
}

SLANG_UNIT_TEST(secureCrtWcscpyS)
{
    // Happy path
    {
        wchar_t dst[16];
        SLANG_CHECK(wcscpy_s(dst, 16, L"hello") == 0);
        SLANG_CHECK(wcscmp(dst, L"hello") == 0);
    }
    // null dest
    {
        errno = 0;
        SLANG_CHECK(wcscpy_s(nullptr, 16, L"hello") == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // dest_size == 0
    {
        wchar_t dst[4];
        errno = 0;
        SLANG_CHECK(wcscpy_s(dst, 0, L"hello") == ERANGE);
        SLANG_CHECK(errno == ERANGE);
    }
    // null src
    {
        wchar_t dst[8];
        dst[0] = L'x';
        errno = 0;
        SLANG_CHECK(wcscpy_s(dst, 8, nullptr) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
        SLANG_CHECK(dst[0] == L'\0');
    }
    // Overflow
    {
        wchar_t dst[4];
        errno = 0;
        SLANG_CHECK(wcscpy_s(dst, 4, L"toolong") == ERANGE);
        SLANG_CHECK(errno == ERANGE);
        SLANG_CHECK(dst[0] == L'\0');
    }
}

SLANG_UNIT_TEST(secureCrtStrncpyS)
{
    // Happy path — copy with count
    {
        char dst[16];
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), "hello", 5) == 0);
        SLANG_CHECK(strcmp(dst, "hello") == 0);
    }
    // null dest
    {
        errno = 0;
        SLANG_CHECK(strncpy_s(nullptr, 16, "hello", 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // numberOfElements == 0
    {
        char dst[4];
        errno = 0;
        SLANG_CHECK(strncpy_s(dst, 0, "hello", 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // null src
    {
        char dst[8];
        dst[0] = 'x';
        errno = 0;
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), nullptr, 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
        SLANG_CHECK(dst[0] == '\0');
    }
    // count == 0 — dest should be empty string
    {
        char dst[8];
        dst[0] = 'x';
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), "hello", 0) == 0);
        SLANG_CHECK(dst[0] == '\0');
    }
    // Truncation with _TRUNCATE
    {
        char dst[4];
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), "hello", _TRUNCATE) == STRUNCATE);
        SLANG_CHECK(strcmp(dst, "hel") == 0);
    }
    // _TRUNCATE with string that fits
    {
        char dst[16];
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), "hi", _TRUNCATE) == 0);
        SLANG_CHECK(strcmp(dst, "hi") == 0);
    }
    // Overflow — count >= numberOfElements and src too long
    {
        char dst[4];
        errno = 0;
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), "toolong", 7) == ERANGE);
        SLANG_CHECK(errno == ERANGE);
        SLANG_CHECK(dst[0] == '\0');
    }
    // count < numberOfElements, src fits within count
    {
        char dst[16];
        SLANG_CHECK(strncpy_s(dst, sizeof(dst), "hello", 3) == 0);
        SLANG_CHECK(strcmp(dst, "hel") == 0);
    }
}

SLANG_UNIT_TEST(secureCrtWcsncpyS)
{
    // Happy path
    {
        wchar_t dst[16];
        SLANG_CHECK(wcsncpy_s(dst, 16, L"hello", 5) == 0);
        SLANG_CHECK(wcscmp(dst, L"hello") == 0);
    }
    // null dest
    {
        errno = 0;
        SLANG_CHECK(wcsncpy_s(nullptr, 16, L"hello", 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // numberOfElements == 0
    {
        wchar_t dst[4];
        errno = 0;
        SLANG_CHECK(wcsncpy_s(dst, 0, L"hello", 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
    }
    // null src
    {
        wchar_t dst[8];
        dst[0] = L'x';
        errno = 0;
        SLANG_CHECK(wcsncpy_s(dst, 8, nullptr, 5) == EINVAL);
        SLANG_CHECK(errno == EINVAL);
        SLANG_CHECK(dst[0] == L'\0');
    }
    // count == 0
    {
        wchar_t dst[8];
        dst[0] = L'x';
        SLANG_CHECK(wcsncpy_s(dst, 8, L"hello", 0) == 0);
        SLANG_CHECK(dst[0] == L'\0');
    }
    // Truncation with _TRUNCATE
    {
        wchar_t dst[4];
        SLANG_CHECK(wcsncpy_s(dst, 4, L"hello", _TRUNCATE) == STRUNCATE);
        SLANG_CHECK(wcscmp(dst, L"hel") == 0);
    }
    // _TRUNCATE with string that fits
    {
        wchar_t dst[16];
        SLANG_CHECK(wcsncpy_s(dst, 16, L"hi", _TRUNCATE) == 0);
        SLANG_CHECK(wcscmp(dst, L"hi") == 0);
    }
    // Overflow — count >= numberOfElements and src too long
    {
        wchar_t dst[4];
        errno = 0;
        SLANG_CHECK(wcsncpy_s(dst, 4, L"toolong", 7) == ERANGE);
        SLANG_CHECK(errno == ERANGE);
        SLANG_CHECK(dst[0] == L'\0');
    }
}

#else
// On Windows, the CRT provides these functions natively — nothing to test.
#include "unit-test/slang-unit-test.h"
SLANG_UNIT_TEST(secureCrtWindowsStub)
{
    // Placeholder so the test binary has at least one test on Windows.
    SLANG_CHECK(true);
}
#endif
