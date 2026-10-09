// package-types.cpp

#include "package-types.h"

#include "core/slang-semantic-version.h"
#include "core/slang-string-util.h"

namespace Slang
{
namespace PackageTool
{

SlangResult parseReleaseTag(const UnownedStringSlice& tag, PackageVersion& outVersion)
{
    if (tag.getLength() < 2 || tag[0] != 'v')
        return SLANG_FAIL;
    return PackageVersion::parse(tag.tail(1), outVersion);
}

bool acceptCanonicalReleaseTag(
    const UnownedStringSlice& tag,
    PackageVersion& outVersion,
    List<String>* outWarnings)
{
    if (SLANG_FAILED(parseReleaseTag(tag, outVersion)))
        return false;
    String canonical = String("v") + outVersion.format();
    if (String(tag) == canonical)
        return true;
    if (outWarnings)
    {
        String warning = String("Git tag '") + String(tag) +
                         "' is not the canonical release tag '" + canonical +
                         "' and will be ignored by the solver.";
        if (!outWarnings->contains(warning))
            outWarnings->add(warning);
    }
    return false;
}

SlangResult parseExactVersion(
    const UnownedStringSlice& text,
    PackageVersion& outVersion,
    String& outError)
{
    if (text.startsWith("v") || SLANG_FAILED(PackageVersion::parse(text, outVersion)))
    {
        outError = String("Expected an exact version without a 'v' prefix: ") + String(text);
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

bool sameExactRelease(const String& left, const String& right)
{
    if (left == right)
        return true;
    PackageVersion leftVersion;
    PackageVersion rightVersion;
    String error;
    if (SLANG_FAILED(parseExactVersion(left, leftVersion, error)) ||
        SLANG_FAILED(parseExactVersion(right, rightVersion, error)))
        return false;
    return leftVersion == rightVersion;
}

/// Build the exclusive upper bound by incrementing `incrementIndex` and dropping every component
/// after it. The new last component is one greater than the chosen component, so the bound does
/// not end in zero.
static SlangResult _exclusiveUpperBound(
    const PackageVersion& lower,
    Index incrementIndex,
    PackageVersion& outUpper,
    const UnownedStringSlice& term,
    String& outError)
{
    uint32_t component = lower.components[incrementIndex];
    if (component == 0xffffffffu)
    {
        outError = String("Version range has no representable upper bound: ") + String(term);
        return SLANG_FAIL;
    }
    outUpper.components.clear();
    for (Index i = 0; i < incrementIndex; ++i)
        outUpper.components.add(lower.components[i]);
    outUpper.components.add(component + 1);
    return SLANG_OK;
}

/// Choose the caret component: the leftmost non-zero component, or the last component when every
/// written component is zero.
///
/// Consider `^0.0`: both components are zero, so the upper bound increments the last written
/// component and the range is `>=0 <0.1`. The explicit `^0.0.0` is `>=0 <0.0.1`. Trailing zeros
/// are not a different release, but they still choose this bound, which is why `^0.0.0` is
/// narrower than `^0`. A later non-zero component moves the boundary there, so `^0.0.0.4` is
/// `>=0.0.0.4 <0.0.0.5`, while `^1.2.3.4` still increments the first component and is
/// `>=1.2.3.4 <2`.
static Index _caretIncrementIndex(const PackageVersion& version)
{
    for (Index i = 0; i < version.components.getCount(); ++i)
    {
        if (version.components[i] != 0)
            return i;
    }
    return version.components.getCount() - 1;
}

static void _addInclusiveExclusiveRange(
    VersionClause& clause,
    const PackageVersion& lower,
    const PackageVersion& upper)
{
    VersionPredicate lowerBound;
    lowerBound.comparison = VersionComparison::GreaterEqual;
    lowerBound.version = lower;
    VersionPredicate upperBound;
    upperBound.comparison = VersionComparison::Less;
    upperBound.version = upper;
    clause.predicates.add(lowerBound);
    clause.predicates.add(upperBound);
}

/// Expand a `^` or `~` term into the ordinary comparison predicates.
///
/// Tilde increments the second component when at least two are written, and the only component
/// otherwise. `~1.2.3.4` means `>=1.2.3.4 <1.3`, and `~1` means `>=1 <2`.
static SlangResult _parseCompatibilityRange(
    const UnownedStringSlice& term,
    VersionClause& clause,
    String& outError)
{
    PackageVersion written;
    if (SLANG_FAILED(PackageVersion::parsePreservingTrailingZeros(term.tail(1), written)))
    {
        outError = String("Invalid version in version constraint: ") + String(term);
        return SLANG_FAIL;
    }
    Index incrementIndex = 0;
    if (term[0] == '^')
        incrementIndex = _caretIncrementIndex(written);
    else if (written.components.getCount() > 1)
        incrementIndex = 1;
    PackageVersion upper;
    SLANG_RETURN_ON_FAIL(_exclusiveUpperBound(written, incrementIndex, upper, term, outError));
    PackageVersion lower;
    SLANG_RETURN_ON_FAIL(PackageVersion::parse(term.tail(1), lower));
    _addInclusiveExclusiveRange(clause, lower, upper);
    return SLANG_OK;
}

static bool _clauseMatches(const VersionClause& clause, const PackageVersion& version)
{
    for (const auto& predicate : clause.predicates)
    {
        switch (predicate.comparison)
        {
        case VersionComparison::Equal:
            if (version != predicate.version)
                return false;
            break;
        case VersionComparison::NotEqual:
            if (version == predicate.version)
                return false;
            break;
        case VersionComparison::Greater:
            if (!(version > predicate.version))
                return false;
            break;
        case VersionComparison::GreaterEqual:
            if (!(version >= predicate.version))
                return false;
            break;
        case VersionComparison::Less:
            if (!(version < predicate.version))
                return false;
            break;
        case VersionComparison::LessEqual:
            if (!(version <= predicate.version))
                return false;
            break;
        }
    }
    return clause.predicates.getCount() != 0;
}

bool VersionConstraint::matches(const PackageVersion& version) const
{
    for (const auto& clause : clauses)
    {
        if (_clauseMatches(clause, version))
            return true;
    }
    return false;
}

bool matchesVersionPolicy(const String& constraintText, const PackageVersion& version)
{
    VersionConstraint constraint;
    String error;
    SLANG_RELEASE_ASSERT(
        SLANG_SUCCEEDED(parseVersionConstraint(constraintText, constraint, error)));
    return constraint.matches(version);
}

static SlangResult _parseVersionClause(
    const UnownedStringSlice& text,
    const UnownedStringSlice& clauseText,
    VersionClause& outClause,
    String& outError)
{
    outClause.predicates.clear();
    List<UnownedStringSlice> terms;
    StringUtil::splitOnWhitespace(clauseText, terms);
    if (terms.getCount() == 0)
    {
        outError = "A dependency version constraint cannot contain an empty '||' clause.";
        return SLANG_FAIL;
    }

    for (auto term : terms)
    {
        if (term.startsWith("^") || term.startsWith("~"))
        {
            SLANG_RETURN_ON_FAIL(_parseCompatibilityRange(term, outClause, outError));
            continue;
        }

        VersionPredicate predicate;
        UnownedStringSlice versionText;
        if (term.startsWith(">="))
        {
            predicate.comparison = VersionComparison::GreaterEqual;
            versionText = term.tail(2);
        }
        else if (term.startsWith("<="))
        {
            predicate.comparison = VersionComparison::LessEqual;
            versionText = term.tail(2);
        }
        else if (term.startsWith("!="))
        {
            predicate.comparison = VersionComparison::NotEqual;
            versionText = term.tail(2);
        }
        else if (term.startsWith(">"))
        {
            predicate.comparison = VersionComparison::Greater;
            versionText = term.tail(1);
        }
        else if (term.startsWith("<"))
        {
            predicate.comparison = VersionComparison::Less;
            versionText = term.tail(1);
        }
        else if (terms.getCount() == 1)
        {
            predicate.comparison = VersionComparison::Equal;
            versionText = term;
        }
        else
        {
            outError = String("Invalid dependency version constraint: ") + String(text);
            return SLANG_FAIL;
        }

        if (versionText.startsWith("v") ||
            SLANG_FAILED(PackageVersion::parse(versionText, predicate.version)))
        {
            outError = String("Invalid version in version constraint: ") + String(term);
            return SLANG_FAIL;
        }
        outClause.predicates.add(predicate);
    }
    return SLANG_OK;
}

SlangResult parseVersionConstraint(
    const UnownedStringSlice& text,
    VersionConstraint& outConstraint,
    String& outError)
{
    outConstraint.clauses.clear();
    UnownedStringSlice trimmedText = text.trim();
    if (trimmedText.getLength() == 0)
    {
        outError = "A dependency version constraint cannot be empty.";
        return SLANG_FAIL;
    }
    if (trimmedText.startsWith("||") || trimmedText.endsWith("||"))
    {
        outError = "A dependency version constraint cannot contain an empty '||' clause.";
        return SLANG_FAIL;
    }

    List<UnownedStringSlice> clauseTexts;
    StringUtil::split(trimmedText, UnownedStringSlice("||"), clauseTexts);

    for (auto clauseText : clauseTexts)
    {
        UnownedStringSlice trimmed = clauseText.trim();
        if (trimmed.getLength() == 0)
        {
            outError = "A dependency version constraint cannot contain an empty '||' clause.";
            return SLANG_FAIL;
        }

        VersionClause clause;
        SLANG_RETURN_ON_FAIL(_parseVersionClause(text, trimmed, clause, outError));
        outConstraint.clauses.add(clause);
    }
    return SLANG_OK;
}

SlangResult parseDependencyConstraint(
    const Dependency& dependency,
    VersionConstraint& outConstraint,
    String& outError)
{
    if (dependency.version.getLength() == 0)
    {
        outError = String("Dependency '") + dependency.name + "' requires 'version'.";
        return SLANG_FAIL;
    }
    return parseVersionConstraint(dependency.version, outConstraint, outError);
}

#ifndef SLANG_PACKAGE_COMPILER_VERSION
#define SLANG_PACKAGE_COMPILER_VERSION "unknown"
#endif

void addSlangToolchainConstraint(const Manifest& manifest, List<ToolchainConstraint>& ioConstraints)
{
    if (!manifest.slangToolchainConstraint.getLength())
        return;
    ToolchainConstraint constraint;
    constraint.packageName = manifest.name;
    constraint.constraint = manifest.slangToolchainConstraint;
    ioConstraints.add(constraint);
}

static UnownedStringSlice _numericToolchainPrefix(const UnownedStringSlice& text)
{
    UnownedStringSlice result = text;
    if (result.startsWith("v") || result.startsWith("V"))
        result = result.tail(1);
    Index dash = result.indexOf('-');
    if (dash >= 0)
        result = result.head(dash);
    Index plus = result.indexOf('+');
    if (plus >= 0)
        result = result.head(plus);
    return result;
}

SlangResult getInstalledSlangToolchainVersion(
    PackageVersion& outVersion,
    String& outExactText,
    String& outError)
{
    UnownedStringSlice numeric =
        _numericToolchainPrefix(UnownedStringSlice(SLANG_PACKAGE_COMPILER_VERSION));
    SemanticVersion parsed;
    if (SLANG_FAILED(SemanticVersion::parse(numeric, parsed)))
    {
        outError = String("Cannot parse the installed slang-toolchain version: ") +
                   SLANG_PACKAGE_COMPILER_VERSION;
        return SLANG_FAIL;
    }
    outVersion = PackageVersion(uint32_t(parsed.m_major), uint32_t(parsed.m_minor), parsed.m_patch);
    outExactText = formatExactVersion(outVersion);
    return SLANG_OK;
}

SlangResult selectSlangToolchain(const List<ToolchainConstraint>& constraints, String& outError)
{
    if (constraints.getCount() == 0)
        return SLANG_OK;
    PackageVersion installed;
    String installedText;
    SLANG_RETURN_ON_FAIL(getInstalledSlangToolchainVersion(installed, installedText, outError));
    for (const auto& constraint : constraints)
    {
        VersionConstraint parsed;
        SLANG_RETURN_ON_FAIL(parseVersionConstraint(constraint.constraint, parsed, outError));
        if (!parsed.matches(installed))
        {
            outError = String("Installed slang-toolchain ") + installedText +
                       " does not satisfy '" + constraint.constraint + "' required by package '" +
                       constraint.packageName + "'.";
            return SLANG_FAIL;
        }
    }
    return SLANG_OK;
}

static bool _hasMatchingWorkspaceExclusion(
    const List<Exclusion>& exclusions,
    const Exclusion& exclusion)
{
    for (const auto& existing : exclusions)
    {
        if (existing.packageName == exclusion.packageName && existing.version == exclusion.version)
            return true;
    }
    return false;
}

void addUnadoptedWorkspaceExclusionWarnings(
    const Manifest& rootManifest,
    const String& packageName,
    const Manifest& packageManifest,
    List<String>* ioWarnings)
{
    if (!ioWarnings)
        return;
    for (const auto& exclusion : packageManifest.workspace.exclusions)
    {
        if (_hasMatchingWorkspaceExclusion(rootManifest.workspace.exclusions, exclusion))
            continue;
        String warning = String("Package '") + packageName + "' declares workspace.excludes for '" +
                         exclusion.packageName + "' " + exclusion.version +
                         ", which this workspace does not exclude. Nested workspace.excludes "
                         "are ignored; copy the entry into this package's workspace.excludes if "
                         "the project should skip that release.";
        if (!ioWarnings->contains(warning))
            ioWarnings->add(warning);
    }
}

} // namespace PackageTool
} // namespace Slang
