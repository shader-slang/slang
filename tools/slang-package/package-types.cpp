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

bool VersionConstraint::matches(const SemanticVersion& version) const
{
    for (const auto& predicate : predicates)
    {
        switch (predicate.comparison)
        {
        case VersionComparison::Equal:
            if (version != predicate.version)
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
    return true;
}

bool matchesVersionPolicy(const String& constraintText, const SemanticVersion& version)
{
    VersionConstraint constraint;
    String error;
    SLANG_RELEASE_ASSERT(
        SLANG_SUCCEEDED(parseVersionConstraint(constraintText, constraint, error)));
    return constraint.matches(version);
}

SlangResult parseVersionConstraint(
    const UnownedStringSlice& text,
    VersionConstraint& outConstraint,
    String& outError)
{
    outConstraint.predicates.clear();
    List<UnownedStringSlice> terms;
    StringUtil::splitOnWhitespace(text, terms);
    if (terms.getCount() == 0)
    {
        outError = "A dependency version constraint cannot be empty.";
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
        outConstraint.predicates.add(predicate);
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

} // namespace PackageTool
} // namespace Slang
