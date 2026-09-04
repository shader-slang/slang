// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "package-report.h"

#include "core/slang-string.h"

namespace Slang
{
namespace PackageTool
{

struct ResolveChange
{
    ResolveChangeKind kind = ResolveChangeKind::Unchanged;
    String name;
    String previousVersion;
    String nextVersion;
    String nextPath;
    const ResolvePackageExplanation* explanation = nullptr;
};

static const ResolvePackageExplanation* _findExplanation(
    const ResolveReport& report,
    const String& name)
{
    for (const auto& explanation : report.packages)
    {
        if (explanation.name == name)
            return &explanation;
    }
    return nullptr;
}

static String _ownerLabel(const ResolveConstraintNote& note)
{
    if (!note.ownerVersion.getLength())
        return note.ownerName;
    return note.ownerName + "@" + note.ownerVersion;
}

/// Write one candidate's rejection reason and terminate the line, aligning any continuation lines
/// under the first so the surrounding list stays readable.
///
/// Most reasons are a single clause such as "outside required range >=2.0.0". Some are not: when a
/// candidate's manifest fails to parse, the reason carries the JSON diagnostic and the source text
/// with it, and printing that verbatim would leave unindented lines sitting at the same margin as
/// the "Candidates considered:" heading, so the reader can no longer tell where one candidate ends
/// and the next begins. Indenting the continuation keeps the reason visually attached to its
/// candidate.
static void _appendReason(StringBuilder& builder, const String& reason)
{
    UnownedStringSlice rest = reason.getUnownedSlice();
    while (rest.getLength() &&
           (rest[rest.getLength() - 1] == '\n' || rest[rest.getLength() - 1] == '\r'))
    {
        rest = rest.head(rest.getLength() - 1);
    }
    for (;;)
    {
        Index newline = rest.indexOf('\n');
        if (newline < 0)
        {
            builder << rest << "\n";
            return;
        }
        builder << rest.head(newline) << "\n    ";
        rest = rest.tail(newline + 1);
    }
}

String formatResolveFailure(const ResolveFailure& failure)
{
    StringBuilder builder;
    builder << "Could not select a version for package '" << failure.packageName << "'.\n";

    if (failure.constraints.getCount())
    {
        builder << "\nRequired by:\n";
        for (const auto& constraint : failure.constraints)
            builder << "  " << _ownerLabel(constraint) << " requires " << constraint.text << "\n";
    }

    if (failure.candidates.getCount())
    {
        builder << "\nCandidates considered:\n";
        for (const auto& candidate : failure.candidates)
        {
            builder << "  " << candidate.version << ": ";
            _appendReason(builder, candidate.reason);
        }
        builder << "\nNo candidate satisfies every requirement for package '" << failure.packageName
                << "'.\n";
        builder << "Help: adjust the listed version requirements or workspace exclusions, then run "
                   "'slang package update' again.";
    }
    else
    {
        builder << "\nPackage '" << failure.packageName
                << "' has no published releases to select from.\n";
        builder << "Help: confirm the dependency points at the right Git location and that it "
                   "publishes 'vMAJOR.MINOR.PATCH' release tags.";
    }
    return builder.produceString();
}

static Index _changeKindOrder(ResolveChangeKind kind)
{
    switch (kind)
    {
    case ResolveChangeKind::Upgraded:
        return 0;
    case ResolveChangeKind::Added:
        return 1;
    case ResolveChangeKind::Downgraded:
        return 2;
    case ResolveChangeKind::Replaced:
        return 3;
    case ResolveChangeKind::Removed:
        return 4;
    default:
        return 5;
    }
}

static const char* _changeVerb(ResolveChangeKind kind, bool dryRun)
{
    switch (kind)
    {
    case ResolveChangeKind::Added:
        return dryRun ? "would add" : "added";
    case ResolveChangeKind::Removed:
        return dryRun ? "would remove" : "removed";
    case ResolveChangeKind::Upgraded:
        return dryRun ? "would upgrade" : "upgraded";
    case ResolveChangeKind::Downgraded:
        return dryRun ? "would downgrade" : "downgraded";
    case ResolveChangeKind::Replaced:
        return dryRun ? "would replace" : "replaced";
    case ResolveChangeKind::Unchanged:
        return "unchanged";
    }
    return "unchanged";
}

static String _headlineRest(const ResolveChange& change)
{
    switch (change.kind)
    {
    case ResolveChangeKind::Added:
    case ResolveChangeKind::Unchanged:
        return change.nextVersion;
    case ResolveChangeKind::Removed:
        return change.previousVersion;
    case ResolveChangeKind::Upgraded:
    case ResolveChangeKind::Downgraded:
        return change.previousVersion + " => " + change.nextVersion;
    case ResolveChangeKind::Replaced:
        if (change.nextPath.getLength())
            return change.previousVersion + " with " + change.nextPath + " as " +
                   change.nextVersion;
        return change.previousVersion + " => " + change.nextVersion;
    }
    return change.nextVersion;
}

static String _goHeadlineRest(const ResolveChange& change)
{
    if (change.kind == ResolveChangeKind::Replaced && change.nextPath.getLength())
        return change.previousVersion + " => " + change.nextPath + " as " + change.nextVersion;
    return _headlineRest(change);
}

static ResolveChangeKind _classifyChange(const LockedPackage* previous, const LockedPackage& next)
{
    if (!previous)
        return ResolveChangeKind::Added;
    if (lockedPackagesEqual(*previous, next))
        return ResolveChangeKind::Unchanged;
    const bool previousPath = previous->path.getLength() != 0;
    const bool nextPath = next.path.getLength() != 0;
    if (previousPath != nextPath || previous->git != next.git || previous->path != next.path)
        return ResolveChangeKind::Replaced;
    SemanticVersion previousVersion;
    SemanticVersion nextVersion;
    String error;
    if (SLANG_SUCCEEDED(parseExactVersion(previous->version, previousVersion, error)) &&
        SLANG_SUCCEEDED(parseExactVersion(next.version, nextVersion, error)))
    {
        if (nextVersion > previousVersion)
            return ResolveChangeKind::Upgraded;
        if (nextVersion < previousVersion)
            return ResolveChangeKind::Downgraded;
    }
    if (previous->version != next.version || previous->ref != next.ref ||
        previous->commit != next.commit)
        return ResolveChangeKind::Replaced;
    return ResolveChangeKind::Unchanged;
}

static void _addSortedName(List<String>& names, const String& name)
{
    for (Index i = 0; i < names.getCount(); ++i)
    {
        if (names[i] == name)
            return;
        if (names[i] > name)
        {
            names.insert(i, name);
            return;
        }
    }
    names.add(name);
}

static void _appendConstraintLines(
    StringBuilder& builder,
    const List<ResolveConstraintNote>& constraints,
    const char* indent)
{
    Index labelWidth = 0;
    for (const auto& note : constraints)
    {
        Index width = _ownerLabel(note).getLength();
        if (width > labelWidth)
            labelWidth = width;
    }
    for (const auto& note : constraints)
    {
        String label = _ownerLabel(note);
        builder << indent << label;
        for (Index i = label.getLength(); i < labelWidth; ++i)
            builder << " ";
        builder << ": " << note.text << "\n";
    }
}

static bool _constraintMatches(const ResolveConstraintNote& note, const SemanticVersion& version)
{
    if (note.constraint.clauses.getCount() == 0)
        return true;
    return note.constraint.matches(version);
}

static void _appendRejectedPrevious(StringBuilder& builder, const ResolveChange& change)
{
    if (!change.previousVersion.getLength() || !change.explanation)
        return;
    SemanticVersion previous;
    String error;
    if (SLANG_FAILED(parseExactVersion(change.previousVersion, previous, error)))
        return;
    for (const auto& note : change.explanation->constraints)
    {
        if (_constraintMatches(note, previous))
            continue;
        builder << "    " << change.previousVersion << " rejected by " << _ownerLabel(note) << ": "
                << note.text << "\n";
    }
}

static void _appendSelectionReason(StringBuilder& builder, const ResolveChange& change)
{
    const ResolvePackageExplanation* explanation = change.explanation;
    if (!explanation)
        return;

    const bool unchanged = change.kind == ResolveChangeKind::Unchanged;
    const char* satisfyVerb =
        unchanged ? "remains highest release satisfying" : "selected highest release satisfying";

    switch (explanation->selectionKind)
    {
    case ResolveSelectionKind::PinnedRef:
        builder << "    " << (unchanged ? "remains pinned ref " : "selected pinned ref ")
                << explanation->ref << " as " << explanation->version << "\n";
        if (explanation->constraints.getCount())
        {
            builder << "    as " << explanation->version << " satisfies:\n";
            _appendConstraintLines(builder, explanation->constraints, "      ");
        }
        break;
    case ResolveSelectionKind::Path:
        builder << "    " << (unchanged ? "remains path " : "selected path ") << explanation->path
                << " as " << explanation->version << "\n";
        if (explanation->constraints.getCount())
        {
            builder << "    as " << explanation->version << " satisfies incoming constraints:\n";
            _appendConstraintLines(builder, explanation->constraints, "      ");
        }
        break;
    case ResolveSelectionKind::Override:
        builder << "    "
                << (unchanged ? "remains a workspace override\n"
                              : "selected by workspace override\n");
        if (explanation->constraints.getCount())
        {
            builder << "    as " << explanation->version
                    << " satisfies incoming Git constraints:\n";
            _appendConstraintLines(builder, explanation->constraints, "      ");
        }
        break;
    case ResolveSelectionKind::Edit:
        builder << "    "
                << (unchanged ? "remains an in-place edit at " : "selected by in-place edit at ")
                << explanation->path << "\n";
        break;
    case ResolveSelectionKind::HighestRelease:
        if (explanation->constraints.getCount())
        {
            builder << "    " << satisfyVerb << ":\n";
            _appendConstraintLines(builder, explanation->constraints, "      ");
        }
        break;
    }

    _appendRejectedPrevious(builder, change);
    for (const auto& skip : explanation->skips)
        builder << "    skipped " << skip.version << ": " << skip.reason << "\n";
}

static void _appendRemovedFromLock(
    StringBuilder& builder,
    const LockFile& previous,
    const ResolveChange& change)
{
    for (const auto& package : previous.packages)
    {
        for (const auto& dependency : package.dependencies)
        {
            if (dependency.name != change.name)
                continue;
            String text = dependency.version;
            if (!text.getLength() && dependency.as.getLength())
                text = String("as ") + dependency.as;
            builder << "    last required by " << package.name << "@" << package.version;
            if (text.getLength())
                builder << ": " << text;
            builder << "\n";
        }
    }
}

static void _appendDetails(
    StringBuilder& builder,
    const Manifest& rootManifest,
    const LockFile* previous,
    const ResolveChange& change)
{
    if (change.kind == ResolveChangeKind::Removed)
    {
        builder << "    no longer reachable from the workspace root\n";
        for (const auto& dependency : rootManifest.dependencies)
        {
            if (dependency.name != change.name)
                continue;
            String text = dependency.version;
            if (!text.getLength() && dependency.as.getLength())
                text = String("as ") + dependency.as;
            builder << "    last required by " << rootManifest.name;
            if (text.getLength())
                builder << ": " << text;
            builder << "\n";
        }
        if (previous)
            _appendRemovedFromLock(builder, *previous, change);
        return;
    }
    _appendSelectionReason(builder, change);
}

static void _collectChanges(
    const Manifest& rootManifest,
    const LockFile* previous,
    const LockFile& next,
    const ResolveReport& report,
    List<ResolveChange>& outChanges)
{
    SLANG_UNUSED(rootManifest);
    List<String> names;
    if (previous)
    {
        for (const auto& package : previous->packages)
            _addSortedName(names, package.name);
    }
    for (const auto& package : next.packages)
        _addSortedName(names, package.name);

    for (const auto& name : names)
    {
        Index previousIndex = previous ? findLockedPackageIndex(*previous, name) : -1;
        Index nextIndex = findLockedPackageIndex(next, name);
        ResolveChange change;
        change.name = name;
        change.explanation = _findExplanation(report, name);
        if (previousIndex >= 0)
        {
            change.previousVersion = previous->packages[previousIndex].version;
        }
        if (nextIndex < 0)
        {
            change.kind = ResolveChangeKind::Removed;
            outChanges.add(change);
            continue;
        }
        const LockedPackage& nextPackage = next.packages[nextIndex];
        change.nextVersion = nextPackage.version;
        change.nextPath = nextPackage.path;
        const LockedPackage* previousPackage =
            previousIndex >= 0 ? &previous->packages[previousIndex] : nullptr;
        change.kind = _classifyChange(previousPackage, nextPackage);
        outChanges.add(change);
    }

    outChanges.sort(
        [](const ResolveChange& left, const ResolveChange& right)
        {
            Index leftOrder = _changeKindOrder(left.kind);
            Index rightOrder = _changeKindOrder(right.kind);
            if (leftOrder != rightOrder)
                return leftOrder < rightOrder;
            return left.name < right.name;
        });
}

static String _countLine(const List<ResolveChange>& changes, bool dryRun)
{
    Index added = 0;
    Index removed = 0;
    Index upgraded = 0;
    Index downgraded = 0;
    Index replaced = 0;
    Index unchanged = 0;
    Index moved = 0;
    for (const auto& change : changes)
    {
        switch (change.kind)
        {
        case ResolveChangeKind::Added:
            ++added;
            ++moved;
            break;
        case ResolveChangeKind::Removed:
            ++removed;
            ++moved;
            break;
        case ResolveChangeKind::Upgraded:
            ++upgraded;
            ++moved;
            break;
        case ResolveChangeKind::Downgraded:
            ++downgraded;
            ++moved;
            break;
        case ResolveChangeKind::Replaced:
            ++replaced;
            ++moved;
            break;
        case ResolveChangeKind::Unchanged:
            ++unchanged;
            break;
        }
    }

    // The summary is a standalone sentence, so it is capitalized for both the dry-run and the
    // applied spelling. Per-package lines are list entries and stay lowercase for every verb, so
    // capitalization tracks the role of the line rather than which verb the line happens to use.
    StringBuilder builder;
    if (moved == 0)
    {
        builder << "No package versions changed.\n";
        builder << (dryRun ? "Would update " : "Updated ");
        builder << "0 packages; " << unchanged << " unchanged.\n";
        return builder.produceString();
    }

    builder << (dryRun ? "Would update " : "Updated ") << moved
            << (moved == 1 ? " package: " : " packages: ");
    bool first = true;
    auto appendPart = [&](Index count, const char* label)
    {
        if (count == 0)
            return;
        if (!first)
            builder << ", ";
        first = false;
        builder << count << " " << label;
    };
    appendPart(upgraded, "upgraded");
    appendPart(added, "added");
    appendPart(downgraded, "downgraded");
    appendPart(replaced, "replaced");
    appendPart(removed, "removed");
    builder << "; " << unchanged << " unchanged.\n";
    return builder.produceString();
}

String formatResolveReport(
    const Manifest& rootManifest,
    const LockFile* previous,
    const LockFile& next,
    const ResolveReport& report,
    bool dryRun,
    bool minimal)
{
    List<ResolveChange> changes;
    _collectChanges(rootManifest, previous, next, report, changes);

    StringBuilder builder;
    if (!minimal)
        builder << "Resolving dependencies...\n\n";

    auto appendHeadline = [&](const ResolveChange& change, bool goStyle)
    {
        builder << (minimal || goStyle ? "" : "  ");
        builder << _changeVerb(change.kind, dryRun) << " " << change.name << " "
                << (goStyle ? _goHeadlineRest(change) : _headlineRest(change)) << "\n";
    };

    if (minimal)
    {
        for (const auto& change : changes)
            appendHeadline(change, true);
        builder << "\n";
    }
    else
    {
        for (const auto& change : changes)
        {
            appendHeadline(change, false);
            _appendDetails(builder, rootManifest, previous, change);
            builder << "\n";
        }
    }

    builder << _countLine(changes, dryRun);
    return builder.produceString();
}

} // namespace PackageTool
} // namespace Slang
