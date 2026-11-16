#ifndef SLANG_CORE_EXCEPTION_H
#define SLANG_CORE_EXCEPTION_H

#include "slang-common.h"
#include "slang-signal.h"
#include "slang-string.h"

#include <setjmp.h>

// Macroizes the opening `try` of an exception-guarded block. The matching `catch` clauses must
// still be wrapped in `#if SLANG_HAS_EXCEPTIONS` directly: once the `try` is elided under
// -fno-exceptions a `catch` is ill-formed, so the whole clause is removed by the preprocessor (this
// applies even to an empty `catch (...) {}`). With exceptions disabled an internal abort
// terminates in handleSignal() instead of throwing, so a plain SLANG_EXCEPTION_TRY block
// forfeits the graceful recovery its catch clauses provide (diagnostics, fallback returns,
// language-server survival). A boundary that must recover compilation aborts in every build
// uses SLANG_EXCEPTION_TRY_RECOVERY instead: in this configuration it installs a per-thread
// AbortCompilationEscape and turns the abort into a longjmp back to the boundary, whose
// SLANG_EXCEPTION_ABORT_RECOVERY branch (written inside the `#else` of the same
// `#if SLANG_HAS_EXCEPTIONS`) replaces the `catch (const AbortCompilationException&)` arm.
#if SLANG_HAS_EXCEPTIONS
#define SLANG_EXCEPTION_TRY try
#define SLANG_EXCEPTION_TRY_RECOVERY try
#else
#define SLANG_EXCEPTION_TRY
// A recovery-enabled guarded block. `setjmp` in the controlling expression of an `if` compared
// against an integer constant is one of the standard-sanctioned positions for it. Locals of the
// enclosing function that change between this point and the longjmp are formally indeterminate
// once the jump lands, so a recovery branch may only write them. The boundary sinks
// (DiagnosticSink etc.) are a deliberate exception: they are constructed before this macro, live
// in memory (their address escapes into every callee), and are only mutated through calls in the
// guarded region, so reading and appending to them after the jump is well-defined on the
// compilers Slang targets. The macro is one guard per function: two
// SLANG_EXCEPTION_TRY_RECOVERY blocks in the same function would redeclare _slangAbortEscape
// and fail to compile.
#define SLANG_EXCEPTION_TRY_RECOVERY \
    ::Slang::AbortCompilationEscape _slangAbortEscape; \
    if (setjmp(_slangAbortEscape.jumpBuffer) == 0)
#define SLANG_EXCEPTION_ABORT_RECOVERY else
#endif

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
