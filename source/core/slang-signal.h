#ifndef SLANG_CORE_SIGNAL_H
#define SLANG_CORE_SIGNAL_H

#include "slang-common.h"

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


/// Raise an internal error of kind `type` from the Slang source location `file`:`line`.
///
/// Every internal error passes through this one function, so it is also the place that records
/// what the error was: the calling thread's signal record keeps the kind, the location and the
/// call stack until the next signal or until `clearLastSignalRecord`. `message` can be null.
/// `file` must outlive the record; the macros below pass `__FILE__`, which is a literal.
[[noreturn]] void handleSignal(SignalType type, char const* message, char const* file, int line);

// Special handler for assertions that can optionally return based on environment variable.
// Pass isReleaseAssert=true for assertions that should fire even in release-assert-only mode.
void handleAssert(char const* message, char const* file, int line, bool isReleaseAssert);

#define SLANG_UNEXPECTED(reason) \
    ::Slang::handleSignal(::Slang::SignalType::Unexpected, reason, __FILE__, __LINE__)

#define SLANG_UNIMPLEMENTED_X(what) \
    ::Slang::handleSignal(::Slang::SignalType::Unimplemented, what, __FILE__, __LINE__)

#define SLANG_UNREACHABLE(msg) \
    ::Slang::handleSignal(::Slang::SignalType::Unreachable, msg, __FILE__, __LINE__)

#define SLANG_ASSERT_FAILURE(msg) ::Slang::handleAssert(msg, __FILE__, __LINE__, false)

#define SLANG_INVALID_OPERATION(msg) \
    ::Slang::handleSignal(::Slang::SignalType::InvalidOperation, msg, __FILE__, __LINE__)

#define SLANG_ABORT_COMPILATION(msg) \
    ::Slang::handleSignal(::Slang::SignalType::AbortCompilation, msg, __FILE__, __LINE__)


const char* getLastSignalMessage();

struct StackTrace;

/// What the calling thread's most recent internal error was raised with.
struct SignalRecord
{
    SignalType type;

    /// The Slang source file and line that raised the error.
    const char* file;
    int line;

    /// The call stack at the raise. It has no frames on a platform that cannot capture one.
    /// The trace belongs to the calling thread and stays valid until that thread raises again.
    const StackTrace* stackTrace;
};

/// Get the record of the calling thread's most recent internal error.
///
/// Return false if this thread has raised none since its last `clearLastSignalRecord`. The
/// record is per thread, so an internal error on one thread never changes what another thread
/// reads here.
bool getLastSignalRecord(SignalRecord& outRecord);

/// Forget the calling thread's record. A caller that is about to run code whose failure it
/// will report clears the record first, so that an older error, which someone already handled,
/// cannot be taken for the new one.
void clearLastSignalRecord();

/// Return the fixed name that reports use for `type`: one of `unexpected`, `unimplemented`,
/// `assert`, `unreachable`, `invalid-operation` and `abort-compilation`.
///
/// These names are part of the crash-report format and do not change when the wording of a
/// message does.
const char* getSignalTypeName(SignalType type);

} // namespace Slang

#endif
