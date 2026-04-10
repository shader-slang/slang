#if !defined(_WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
#ifndef SLANG_CORE_SECURE_CRT_H
#define SLANG_CORE_SECURE_CRT_H
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>

// Define MSVC specific macros
#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif
#ifndef STRUNCATE
#define STRUNCATE 80
#endif
#if !defined(_stricmp) && !defined(__MINGW32__)
#define _stricmp strcasecmp
#endif

#ifndef HAVE_MEMCPY_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/memcpy-s-wmemcpy-s
inline int memcpy_s(void* dest, size_t destSize, const void* src, size_t count)
{
    if (count == 0)
        return 0;
    if (dest == nullptr)
    {
        errno = EINVAL;
        return EINVAL;
    }
    if (src == nullptr || destSize < count)
    {
        memset(dest, 0, destSize);
        if (src == nullptr)
        {
            errno = EINVAL;
            return EINVAL;
        }
        errno = ERANGE;
        return ERANGE;
    }
    memcpy(dest, src, count);
    return 0;
}
#endif // HAVE_MEMCPY_S

#ifndef HAVE_FOPEN_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-s-wfopen-s
inline int fopen_s(FILE** f, const char* fileName, const char* mode)
{
    if (f == nullptr || fileName == nullptr || mode == nullptr)
    {
        errno = EINVAL;
        return EINVAL;
    }
    *f = fopen(fileName, mode);
    if (*f == nullptr)
    {
        return errno;
    }
    return 0;
}
#endif // HAVE_FOPEN_S

#ifndef HAVE_FREAD_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fread-s
inline size_t fread_s(
    void* buffer,
    size_t bufferSize,
    size_t elementSize,
    size_t count,
    FILE* stream)
{
    if (elementSize == 0 || count == 0)
        return 0;
    if (buffer == nullptr || stream == nullptr)
    {
        errno = EINVAL;
        return 0;
    }
    if (count > bufferSize / elementSize)
    {
        errno = ERANGE;
        return 0;
    }
    return fread(buffer, elementSize, count, stream);
}
#endif // HAVE_FREAD_S

#ifndef HAVE_WCSNLEN_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strnlen-strnlen-s
inline size_t wcsnlen_s(const wchar_t* str, size_t numberOfElements)
{
    if (str == nullptr)
        return 0;
    size_t count = 0;
    while (count < numberOfElements && str[count] != L'\0')
        count++;
    return count;
}
#endif // HAVE_WCSNLEN_S
#ifndef HAVE_STRNLEN_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strnlen-strnlen-s
inline size_t strnlen_s(const char* str, size_t numberOfElements)
{
    if (str == nullptr)
        return 0;
    size_t count = 0;
    while (count < numberOfElements && str[count] != '\0')
        count++;
    return count;
}
#endif // HAVE_STRNLEN_S

#ifndef HAVE_SPRINTF_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l
// Note: unlike MSVC's sprintf_s which invokes the invalid parameter handler
// on overflow, this fallback detects overflow via vsnprintf's return value.
// On overflow, it zeros the buffer and returns -1, matching MSVC's return
// value semantics but without the invalid parameter handler invocation.
#ifdef __GNUC__
__attribute__((format(printf, 3, 4)))
#endif
inline int
sprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, ...)
{
    if (buffer == nullptr || format == nullptr || sizeOfBuffer == 0)
    {
        errno = EINVAL;
        return -1;
    }
    va_list argptr;
    va_start(argptr, format);
    int rs = vsnprintf(buffer, sizeOfBuffer, format, argptr);
    va_end(argptr);
    if (rs < 0 || (size_t)rs >= sizeOfBuffer)
    {
        buffer[0] = '\0';
        return -1;
    }
    return rs;
}
#endif // HAVE_SPRINTF_S

#ifndef HAVE_SWPRINTF_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l

// A patch was submitted to GCC wchar_t support in 2001, so I'm sure we can
// enable this any day now...
// __attribute__((format(wprintf, 3, 4)))
inline int swprintf_s(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, ...)
{
    if (buffer == nullptr || format == nullptr || sizeOfBuffer == 0)
    {
        errno = EINVAL;
        return -1;
    }
    va_list argptr;
    va_start(argptr, format);
    int rs = vswprintf(buffer, sizeOfBuffer, format, argptr);
    va_end(argptr);
    if (rs < 0 || (size_t)rs >= sizeOfBuffer)
    {
        buffer[0] = L'\0';
        return -1;
    }
    return rs;
}
#endif // HAVE_SWPRINTF_S

