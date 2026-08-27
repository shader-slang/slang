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
    String packageName;
    String sourcePath;
};

enum class ProjectValidationMode
{
    Full,
    SourceAndDependencies,
};

/// Return the placeholder text written by `slang package init`.
const char* getLicensePlaceholderText();

/// Validate the workspace package and its materialized, locked dependency closure. When requested,
/// return every primary module in the resolved graph in import-path order.
SlangResult validateProject(
    const String& projectRoot,
    String& outError,
    List<String>* outWarnings = nullptr,
    List<PrimaryModule>* outPrimaryModules = nullptr,
    ProjectValidationMode mode = ProjectValidationMode::Full);

} // namespace PackageTool
} // namespace Slang

#endif
