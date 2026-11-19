#include "slang-signal.h"

#include "slang-exception.h"
#include "slang-test-tool-util.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if _WIN32 && defined(_MSC_VER)
#include <cassert>
#include <windows.h>
#endif

namespace Slang
{

thread_local String g_lastSignalMessage;

#if !SLANG_HAS_EXCEPTIONS
namespace {
// Per-thread stacks for the abort-recovery machinery. A fixed depth keeps installation
// allocation-free. The escape stack depth is driven by two things: API boundaries the embedder
// calls re-entrantly (compile → link → loadModule, a handful), and one function-scope guard per
// module of an in-flight import chain — loadParsedModule and loadSerializedModule are recovery
// boundaries, so an import chain of N modules keeps N entries alive during checking. 64 covers
// import chains far deeper than real shader libraries. The stack-full fallback lives in the
// AbortCompilationEscape header comment, which is the contract for both stacks' bound. One
// deliberate asymmetry: a full escape stack degrades gracefully (the abort recovers one
// boundary further out), while a full lock stack strands the mutex — see the
// AbortRecoveryLock constructor.
constexpr int kMaxAbortCompilationEscapeDepth = 64;
thread_local AbortCompilationEscape* g_abortCompilationEscapes[kMaxAbortCompilationEscapeDepth];
thread_local int g_abortCompilationEscapeCount = 0;
thread_local AbortRecoveryLock* g_abortRecoveryLocks[kMaxAbortCompilationEscapeDepth];
thread_local int g_abortRecoveryLockCount = 0;
} // namespace

AbortCompilationEscape::AbortCompilationEscape()
{
    if (g_abortCompilationEscapeCount < kMaxAbortCompilationEscapeDepth)
    {
        active = true;
        lockStackDepthAtInstall = g_abortRecoveryLockCount;
        g_abortCompilationEscapes[g_abortCompilationEscapeCount++] = this;
    }
}

AbortCompilationEscape::~AbortCompilationEscape()
{
    if (active)
    {
        // An active guard is always the top of the stack: `handleSignal` consumes the innermost
        // escape before jumping and marks it inactive, and guards otherwise destroy in reverse
        // construction order.
        SLANG_ASSERT(g_abortCompilationEscapes[g_abortCompilationEscapeCount - 1] == this);
        g_abortCompilationEscapes[--g_abortCompilationEscapeCount] = nullptr;
    }
}

AbortRecoveryLock::AbortRecoveryLock(std::recursive_mutex& mutex)
    : mutex(&mutex)
{
    mutex.lock();
    if (g_abortRecoveryLockCount < kMaxAbortCompilationEscapeDepth)
    {
        state = State::Registered;
        g_abortRecoveryLocks[g_abortRecoveryLockCount++] = this;
    }
    // A full lock stack leaves the lock held but unregistered (NeverRegistered): an abort under
    // it would strand the mutex. Reaching depth 64 of *nested* recovery locks on one thread
    // requires far more re-entrancy than the API allows, so this registers as unreachable in
    // practice.
}

AbortRecoveryLock::~AbortRecoveryLock()
{
    switch (state)
    {
    case State::Registered:
        SLANG_ASSERT(g_abortRecoveryLocks[g_abortRecoveryLockCount - 1] == this);
        g_abortRecoveryLocks[--g_abortRecoveryLockCount] = nullptr;
        mutex->unlock();
        break;
    case State::NeverRegistered:
        mutex->unlock();
        break;
    case State::Consumed:
        // handleSignal consumed this guard for a jump that abandoned this frame — and with
        // exceptions enabled, MSVC's /EHsc longjmp unwinds the crossed frames' destructors, so
        // this destructor legitimately runs there, after the manual unlock. Doing nothing is the
        // correct completion of the consume; unlocking again would corrupt the mutex.
        break;
    }
}

// Pops the innermost installed escape and jumps to it, releasing the recovery locks the jump
// crosses, mirroring what exception unwinding would have done; locks registered before that
// escape was installed belong to frames the jump does not cross and stay held. On success this
// longjmps and does not return to this frame; the bool return is false only when no escape is
// installed, which the caller turns into the trap below (handleSignal) or the unguarded abort's
// termination (reraiseAbortCompilation). The message recorded for the in-flight signal is left
// untouched, so a reraise reports the original abort text. No allocation: the signal path may
// run with a corrupted heap.
static bool tryJumpToInnermostAbortEscape()
{
    if (g_abortCompilationEscapeCount == 0)
        return false;
    AbortCompilationEscape* escape = g_abortCompilationEscapes[--g_abortCompilationEscapeCount];
    escape->active = false;
    while (g_abortRecoveryLockCount > escape->lockStackDepthAtInstall)
    {
        AbortRecoveryLock* lock = g_abortRecoveryLocks[--g_abortRecoveryLockCount];
        lock->state = AbortRecoveryLock::State::Consumed;
        lock->mutex->unlock();
    }
    longjmp(escape->jumpBuffer, 1);
    // Unreachable: the jump above does not return.
    return false;
}

void reraiseAbortCompilation()
{
    // Propagates the in-flight AbortCompilation outward after a recovery branch has done its
    // local cleanup — the counterpart of `catch (...) { cleanup; throw; }`. The terminal path
    // below intentionally duplicates handleSignal's exit rather than calling it: a handleSignal
    // call would re-wrap g_lastSignalMessage, clobbering the original abort text the boundary
    // that ultimately recovers must report. With no enclosing recovery boundary left, the abort
    // terminates exactly like an unguarded one: the defined internal-compiler-error exit code,
    // no trap.
    if (!tryJumpToInnermostAbortEscape())
    {
        // SLANG_BREAKPOINT(0);
        exit(int(ToolReturnCode::InternalCompilerError));
    }
}
#endif

static const char* _getSignalTypeAsText(SignalType type)
{
    switch (type)
    {
    case SignalType::AssertFailure:
        return "assert failure";
    case SignalType::Unimplemented:
        return "unimplemented";
    case SignalType::Unreachable:
        return "hit unreachable code";
    case SignalType::Unexpected:
        return "unexpected";
    case SignalType::InvalidOperation:
        return "invalid operation";
    case SignalType::AbortCompilation:
        return "abort compilation";
    default:
        return "unhandled";
    }
}

String _getMessage(SignalType type, char const* message)
{
    StringBuilder buf;
    const char* const typeText = _getSignalTypeAsText(type);
    buf << typeText;
    if (message)
    {
        buf << ": " << message;
    }

    return buf.produceString();
}

// Returns true if the NUL-terminated strings `a` and `b` are equal ignoring ASCII case.
static bool _caseInsensitiveEquals(const char* a, const char* b)
{
    for (; *a && *b; ++a, ++b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
        {
            return false;
        }
    }
    return *a == *b;
}

/// Reads the `SLANG_ASSERT` environment variable into `buffer`, returning true if it was set and
/// fit. An over-long value is reported as unset, which is harmless since no mode name is that long.
///
/// Uses only C library calls and stack storage: Slang's own containers assert internally, so
/// building a `StringBuilder` here would let an assert inside them re-enter `handleAssert`
/// and recurse until the stack overflowed.
static bool _readAssertEnvVar(char* buffer, size_t bufferSize)
{
#if _WIN32 && defined(_MSC_VER)
    size_t requiredSize = 0;
    return getenv_s(&requiredSize, buffer, bufferSize, "SLANG_ASSERT") == 0 && requiredSize > 0;
#else
    const char* value = getenv("SLANG_ASSERT");
    if (!value)
    {
        return false;
    }
    // Copy with the same length that was bounds-checked, so the check and the copy cannot drift
    // apart the way a `strlen` guard followed by an unbounded `strcpy` can.
    const size_t length = strlen(value);
    if (length >= bufferSize)
    {
        return false;
    }
    memcpy(buffer, value, length + 1);
    return true;
#endif
}

void handleAssert(char const* message, char const* file, int line, bool isReleaseAssert)
{
    // Sized generously past the longest recognized mode name, "release-asserts-only".
    char envValue[64];
    if (_readAssertEnvVar(envValue, sizeof(envValue)))
    {
        if (_caseInsensitiveEquals(envValue, "release-asserts-only") ||
            _caseInsensitiveEquals(envValue, "release-assert-only"))
        {
            // Ignore the assert and continue execution.
            // This is to mimic the behavior of Release build with Debug build.
            if (!isReleaseAssert)
            {
                return;
            }
        }
#if _WIN32 && defined(_MSC_VER)
        else if (_caseInsensitiveEquals(envValue, "system"))
        {
            assert(!"SLANG_ASSERT triggered");
        }
        else if (_caseInsensitiveEquals(envValue, "debugbreak"))
        {
            if (IsDebuggerPresent())
            {
                SLANG_BREAKPOINT(0);
            }
            else
            {
                // Fallback when no debugger is attached
                assert(!"SLANG_ASSERT triggered (no debugger attached)");
            }
        }
#endif
    }

    // Strip any remaining directory prefix for readability (the build system already
    // maps the source root away via -fmacro-prefix-map / /d1trimfile).
    const char* basename = file ? file : "unknown";
    if (file)
    {
        for (const char* p = file; *p; ++p)
        {
            if (*p == '/' || *p == '\\')
                basename = p + 1;
        }
    }
    // Use a stack buffer to avoid heap allocation on the assertion path, which could
    // mask the original failure if the heap is corrupted.
    char locMsg[1024];
    const char* safeMessage = message ? message : "unknown assert";
    snprintf(locMsg, sizeof(locMsg), "%s(%d): %s", basename, line, safeMessage);
    handleSignal(SignalType::AssertFailure, locMsg);
}

// One point of having as a single function is a choke point both for handling (allowing different
// handling scenarios) as well as a choke point to set a breakpoint to catch 'signal' types
[[noreturn]] void handleSignal(SignalType type, char const* message)
{
    g_lastSignalMessage = _getMessage(type, message);

    // Can be useful to enable during debug when problem is on CI
    static bool enableSignalPrint = false;
    if (enableSignalPrint)
    {
        printf("%s\n", g_lastSignalMessage.getBuffer());
    }

#if SLANG_HAS_EXCEPTIONS
    switch (type)
    {
    case SignalType::InvalidOperation:
        throw InvalidOperationException(_getMessage(type, message));
    case SignalType::AbortCompilation:
        throw AbortCompilationException(_getMessage(type, message));
    default:
        throw InternalError(_getMessage(type, message));
    }
#else
    // With exceptions disabled nothing can throw, so a compilation abort can only reach recovery
    // code by jumping: if the abort happened inside an API boundary that installed an
    // AbortCompilationEscape, hand control back to that boundary, whose recovery branch reports
    // failure diagnostics and returns. Only SignalType::AbortCompilation takes the escape — all
    // non-AbortCompilation signals (Unexpected, Unimplemented, AssertFailure, Unreachable,
    // InvalidOperation) always terminate the process with the internal-compiler-error exit
    // code. Note this is a deliberate divergence from the exceptions build, where those signals
    // throw and boundaries recover them like any other exception; an internal error leaves
    // compiler state untrustworthy enough that terminating is the safer outcome here.
    if (type == SignalType::AbortCompilation)
    {
        // When no escape is installed this returns false and the abort falls through to the
        // exit below, exactly like an unguarded path.
        tryJumpToInnermostAbortEscape();
    }

    // A debugger trap here would turn every unrecovered abort and internal error into a
    // SIGILL or STATUS_BREAKPOINT (plus a core dump) inside the embedding process; exiting
    // with a defined code is the terminal behavior hosts can handle. The breakpoint stays
    // commented out for quick re-enabling under a debugger.
    //
    // SLANG_BREAKPOINT(0);

    // 'panic'. Exit with the internal-compiler-error code as we can't throw or catch.
    exit(int(ToolReturnCode::InternalCompilerError));
#endif
}

const char* getLastSignalMessage()
{
    return g_lastSignalMessage.getBuffer();
}

} // namespace Slang
