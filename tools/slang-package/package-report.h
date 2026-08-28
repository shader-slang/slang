// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SLANG_PACKAGE_REPORT_H
#define SLANG_PACKAGE_REPORT_H

#include "package-lock.h"
#include "package-types.h"

namespace Slang
{
namespace PackageTool
{

enum class ResolveSelectionKind
{
    HighestRelease,
    PinnedRef,
    Path,
    Override,
    Edit,
};

enum class ResolveChangeKind
{
    Added,
    Removed,
    Upgraded,
    Downgraded,
    Replaced,
    Unchanged,
};

/// One incoming version requirement that participated in selecting a package.
struct ResolveConstraintNote
{
    String ownerName;
    String ownerVersion;
    String text;
    VersionConstraint constraint;
};

/// A candidate the solver considered and skipped before the selected release.
struct ResolveSkipNote
{
    String version;
    String reason;
};

/// Solver explanation for one reachable package in the selected graph.
struct ResolvePackageExplanation
{
    String name;
    String version;
    String git;
    String ref;
    String path;
    ResolveSelectionKind selectionKind = ResolveSelectionKind::HighestRelease;
    List<ResolveConstraintNote> constraints;
    List<ResolveSkipNote> skips;
};

/// Graph-wide data collected during a solve, used to explain lock changes.
struct ResolveReport
{
    String rootPackageName;
    List<ResolvePackageExplanation> packages;
    List<ToolchainConstraint> toolchainConstraints;
    String installedToolchain;
};

/// Format a resolve report for `slang package update`.
///
/// Default output is a Gradle-style rationale for each package, then Go-style one-liners for
/// packages that moved, then a count. `--minimal` keeps the one-liners, including unchanged
/// packages, and the count.
String formatResolveReport(
    const Manifest& rootManifest,
    const LockFile* previous,
    const LockFile& next,
    const ResolveReport& report,
    bool dryRun,
    bool minimal);

} // namespace PackageTool
} // namespace Slang

#endif
