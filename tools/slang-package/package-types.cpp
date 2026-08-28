// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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
