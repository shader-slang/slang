// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_VALIDATE_H
#define SLANG_PACKAGE_VALIDATE_H

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{
namespace PackageTool
{

struct PrimaryModule
{
    String importPath;
    String sourcePath;
};

/// Return the placeholder text written by `slang package init`.
const char* getLicensePlaceholderText();

/// Validate the workspace package and its materialized, locked dependency closure. When requested,
/// return the workspace package's primary modules in import-path order.
SlangResult validateProject(
    const String& projectRoot,
    String& outError,
    List<String>* outWarnings = nullptr,
    List<PrimaryModule>* outPrimaryModules = nullptr);

} // namespace PackageTool
} // namespace Slang

#endif
