// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_RESOLVER_H
#define SLANG_PACKAGE_RESOLVER_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

SlangResult resolveDependencies(
    const String& projectRoot,
    const Manifest& manifest,
    LockFile& outLock,
    String& outError);

} // namespace PackageTool
} // namespace Slang

#endif
