// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_BUNDLE_H
#define SLANG_PACKAGE_BUNDLE_H

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "package-types.h"
#include "package-validate.h"

namespace Slang
{
namespace PackageTool
{

/// Recreate `path` as an empty directory, deleting any previous files.
SlangResult resetDirectory(const String& path, String& outError);

/// Write `build/bundle/modules/provenance.json` so a consumer can require the same unversioned
/// `.slang-module` toolchain that produced the files beside it.
SlangResult writeModuleProvenance(
    const String& modulesRoot,
    const String& slangcPath,
    String& outError);

/// Copy every exported `.slang` file into `build/bundle/source` using import-relative paths, so
/// that directory is a single compiler search path. Two files that would occupy the same name on a
/// case-insensitive filesystem are an error.
SlangResult copyBundleSource(
    const String& sourceRoot,
    const List<ExportedSourceFile>& sourceFiles,
    String& outError);

} // namespace PackageTool
} // namespace Slang

#endif
