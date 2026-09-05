// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_TOOL_H
#define SLANG_PACKAGE_TOOL_H

#include "core/slang-basic.h"
#include "core/slang-command-line.h"

namespace Slang
{
namespace PackageTool
{

SlangResult executeInDirectory(
    const String& projectRoot,
    int argc,
    const char* const* argv,
    String& outError);

/// Locate the package whose `slang-package.json` should be used for commands started in
/// `startDirectory`.
///
/// Walk toward filesystem root and stop at the nearest `slang-package.json`. Ordinary
/// subdirectories such as `src/` therefore load that package's manifest, lock, and
/// `slang-workspace.json`. A nested package (for example a dependency under `deps/` or an example
/// with its own manifest) keeps that nearer root. `init` does not use this discovery path.
SlangResult discoverPackageRoot(const String& startDirectory, String& outRoot, String& outError);

/// Produce the same workspace-state report printed by `slang package status`.
///
/// A current workspace is one header line. Drift is listed only when something is dirty.
/// The header says `buildable`, `incomplete` (lock or checkouts missing), or `not buildable`.
/// Unreadable or malformed required JSON is returned through `outError` as a failure.
SlangResult getWorkspaceStatusReport(
    const String& projectRoot,
    String& outReport,
    String& outError);

/// Format one command failure exactly as the `slang-package` executable writes it to stderr.
String formatCommandError(const String& error);

/// Return whether a confirmation answer typed at a prompt approves the operation.
///
/// The answer is passed in exactly as it was read, so it may still carry its line terminator and
/// surrounding spaces. Only an explicit "y" or "yes", in any casing, approves; every other answer,
/// including an empty line, declines.
bool isAffirmativeConfirmationAnswer(const UnownedStringSlice& answer);

/// Fill `outCommand` with the host command that opens `path` in the registered application for
/// its file type: `open` on macOS, `xdg-open` on other Unix, and `cmd /c start` on Windows.
void getRegisteredApplicationOpenCommand(const String& path, CommandLine& outCommand);

/// Run a `slang-package` command as if the process current directory were `startDirectory`.
///
/// Package commands that load workspace JSON discover the enclosing package from that directory.
/// `init` still creates a package in `startDirectory` itself.
int executeFromStartDirectory(const String& startDirectory, int argc, const char* const* argv);

int execute(int argc, const char* const* argv);

} // namespace PackageTool
} // namespace Slang

#endif
