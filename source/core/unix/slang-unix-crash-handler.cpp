// slang-unix-crash-handler.cpp
#include "core/slang-crash-handler.h"
#include "core/slang-crash-report.h"
#include "core/slang-stack-trace.h"

#if !SLANG_WASM
#include <atomic>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

// macOS declares the saved register context in <sys/ucontext.h>; its <ucontext.h> holds only
// the deprecated context-switching functions and refuses to compile without _XOPEN_SOURCE.
#if SLANG_APPLE_FAMILY
#include <pthread.h>
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif
#endif

namespace Slang
{

#if SLANG_WASM

// A WebAssembly module has no signals. Its JavaScript host sees a trap as an exception and
// reports it there.

/* static */ bool CrashHandler::isSupported()
{
    return false;
}

/* static */ bool CrashHandler::install(const char* frontEnd, const char* slangVersion)
{
    SLANG_UNUSED(frontEnd);
    SLANG_UNUSED(slangVersion);
    return false;
}

/* static */ void CrashHandler::prepareThread() {}

#else

/// The signals that end a process because of a fault in it, with their names for the report.
struct FatalSignal
{
    int number;
    const char* name;
};
static const FatalSignal kFatalSignals[] = {
    {SIGSEGV, "SIGSEGV"},
    {SIGBUS, "SIGBUS"},
    {SIGILL, "SIGILL"},
    {SIGFPE, "SIGFPE"},
    {SIGABRT, "SIGABRT"},
};
static const int kFatalSignalCount = int(SLANG_COUNT_OF(kFatalSignals));

// The actions that were installed before ours. After the report, the handler puts the previous
// action back and raises the signal again, so an earlier handler, or the default action, still
// runs.
static struct sigaction g_previousActions[kFatalSignalCount];

// The handler builds its report here and not on the stack, because after a stack overflow it
// runs on the small alternate stack. Only one thread builds a report: see `g_isReporting`.
static CrashReportText g_report;

// Set by the first thread that enters the handler. A second thread that crashes while the
// first one reports waits for the process to end, so the two reports cannot interleave.
static std::atomic<bool> g_isReporting{false};

// The stack that the handler runs on. A thread whose stack overflowed has no stack left for
// the handler, so each thread that wants a report of a stack overflow registers this extra
// stack with `sigaltstack`. The size leaves room for the stack walk and the formatting.
static const size_t kAlternateStackSize = 64 * 1024;
static thread_local char t_alternateStack[kAlternateStackSize];

/// Return the name of the fatal signal `number`.
static const char* _getSignalName(int number)
{
    for (int i = 0; i < kFatalSignalCount; ++i)
    {
        if (kFatalSignals[i].number == number)
            return kFatalSignals[i].name;
    }
    return "unknown";
}

/// Return the program counter of the faulting instruction, from the context that the kernel
/// saved for the handler, or null on a platform that we do not read.
static void* _getFaultProgramCounter(const void* context)
{
    const ucontext_t* ucontext = static_cast<const ucontext_t*>(context);
#if SLANG_LINUX_FAMILY && SLANG_PROCESSOR_X86_64
    return reinterpret_cast<void*>(ucontext->uc_mcontext.gregs[REG_RIP]);
#elif SLANG_LINUX_FAMILY && SLANG_PROCESSOR_ARM_64
    return reinterpret_cast<void*>(ucontext->uc_mcontext.pc);
#elif SLANG_APPLE_FAMILY && SLANG_PROCESSOR_X86_64
    return reinterpret_cast<void*>(ucontext->uc_mcontext->__ss.__rip);
#elif SLANG_APPLE_FAMILY && SLANG_PROCESSOR_ARM_64
    return reinterpret_cast<void*>(__darwin_arm_thread_state64_get_pc(ucontext->uc_mcontext->__ss));
#else
    SLANG_UNUSED(ucontext);
    return nullptr;
#endif
}

#if SLANG_APPLE_FAMILY

/// Return the frame pointer of the interrupted code, from the saved register context.
static uintptr_t _getFaultFramePointer(const ucontext_t* ucontext)
{
#if SLANG_PROCESSOR_X86_64
    return uintptr_t(ucontext->uc_mcontext->__ss.__rbp);
#elif SLANG_PROCESSOR_ARM_64
    return uintptr_t(__darwin_arm_thread_state64_get_fp(ucontext->uc_mcontext->__ss));
#else
    SLANG_UNUSED(ucontext);
    return 0;
#endif
}

/// Capture the stack of the crashing thread into `outTrace` by following the chain of frame
/// pointers from the interrupted code: the faulting instruction first, then the return address
/// that each frame record holds.
///
/// We do not use `backtrace` here. Through the signal trampoline on macOS it reports the link
/// register of the interrupted code as a caller, which after any call is the return address of
/// that call and not a caller at all. Apple's ABIs keep frame pointers in every function, so
/// the chain is complete, except that a leaf function that faults has no frame record and its
/// own caller is missing from the trace.
static void _captureCrashStack(const void* context, StackTrace& outTrace)
{
    const ucontext_t* ucontext = static_cast<const ucontext_t*>(context);
    outTrace.frameCount = 0;
    void* const programCounter = _getFaultProgramCounter(context);
    if (programCounter)
        outTrace.returnAddresses[outTrace.frameCount++] = programCounter;

    // A frame record is two words: the caller's frame pointer and the return address. After a
    // crash the chain can point anywhere, so we read a record only inside this thread's stack
    // and only while the chain moves up the stack.
    const pthread_t thread = pthread_self();
    const uintptr_t stackTop = uintptr_t(pthread_get_stackaddr_np(thread));
    const uintptr_t stackBottom = stackTop - pthread_get_stacksize_np(thread);

    uintptr_t framePointer = _getFaultFramePointer(ucontext);
    while (outTrace.frameCount < StackTrace::kMaxFrameCount)
    {
        if (framePointer < stackBottom || framePointer + 2 * sizeof(uintptr_t) > stackTop ||
            (framePointer & (sizeof(uintptr_t) - 1)) != 0)
        {
            break;
        }
        const uintptr_t* record = reinterpret_cast<const uintptr_t*>(framePointer);
        const uintptr_t callerFramePointer = record[0];
        const uintptr_t returnAddress = record[1];
        if (returnAddress == 0)
            break;
        outTrace.returnAddresses[outTrace.frameCount++] = reinterpret_cast<void*>(returnAddress);
        if (callerFramePointer <= framePointer)
            break;
        framePointer = callerFramePointer;
    }
}

#else

/// Capture the stack of the crashing thread into `outTrace`, starting at the faulting
/// instruction.
///
/// The walk starts inside this handler, so its first frames are the handler and the code that
/// the kernel uses to call it. On Linux the unwinder follows the unwind tables through the
/// signal frame and reports the faulting instruction itself as one of the frames, so we start
/// there. If we cannot find it, we keep the whole walk, so that the report still has a stack.
static void _captureCrashStack(const void* context, StackTrace& outTrace)
{
    void* const faultProgramCounter = _getFaultProgramCounter(context);

    StackTrace walk;
    StackTraceUtil::capture(0, walk);

    int start = 0;
    for (int i = 0; i < walk.frameCount; ++i)
    {
        if (walk.returnAddresses[i] == faultProgramCounter)
        {
            start = i;
            break;
        }
    }

    outTrace.frameCount = 0;
    for (int i = start; i < walk.frameCount && outTrace.frameCount < StackTrace::kMaxFrameCount;
         ++i)
    {
        outTrace.returnAddresses[outTrace.frameCount++] = walk.returnAddresses[i];
    }
}

#endif

/// Write all `length` bytes of `text` to stderr, retrying a write that was cut short.
static void _writeToStandardError(const char* text, size_t length)
{
    size_t writtenLength = 0;
    while (writtenLength < length)
    {
        const ssize_t count = ::write(STDERR_FILENO, text + writtenLength, length - writtenLength);
        if (count > 0)
            writtenLength += size_t(count);
        else if (count < 0 && errno == EINTR)
            continue;
        else
            break;
    }
}

/// Put back the action that was installed for `number` before ours. If that action ignored the
/// signal, we use the default action instead: an ignored fault would run the faulting
/// instruction again forever.
static void _restorePreviousAction(int number)
{
    for (int i = 0; i < kFatalSignalCount; ++i)
    {
        if (kFatalSignals[i].number != number)
            continue;
        struct sigaction action = g_previousActions[i];
        if (!(action.sa_flags & SA_SIGINFO) && action.sa_handler == SIG_IGN)
            action.sa_handler = SIG_DFL;
        ::sigaction(number, &action, nullptr);
        return;
    }
}

static void _handleFatalSignal(int number, siginfo_t* info, void* context)
{
    bool expected = false;
    if (!g_isReporting.compare_exchange_strong(expected, true))
    {
        // Another thread is writing its report and will end the process.
        for (;;)
            ::pause();
    }

    // The handler calls the loader to name modules, and the loader takes a lock that the
    // crashing thread may already hold. The alarm ends the process if the handler gets stuck.
    ::alarm(10);

    StackTrace stack;
    _captureCrashStack(context, stack);

    const bool hasFaultAddress = number == SIGSEGV || number == SIGBUS;
    const uintptr_t faultAddress = info ? reinterpret_cast<uintptr_t>(info->si_addr) : 0;

    g_report.append("\n");
    if (CrashReportUtil::appendCrashReportOfThisProcess(
            _getSignalName(number),
            hasFaultAddress,
            faultAddress,
            stack,
            g_report))
    {
        _writeToStandardError(g_report.getBuffer(), size_t(g_report.getLength()));
    }

    // We end the process the way it would have ended without us. The signal is blocked while
    // this handler runs, so the raised signal arrives when the handler returns, and with the
    // previous action back in place, that action runs.
    _restorePreviousAction(number);
    ::raise(number);
}

/* static */ bool CrashHandler::isSupported()
{
    return true;
}

/* static */ void CrashHandler::prepareThread()
{
    stack_t alternateStack = {};
    alternateStack.ss_sp = t_alternateStack;
    alternateStack.ss_size = kAlternateStackSize;
    alternateStack.ss_flags = 0;
    ::sigaltstack(&alternateStack, nullptr);
}

/* static */ bool CrashHandler::install(const char* frontEnd, const char* slangVersion)
{
    CrashReportUtil::enableReports(frontEnd, slangVersion);
    prepareThread();

    // The first stack walk in a process can load the unwinder, which a signal handler must not
    // do, so we walk once here.
    StackTrace warmUp;
    StackTraceUtil::capture(0, warmUp);

    struct sigaction action = {};
    action.sa_sigaction = _handleFatalSignal;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&action.sa_mask);

    bool isInstalled = true;
    for (int i = 0; i < kFatalSignalCount; ++i)
    {
        if (::sigaction(kFatalSignals[i].number, &action, &g_previousActions[i]) != 0)
            isInstalled = false;
    }
    return isInstalled;
}

#endif

} // namespace Slang
