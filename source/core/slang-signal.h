#ifndef SLANG_CORE_SIGNAL_H
#define SLANG_CORE_SIGNAL_H

#include "slang-common.h"

#include <setjmp.h>

namespace Slang
{

enum class SignalType
{
    Unexpected,
    Unimplemented,
    AssertFailure,
    Unreachable,
    InvalidOperation,
    AbortCompilation,
};


// Note that message can be passed as nullptr for no message.
[[noreturn]] void handleSignal(SignalType type, char const* message);

#if !SLANG_HAS_EXCEPTIONS
// In a build with exceptions disabled `handleSignal` cannot throw, so a compilation abort has
// no built-in way to reach the recovery code at an API boundary: the process would terminate
// where an exceptions-enabled build would unwind to a `catch`. A boundary that must recover
// compilation aborts in every build installs one of these guards (through the
// SLANG_EXCEPTION_TRY_RECOVERY macro); `handleSignal` then jumps back to `jumpBuffer` for
// SignalType::AbortCompilation instead of trapping, and the boundary's recovery branch reports
// failure diagnostics and returns. Guards form a per-thread stack (see slang-signal.cpp) so
// nested boundaries each unwind to their own frame.
struct AbortCompilationEscape
{
    AbortCompilationEscape();
    ~AbortCompilationEscape();

    jmp_buf jumpBuffer;
    bool active = false;
};
#endif

// Special handler for assertions that can optionally return based on environment variable.
// Pass isReleaseAssert=true for assertions that should fire even in release-assert-only mode.
void handleAssert(char const* message, char const* file, int line, bool isReleaseAssert);

#define SLANG_UNEXPECTED(reason) ::Slang::handleSignal(::Slang::SignalType::Unexpected, reason)

#define SLANG_UNIMPLEMENTED_X(what) ::Slang::handleSignal(::Slang::SignalType::Unimplemented, what)

#define SLANG_UNREACHABLE(msg) ::Slang::handleSignal(::Slang::SignalType::Unreachable, msg)

#define SLANG_ASSERT_FAILURE(msg) ::Slang::handleAssert(msg, __FILE__, __LINE__, false)

#define SLANG_INVALID_OPERATION(msg) \
    ::Slang::handleSignal(::Slang::SignalType::InvalidOperation, msg)

#define SLANG_ABORT_COMPILATION(msg) \
    ::Slang::handleSignal(::Slang::SignalType::AbortCompilation, msg)


const char* getLastSignalMessage();

} // namespace Slang

#endif
