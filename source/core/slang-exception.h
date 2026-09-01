#ifndef SLANG_CORE_EXCEPTION_H
#define SLANG_CORE_EXCEPTION_H

#include "slang-common.h"
#include "slang-string.h"

namespace Slang
{
// NOTE!
// Exceptions should not generally be used in core/compiler-core, use the 'signal' mechanism
// ideally using the macros in the slang-signal.h such as `SLANG_UNEXPECTED`
//
// If core/compiler-core libraries are compiled with SLANG_DISABLE_EXCEPTIONS,
// these classes will *never* be thrown.

// These exceptions can be thrown in one dynamic library and caught by type
// in another. Slang builds with hidden symbol visibility, so each dynamic
// library gets its own copy of the typeinfo. libc++abi (macOS) matches catch
// clauses by RTTI pointer identity, so a cross-library catch fails to match
// there; libstdc++ and MSVC match by type name and are unaffected. Default
// type visibility exports the typeinfo/vtable so the dynamic linker
// coalesces all copies into one.
//
// Gate on the platform, not the compiler: all non-Windows targets use the
// Itanium ABI; PE/COFF doesn't have the problem. Don't use SLANG_API here:
// core is a static library linked into several dynamic libraries, and
// SLANG_API is empty unless SLANG_DYNAMIC is defined.
//
// The macro stays defined on purpose: any Exception subclass that crosses a
// library boundary needs the same annotation, wherever it is declared (see
// TextFormatException in slang-token-reader.h and the replay exceptions in
// replay-context.h). Subclasses thrown and caught inside one binary don't
// need it.
#if SLANG_WINDOWS_FAMILY
#define SLANG_EXCEPTION_TYPE_VISIBLE
#else
#define SLANG_EXCEPTION_TYPE_VISIBLE __attribute__((visibility("default")))
#endif

class SLANG_EXCEPTION_TYPE_VISIBLE Exception
{
public:
    String Message;
    Exception() {}
    Exception(const String& message)
        : Message(message)
    {
    }

    virtual ~Exception() {}
};

class SLANG_EXCEPTION_TYPE_VISIBLE InvalidOperationException : public Exception
{
public:
    InvalidOperationException() {}
    InvalidOperationException(const String& message)
        : Exception(message)
    {
    }
};

class SLANG_EXCEPTION_TYPE_VISIBLE InternalError : public Exception
{
public:
    InternalError() {}
    InternalError(const String& message)
        : Exception(message)
    {
    }
};

class SLANG_EXCEPTION_TYPE_VISIBLE AbortCompilationException : public Exception
{
public:
    AbortCompilationException() {}
    AbortCompilationException(const String& message)
        : Exception(message)
    {
    }
};
} // namespace Slang

#endif
