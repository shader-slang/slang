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

int execute(int argc, const char* const* argv);

} // namespace PackageTool
} // namespace Slang

#endif
