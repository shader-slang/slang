// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_PATH_H
#define SLANG_PACKAGE_PATH_H

#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

/// Return true when `canonicalPath` is `canonicalRoot` or a file under it.
///
/// Both arguments must already be canonical. Prefix comparison is only safe after `.` and `..`
/// have been resolved, and after the paths share a filesystem so `Path::getCanonical` succeeded.
bool isCanonicalPathWithin(const String& canonicalRoot, const String& canonicalPath);

/// Return true when a relative path's first component is `..`.
///
/// Git nested path dependencies are resolved with `git show` of a repo-relative path. A leading
/// `..` would leave that tree, so the resolver rejects it instead of warning the way a local
/// filesystem path dependency does.
bool pathStartsWithParentComponent(const String& path);

/// Reject a path that points at `.slang` unless the declaring package itself lives there.
///
/// Consider this layout: the workspace at `/game` depends on `.slang/cache/evil`. That directory is
/// under the workspace root, so a naive "stay inside the declaring tree" check would allow it, and
/// a crafted lock could then treat resolver state as a path package. The intended rule is: normal
/// packages cannot select trees under the workspace's hidden state directory.
SlangResult validatePathDoesNotEscapeIntoToolState(
    const String& projectRoot,
    const String& canonicalDeclaringRoot,
    const String& canonicalPath,
    const String& dependencyName,
    String& outError);

/// Confirm a path dependency and its lock entry name the same directory, then apply the `.slang`
/// rule.
///
/// When `outWarnings` is non-null, a path that leaves the declaring package (for example
/// `../shared`) is recorded as a warning rather than an error, matching `slang package update` and
/// `validate`.
SlangResult validateLockedPathDependency(
    const String& projectRoot,
    const String& declaringRoot,
    const String& declaringPackageName,
    const Dependency& dependency,
    const LockedPackage& lockedPackage,
    String& outError,
    List<String>* outWarnings = nullptr);

} // namespace PackageTool
} // namespace Slang

#endif
