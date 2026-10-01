// package-types.cpp

#include "package-types.h"

#include "core/slang-string-util.h"

namespace Slang
{
namespace PackageTool
{

static bool _isDecimalVersion(const UnownedStringSlice& text)
{
    Index dotCount = 0;
    bool componentHasDigit = false;
    for (auto c : text)
    {
        if (c == '.')
        {
            if (!componentHasDigit)
                return false;
            componentHasDigit = false;
            ++dotCount;
        }
        else if (c >= '0' && c <= '9')
        {
            componentHasDigit = true;
        }
        else
        {
            return false;
        }
    }
    return componentHasDigit && dotCount == 2;
}

static SlangResult _parseVersion(const UnownedStringSlice& text, SemanticVersion& outVersion)
{
    if (!_isDecimalVersion(text))
        return SLANG_FAIL;
    return SemanticVersion::parse(text, outVersion);
}

SlangResult parseReleaseTag(const UnownedStringSlice& tag, SemanticVersion& outVersion)
{
    if (tag.getLength() < 2 || tag[0] != 'v')
        return SLANG_FAIL;
    return _parseVersion(tag.tail(1), outVersion);
}

SlangResult parseExactVersion(
    const UnownedStringSlice& text,
    SemanticVersion& outVersion,
    String& outError)
{
    if (text.startsWith("v") || SLANG_FAILED(_parseVersion(text, outVersion)))
    {
        outError =
            String("Expected an exact semantic version without a 'v' prefix: ") + String(text);
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/// The same limits `SemanticVersion::parse` enforces: major and minor fit in 16 bits, and patch
/// fits in a signed 31-bit value.
static const int kMaxMajorMinorVersion = 0xffff;
static const int kMaxPatchVersion = 0x7fffffff;

struct PartialVersion
{
    int componentCount = 0;
    int major = 0;
    int minor = 0;
    int patch = 0;
};

/// Parse a decimal version component that fits in `maxValue`.
static SlangResult _parseVersionComponent(
    const UnownedStringSlice& text,
    int maxValue,
    int& outValue)
{
    if (text.getLength() == 0)
        return SLANG_FAIL;
    for (auto character : text)
    {
        if (character < '0' || character > '9')
            return SLANG_FAIL;
    }
    Int value = 0;
    if (SLANG_FAILED(StringUtil::parseInt(text, value)) || value < 0 || value > maxValue)
        return SLANG_FAIL;
    outValue = int(value);
    return SLANG_OK;
}

/// Parse one, two, or three decimal components. Omitted components stay zero.
///
/// `1`, `1.2`, and `1.2.3` are accepted. A fourth component, an empty component, or a non-digit
/// is rejected because package versions are a major.minor.patch triple, and a missing component
/// is shorthand for that triple rather than another version length.
static SlangResult _parsePartialVersion(const UnownedStringSlice& text, PartialVersion& outVersion)
{
    List<UnownedStringSlice> components;
    StringUtil::split(text, '.', components);
    if (components.getCount() < 1 || components.getCount() > 3)
        return SLANG_FAIL;
    outVersion = PartialVersion();
    outVersion.componentCount = int(components.getCount());
    SLANG_RETURN_ON_FAIL(
        _parseVersionComponent(components[0], kMaxMajorMinorVersion, outVersion.major));
    if (components.getCount() >= 2)
    {
        SLANG_RETURN_ON_FAIL(
            _parseVersionComponent(components[1], kMaxMajorMinorVersion, outVersion.minor));
    }
    if (components.getCount() == 3)
    {
        SLANG_RETURN_ON_FAIL(
            _parseVersionComponent(components[2], kMaxPatchVersion, outVersion.patch));
    }
    return SLANG_OK;
}

static bool _incrementBoundedComponent(int& component, int maxValue)
{
    if (component >= maxValue)
        return false;
    ++component;
    return true;
}

/// Expand a caret range into an inclusive lower bound and an exclusive upper bound.
///
/// When all three components are written, the upper bound advances the left-most non-zero
/// component. An omitted component stays flexible even when the written components are zero.
/// Consider `^0.0`: it means `>=0.0.0 <0.1.0`, while the explicit `^0.0.0` means
/// `>=0.0.0 <0.0.1`. `^0` means `>=0.0.0 <1.0.0`.
static SlangResult _caretUpperBound(
    const PartialVersion& partial,
    SemanticVersion& outUpper,
    const UnownedStringSlice& term,
    String& outError)
{
    int major = partial.major;
    int minor = partial.minor;
    int patch = partial.patch;
    bool representable = false;
    if (partial.componentCount == 1 || partial.major > 0)
    {
        representable = _incrementBoundedComponent(major, kMaxMajorMinorVersion);
        minor = 0;
        patch = 0;
    }
    else if (partial.componentCount == 2 || partial.minor > 0)
    {
        representable = _incrementBoundedComponent(minor, kMaxMajorMinorVersion);
        patch = 0;
    }
    else
    {
        representable = _incrementBoundedComponent(patch, kMaxPatchVersion);
    }
    if (!representable)
    {
        outError = String("Version range has no representable upper bound: ") + String(term);
        return SLANG_FAIL;
    }
    outUpper = SemanticVersion(major, minor, patch);
    return SLANG_OK;
}

/// Expand a tilde range into an inclusive lower bound and an exclusive upper bound.
///
/// Three components allow later patches: `~1.2.3` means `>=1.2.3 <1.3.0`. Two components do the
/// same from patch zero: `~1.2` means `>=1.2.0 <1.3.0`. One component allows any later minor:
/// `~1` means `>=1.0.0 <2.0.0`.
static SlangResult _tildeUpperBound(
    const PartialVersion& partial,
    SemanticVersion& outUpper,
    const UnownedStringSlice& term,
    String& outError)
{
    int major = partial.major;
    int minor = partial.minor;
    int patch = 0;
    bool representable = false;
    if (partial.componentCount == 1)
    {
        representable = _incrementBoundedComponent(major, kMaxMajorMinorVersion);
        minor = 0;
    }
    else
    {
        representable = _incrementBoundedComponent(minor, kMaxMajorMinorVersion);
    }
    if (!representable)
    {
        outError = String("Version range has no representable upper bound: ") + String(term);
        return SLANG_FAIL;
    }
    outUpper = SemanticVersion(major, minor, patch);
    return SLANG_OK;
}

static void _addInclusiveExclusiveRange(
    VersionClause& clause,
    const PartialVersion& lower,
    const SemanticVersion& upper)
{
    VersionPredicate lowerBound;
    lowerBound.comparison = VersionComparison::GreaterEqual;
    lowerBound.version = SemanticVersion(lower.major, lower.minor, lower.patch);
    VersionPredicate upperBound;
    upperBound.comparison = VersionComparison::Less;
    upperBound.version = upper;
    clause.predicates.add(lowerBound);
    clause.predicates.add(upperBound);
}

/// Expand a `^` or `~` term into the ordinary comparison predicates.
static SlangResult _parseCompatibilityRange(
    const UnownedStringSlice& term,
    VersionClause& clause,
    String& outError)
{
    PartialVersion partial;
    if (SLANG_FAILED(_parsePartialVersion(term.tail(1), partial)))
    {
        outError = String("Invalid semantic version in version constraint: ") + String(term);
        return SLANG_FAIL;
    }
    SemanticVersion upper;
    if (term[0] == '^')
    {
        SLANG_RETURN_ON_FAIL(_caretUpperBound(partial, upper, term, outError));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(_tildeUpperBound(partial, upper, term, outError));
    }
    _addInclusiveExclusiveRange(clause, partial, upper);
    return SLANG_OK;
}

static bool _clauseMatches(const VersionClause& clause, const SemanticVersion& version)
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

bool VersionConstraint::matches(const SemanticVersion& version) const
{
    for (const auto& clause : clauses)
    {
        if (_clauseMatches(clause, version))
            return true;
    }
    return false;
}

bool matchesVersionPolicy(const String& constraintText, const SemanticVersion& version)
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
            SLANG_FAILED(_parseVersion(versionText, predicate.version)))
        {
            outError = String("Invalid semantic version in version constraint: ") + String(term);
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
    SemanticVersion& outVersion,
    String& outExactText,
    String& outError)
{
    UnownedStringSlice numeric =
        _numericToolchainPrefix(UnownedStringSlice(SLANG_PACKAGE_COMPILER_VERSION));
    if (SLANG_FAILED(SemanticVersion::parse(numeric, outVersion)))
    {
        outError = String("Cannot parse the installed slang-toolchain version: ") +
                   SLANG_PACKAGE_COMPILER_VERSION;
        return SLANG_FAIL;
    }
    outExactText = formatExactVersion(outVersion);
    return SLANG_OK;
}

SlangResult selectSlangToolchain(const List<ToolchainConstraint>& constraints, String& outError)
{
    if (constraints.getCount() == 0)
        return SLANG_OK;
    SemanticVersion installed;
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
