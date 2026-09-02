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
    /// Identifies the selected representation that contributed this requirement, matching the
    /// owner recorded on the solver's requirement for the same edge. The solver retracts notes and
    /// requirements together, so a report never cites a requirement that is no longer enforced.
    /// This is solver identity rather than display text and is not printed.
    String ownerKey;
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

/// One candidate rejected while proving that a package cannot be selected.
struct ResolveCandidateRejection
{
    String version;
    String reason;
};

/// The package conflict that ended dependency resolution.
///
/// When the solver backtracks it explores candidate branches from the highest release downward, so
/// this records the conflict reached on the last branch it tried rather than the one nested most
/// deeply. Every branch had to fail for the solve to fail, so any one of them is a truthful
/// explanation; the last is reported because it is the branch the solver gave up on.
///
/// The constraints identify every package that requires this package. The candidate list records
/// why every published release was unavailable, so a caller can render the failed solve without
/// reconstructing solver state from a nested error string.
struct ResolveFailure
{
    String packageName;
    List<ResolveConstraintNote> constraints;
    List<ResolveCandidateRejection> candidates;
};

/// Graph-wide data collected during a solve, used to explain lock changes.
struct ResolveReport
{
    String rootPackageName;
    List<ResolvePackageExplanation> packages;
    List<ToolchainConstraint> toolchainConstraints;
    String installedToolchain;
    ResolveFailure failure;
};

/// Format the package conflict that prevented dependency resolution from succeeding.
String formatResolveFailure(const ResolveFailure& failure);

/// Format a resolve report for `slang package update`.
///
/// Default output is a rationale for each selected package, then a count. `--minimal` keeps
/// one-liners for every selected package, including unchanged ones, and the count. The installed
/// Slang toolchain is not listed unless resolution fails its constraint.
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
