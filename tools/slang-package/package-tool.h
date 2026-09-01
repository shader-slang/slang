// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_TOOL_H
#define SLANG_PACKAGE_TOOL_H

#include "core/slang-basic.h"

namespace Slang
{
namespace PackageTool
{

SlangResult executeInDirectory(
    const String& projectRoot,
    int argc,
    const char* const* argv,
    String& outError);

/// Return whether a confirmation answer typed at a prompt approves the operation.
///
/// The answer is passed in exactly as it was read, so it may still carry its line terminator and
/// surrounding spaces. Only an explicit "y" or "yes", in any casing, approves; every other answer,
/// including an empty line, declines.
bool isAffirmativeConfirmationAnswer(const UnownedStringSlice& answer);

int execute(int argc, const char* const* argv);

} // namespace PackageTool
} // namespace Slang

#endif
