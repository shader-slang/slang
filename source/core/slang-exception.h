#ifndef SLANG_CORE_EXCEPTION_H
#define SLANG_CORE_EXCEPTION_H

#include "slang-common.h"
#include "slang-signal.h"
#include "slang-string.h"

// Macroizes the opening `try` of an exception-guarded block. The matching `catch` clauses of a
// plain SLANG_EXCEPTION_TRY block must still be wrapped in `#if SLANG_HAS_EXCEPTIONS` directly:
// once the `try` is elided under -fno-exceptions a `catch` is ill-formed, so the whole clause is
// removed by the preprocessor (this applies even to an empty `catch (...) {}`). With exceptions
// disabled an internal abort terminates in handleSignal() instead of throwing, so a plain
// SLANG_EXCEPTION_TRY block forfeits the graceful recovery its catch clauses provide
// (diagnostics, fallback returns, language-server survival).
//
// A boundary that must recover from compilation aborts in every build uses
// SLANG_EXCEPTION_TRY_RECOVERY instead. Which boundaries qualify is a stated contract: a site is
// converted when it is a point where an embedder's failed-compile must surface as a returned
// failure plus diagnostics rather than process death — the outermost public entry points that
// load or compile user code (module load, end-to-end compile, link and its artifact/code
// getters) — or an interior site whose catch arm does shared-state cleanup the recovery must
// reproduce (the module-load registration sites). Other guarded sites keep plain
// SLANG_EXCEPTION_TRY for now, with two distinct fates in a build without exceptions: an abort
// at such a site terminates in handleSignal() when no recovery boundary encloses it — today's
// behavior, and the safe default for the still-unconverted public entry points (specializeType,
// the reflection API, the language server's own paths) — but when a recovery boundary does
// enclose it dynamically, the abort reraises straight through the site to that boundary without
// running its own catch arm, so the site's arm-level effects (for example
// loadSourceModuleImpl's per-import context reporting) do not happen in that configuration.
// Coverage is extended boundary by boundary against that contract.
//
// In the disabled configuration SLANG_EXCEPTION_TRY_RECOVERY installs a per-thread
// AbortCompilationEscape and turns the abort into a longjmp back to the boundary. The recovery
// branch is written as SLANG_EXCEPTION_ABORT_RECOVERY inside the `#else` of the same
// `#if SLANG_HAS_EXCEPTIONS` — it expands to `else`, completing the `if (setjmp(...) == 0)` the
// opening macro left open. The macro is deliberately NOT defined in the exceptions branch: a
// site writing it outside its `#else` then fails to compile, which guards against the misuse of
// placing the recovery branch where the exceptions build could reach it. At most sites it
// mirrors the `catch (const AbortCompilationException&)` arm; a site whose only arm is
// `catch (const Exception&)` mirrors that instead.
#if SLANG_HAS_EXCEPTIONS
#define SLANG_EXCEPTION_TRY try
#define SLANG_EXCEPTION_TRY_RECOVERY try
#else
#include <setjmp.h>
#define SLANG_EXCEPTION_TRY
// A recovery-enabled guarded block. `setjmp` in the controlling expression of an `if` compared
// against an integer constant is one of the standard-sanctioned positions for it.
//
// Locals of the enclosing function that change between this point and the longjmp are formally
// indeterminate once the jump lands, so a recovery branch may only write them; only a `volatile`
// local is guaranteed to read back its most recent pre-jump value, which a branch that keeps a
// pre-set failure result needs. State created entirely inside the recovery branch is a fresh
// object with no setjmp exposure, so it is always safe. The boundary sinks (DiagnosticSink etc.)
// are a deliberate exception in the other direction: they are created before this macro, live in
// memory (their address escapes into every callee), and are only mutated through calls in the
// guarded region, so reading and appending to them after the jump is reliable in practice on the
// compilers Slang targets, though the standard technically makes them indeterminate too.
//
// The sharper hazard is the frames between the abort site and this boundary: the longjmp skips
// every destructor in them, so RAII state there — memory, and more importantly locks and other
// shared-state bookkeeping — is silently abandoned. A lock held by a frame below a recovery
// boundary must use AbortRecoveryLock (see slang-signal.h), which handleSignal releases before
// the jump; any other shared state a callee's RAII owns must either be safe to strand or be
// restored by the recovery branch itself (for example
// Linkage::resetModulesBeingImported()). The memory cost is not benign for retrying hosts: a
// recovered abort abandons the crossed frames' RefPtr-owned state — one full front-end compile
// attempt per recovered abort, unbounded under repeated retries — which we accept because the
// alternative in this configuration is terminating the host process. Hosts that need bounded
// retries need the crossed frames to register their owning state with the escape; that is a
// follow-up, not part of this mechanism.
//
// The recovery counterpart of an exceptions-build catch arm that performs local cleanup and
// rethrows (for example removing a failed module from the loaded-module caches) is the same
// cleanup in the recovery branch followed by reraiseAbortCompilation(), which jumps to the next
// enclosing recovery boundary and preserves the recorded abort message — so intermediate
// boundaries each run their own cleanup on the way out, exactly as unwinding them would.
//
// The macro is one guard per function: two SLANG_EXCEPTION_TRY_RECOVERY blocks in the same
// function would redeclare _slangAbortEscape and fail to compile.
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
