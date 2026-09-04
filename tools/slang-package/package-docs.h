// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_DOCS_H
#define SLANG_PACKAGE_DOCS_H

#include "core/slang-basic.h"

namespace Slang
{
namespace PackageTool
{

/// Copy Markdown documentation from the workspace and every materialized dependency under
/// `build/docs/<package-name>/`, then write `build/docs/index.md` with the full package
/// relationship tree and an alphabetized file list for packages that contributed Markdown.
SlangResult buildDocumentation(const String& projectRoot, String& outError);

} // namespace PackageTool
} // namespace Slang

#endif
