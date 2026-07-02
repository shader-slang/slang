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

// These exception types can be thrown in one dynamic library and caught by
// type in another. For example, the `SLANG_RELEASE_ASSERT` signal path inside
// the slang shared library throws `InternalError`, and a test in the
// separately loaded slang-unit-test-tool module catches it by type. For such
// a typed catch to match, the exception type's RTTI must be one canonical
// definition per process. Slang builds every target with hidden symbol
// visibility, which would give each dynamic library its own private copy of
// the typeinfo; libc++abi on Apple platforms matches a catch clause against a
// thrown exception by RTTI identity, so a cross-library typed catch silently
// fails to match there (libstdc++ and MSVC match by type name and are
// unaffected). Marking the classes with default type visibility exports the
// typeinfo and vtable so the dynamic linker coalesces every image's copy into
// one canonical definition. MSVC needs no annotation, and `__declspec` export
// semantics do not apply to this problem on Windows.
#ifdef _MSC_VER
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