#ifndef HAVE_WCSCPY_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s
inline int wcscpy_s(wchar_t* dest, size_t dest_size, const wchar_t* src)
{
    if (dest == nullptr)
    {
        errno = EINVAL;
        return EINVAL;
    }
    if (dest_size == 0)
    {
        errno = ERANGE;
        return ERANGE;
    }
    if (src == nullptr)
    {
        dest[0] = L'\0';
        errno = EINVAL;
        return EINVAL;
    }
    // Copy characters until we hit null terminator or run out of space
    size_t i = 0;
    while (i < dest_size - 1 && src[i] != L'\0')
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = L'\0';
    if (src[i] != L'\0')
    {
        dest[0] = L'\0';
        errno = ERANGE;
        return ERANGE;
    }
    return 0;
}
#endif // HAVE_WCSCPY_S
#ifndef HAVE_STRCPY_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strcpy-s-wcscpy-s-mbscpy-s
inline int strcpy_s(char* dest, size_t dest_size, const char* src)
{
    if (dest == nullptr)
    {
        errno = EINVAL;
        return EINVAL;
    }
    if (dest_size == 0)
    {
        errno = ERANGE;
        return ERANGE;
    }
    if (src == nullptr)
    {
        dest[0] = '\0';
        errno = EINVAL;
        return EINVAL;
    }
    size_t i = 0;
    while (i < dest_size - 1 && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    if (src[i] != '\0')
    {
        dest[0] = '\0';
        errno = ERANGE;
        return ERANGE;
    }
    return 0;
}
#endif // HAVE_STRCPY_S

#ifndef HAVE_WCSNCPY_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strncpy-s-strncpy-s-l-wcsncpy-s-wcsncpy-s-l-mbsncpy-s-mbsncpy-s-l?view=msvc-170
inline int wcsncpy_s(
    wchar_t* strDest,
    size_t numberOfElements,
    const wchar_t* strSource,
    size_t count)
{
    if (strDest == nullptr || numberOfElements == 0)
    {
        errno = EINVAL;
        return EINVAL;
    }
    if (strSource == nullptr)
    {
        strDest[0] = L'\0';
        errno = EINVAL;
        return EINVAL;
    }
    if (count == 0)
    {
        strDest[0] = L'\0';
        return 0;
    }
    size_t limit = (count == _TRUNCATE)
                       ? numberOfElements - 1
                       : (count < numberOfElements - 1 ? count : numberOfElements - 1);
    size_t i = 0;
    while (i < limit && strSource[i] != L'\0')
    {
        strDest[i] = strSource[i];
        i++;
    }
    strDest[i] = L'\0';
    if (strSource[i] != L'\0')
    {
        if (count == _TRUNCATE)
            return STRUNCATE;
        if (count >= numberOfElements)
        {
            strDest[0] = L'\0';
            errno = ERANGE;
            return ERANGE;
        }
    }
    return 0;
}
#endif // HAVE_WCSNCPY_S
#ifndef HAVE_STRNCPY_S
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/strncpy-s-strncpy-s-l-wcsncpy-s-wcsncpy-s-l-mbsncpy-s-mbsncpy-s-l?view=msvc-170
inline int strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count)
{
    if (strDest == nullptr || numberOfElements == 0)
    {
        errno = EINVAL;
        return EINVAL;
    }
    if (strSource == nullptr)
    {
        strDest[0] = '\0';
        errno = EINVAL;
        return EINVAL;
    }
    if (count == 0)
    {
        strDest[0] = '\0';
        return 0;
    }
    size_t limit = (count == _TRUNCATE)
                       ? numberOfElements - 1
                       : (count < numberOfElements - 1 ? count : numberOfElements - 1);
    size_t i = 0;
    while (i < limit && strSource[i] != '\0')
    {
        strDest[i] = strSource[i];
        i++;
    }
    strDest[i] = '\0';
    if (strSource[i] != '\0')
    {
        if (count == _TRUNCATE)
            return STRUNCATE;
        if (count >= numberOfElements)
        {
            strDest[0] = '\0';
            errno = ERANGE;
            return ERANGE;
        }
    }
    return 0;
}
#endif // HAVE_STRNCPY_S
#endif // SLANG_CORE_SECURE_CRT_H
#endif // !defined(_WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
