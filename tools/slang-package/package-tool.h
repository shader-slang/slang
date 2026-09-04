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

int execute(int argc, const char* const* argv);

} // namespace PackageTool
} // namespace Slang

#endif
