#ifndef SLANG_INTERNAL_H_INCLUDED
#define SLANG_INTERNAL_H_INCLUDED

#include "core/slang-crash-breadcrumbs.h"
#include "slang.h"

namespace Slang
{
struct GlobalSessionInternalDesc
{
    bool isBootstrap = false;
};
} // namespace Slang

SLANG_API SlangResult slang_createGlobalSessionImpl(
    const SlangGlobalSessionDesc* desc,
    const Slang::GlobalSessionInternalDesc* internalDesc,
    slang::IGlobalSession** outGlobalSession);

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

/// Turn crash reports on for this process, or off again with a null `frontEndName`.
///
/// While reports are on, an API call that fails on an internal compiler error ends its
/// diagnostics with a plain-text crash report: what failed, where in Slang, the Slang version,
/// the host and the call stack. `frontEndName` names the calling program in the report, for
/// example `slangc`, and must stay valid while reports are on. Reports are off by default, so a
/// host application that loads Slang sees no change unless it calls this.
SLANG_API void slang_enableCrashReports(const char* frontEndName);

/// Set the directory in which every crash report is also saved, as
/// `slang-report-<signature>.txt`, or pass null to save none. Nothing is saved by default.
/// The path is UTF-8 and is copied.
SLANG_API void slang_setCrashReportDirectory(const char* directory);

/// Return the crash breadcrumbs of the calling thread, as the Slang library sets them: the
/// phase, the IR pass, the target and the stage that the thread is working on. A crash handler
/// in a program that links its own copy of `core` reads them through this function.
SLANG_API const Slang::CrashBreadcrumbs* slang_getCrashBreadcrumbs();

#endif
