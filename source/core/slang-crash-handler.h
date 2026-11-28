// slang-crash-handler.h
#ifndef SLANG_CORE_CRASH_HANDLER_H
#define SLANG_CORE_CRASH_HANDLER_H

#include "slang-common.h"

namespace Slang
{

/// The handler that writes a crash report when the operating system stops the process: on a
/// segmentation fault, a bus error, an illegal instruction, a floating-point exception, or an
/// abort.
///
/// An internal compiler error is a failure that Slang detects and reports itself. A crash is
/// one that it does not detect, so the only code that can report it is a handler that the
/// operating system calls. The handler writes the same plain-text report to stderr, saves it
/// like the report of an internal error, and then lets the default action of the signal run,
/// so the exit status and any core dump are the same as without the handler.
///
/// Only a program installs the handler, at start-up: the handler of a fatal signal is a
/// setting of the whole process, and a library must not replace the handler of the
/// application that loaded it.
struct CrashHandler
{
    /// Install the handler for this process and turn crash reports on, with `frontEnd` and
    /// `slangVersion` as for `CrashReportUtil::enableReports`. Return false if this platform
    /// has no crash handler.
    ///
    /// A thread that the process starts later shares the handler. A stack overflow on such a
    /// thread is reported only if the thread calls `prepareThread`.
    static bool install(const char* frontEnd, const char* slangVersion);

    /// Give the calling thread the stack that the handler runs on after a stack overflow.
    static void prepareThread();

    /// Return true if this platform has a crash handler.
    static bool isSupported();
};

} // namespace Slang

#endif
