// slang-wasi-process.cpp — Process interface stubs for the WASI target.
//
// WASI has no process spawning (no fork/exec/posix_spawn) and no signals,
// so most of the Process interface is either stubbed to SLANG_FAIL or
// implemented with the subset of POSIX that wasi-libc does provide
// (clock_gettime, nanosleep).

#include "../slang-common.h"
#include "../slang-process.h"
#include "../slang-string-escape-util.h"

#include <time.h>

namespace Slang
{

/* static */ UnownedStringSlice Process::getExecutableSuffix()
{
    // WASI has no executable concept; return an empty suffix.
    return UnownedStringSlice::fromLiteral("");
}

/* static */ StringEscapeHandler* Process::getEscapeHandler()
{
    return StringEscapeUtil::getHandler(StringEscapeUtil::Style::Space);
}

/* static */ SlangResult Process::create(
    const CommandLine& /* commandLine */,
    Process::Flags /* flags */,
    RefPtr<Process>& /* outProcess */)
{
    // Process spawning is not available on WASI.
    return SLANG_E_NOT_IMPLEMENTED;
}

/* static */ uint64_t Process::getClockFrequency()
{
    return 1000000000; // nanoseconds
}

/* static */ uint64_t Process::getClockTick()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return uint64_t(now.tv_sec) * 1000000000 + uint64_t(now.tv_nsec);
}

/* static */ void Process::sleepCurrentThread(Int timeInMs)
{
    if (timeInMs <= 0)
        return;

    struct timespec ts;
    ts.tv_sec = timeInMs / 1000;
    ts.tv_nsec = (timeInMs % 1000) * 1000000;
    nanosleep(&ts, nullptr);
}

} // namespace Slang
