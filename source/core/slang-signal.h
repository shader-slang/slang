#ifndef SLANG_CORE_SIGNAL_H
#define SLANG_CORE_SIGNAL_H

#include "slang-common.h"

// SLANG_HAS_EXCEPTIONS gates this header's conditional content, and slang-common.h includes
// this header before it includes slang.h (which owns the definition) — so include it here
// directly: without it a TU reaching this header first sees the macro undefined and takes the
// wrong configuration branch. The definition stays in slang.h because SLANG_DISABLE_EXCEPTIONS
// is an embedder's build flag documented on the public surface; core re-deriving it in a
// separate config header that both include is a possible cleanup, but it changes the include
// structure of the public header and is left as a maintainer decision.
#include "slang.h"

#include <mutex>

#if !SLANG_HAS_EXCEPTIONS
#include <setjmp.h>
#endif

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
// from compilation aborts in every build installs one of these guards (through the
// SLANG_EXCEPTION_TRY_RECOVERY macro); `handleSignal` then jumps back to `jumpBuffer` for
// SignalType::AbortCompilation instead of trapping, and the boundary's recovery branch reports
// failure diagnostics and returns. Guards form a per-thread stack (see slang-signal.cpp), bounded
// at 64 entries — far beyond any realistic API-boundary nesting — and a boundary that finds the
// stack full installs no guard, so an abort inside it is recovered by the innermost boundary
// already installed rather than its own recovery branch.
struct AbortCompilationEscape
{
    AbortCompilationEscape();
    ~AbortCompilationEscape();

    jmp_buf jumpBuffer;
    // True while this guard sits on the thread's escape stack. `handleSignal` clears it right
    // before consuming the guard with longjmp, so the destructor does not pop an entry the jump
    // already removed.
    bool active = false;
    // The abort-recovery lock-stack depth when this escape was installed. `handleSignal` unlocks
    // only the locks registered past this depth: locks held by frames above this boundary belong
    // to frames the longjmp does not cross, and must stay held.
    int lockStackDepthAtInstall = 0;
};

// A lock guard for a recursive mutex that survives the AbortCompilation longjmp: in a build
// without exceptions, an abort raised while this lock is held must release it before the jump,
// exactly as exception unwinding would. Convert a `std::lock_guard` on a mutex to this guard iff
// the lock can be held by a frame the recovery jump crosses — a frame strictly below a
// SLANG_EXCEPTION_TRY_RECOVERY boundary when the abort fires. A lock in the boundary's own frame
// does not qualify (the recovery return exits its scope normally, releasing it), and neither does
// a lock whose region cannot raise AbortCompilation.
class AbortRecoveryLock
{
public:
    AbortRecoveryLock(std::recursive_mutex& mutex);
    ~AbortRecoveryLock();

    // The push/pop protocol state, consumed by handleSignal()'s escape path (see
    // slang-signal.cpp).
    enum class State
    {
        // On the thread's lock stack: the destructor pops the entry and unlocks.
        Registered,
        // Released and unregistered by handleSignal for a jump that crossed this frame. The
        // frame was abandoned, so this destructor cannot run; it asserts that invariant rather
        // than double-unlocking.
        Consumed,
        // The lock stack was full: the mutex is held but unregistered, so the destructor must
        // still unlock.
        NeverRegistered,
    };
    std::recursive_mutex* mutex;
    State state = State::NeverRegistered;
};

// Propagates the in-flight AbortCompilation outward after a recovery branch has done its local
// cleanup — the counterpart of `catch (...) { cleanup; throw; }`. The message recorded for the
// signal is preserved, so the boundary that ultimately recovers reports the original abort. With
// no enclosing recovery boundary, the abort terminates like an unguarded one.
[[noreturn]] void reraiseAbortCompilation();
#else
// With exceptions enabled, unwinding releases locks on the way out, so the plain lock_guard
// already has the behavior the no-exceptions build needs AbortRecoveryLock for.
using AbortRecoveryLock = std::lock_guard<std::recursive_mutex>;
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
