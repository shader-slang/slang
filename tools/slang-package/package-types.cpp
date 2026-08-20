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
    UnownedStringSlice versionText = text;
    if (versionText.getLength() > 0 && versionText[0] == 'v')
        versionText = versionText.tail(1);
    if (!_isDecimalVersion(versionText))
        return SLANG_FAIL;
    return SemanticVersion::parse(versionText, outVersion);
}

SlangResult parseReleaseTag(const UnownedStringSlice& tag, SemanticVersion& outVersion)
{
    if (tag.getLength() < 2 || tag[0] != 'v')
        return SLANG_FAIL;
    return _parseVersion(tag, outVersion);
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
        outError = "A dependency tag constraint cannot be empty.";
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
        else if (terms.getCount() == 1 && term.startsWith("v"))
        {
            predicate.comparison = VersionComparison::Equal;
            versionText = term;
        }
        else
        {
            outError = String("Invalid dependency tag constraint: ") + String(text);
            return SLANG_FAIL;
        }

        if (!versionText.startsWith("v") ||
            SLANG_FAILED(_parseVersion(versionText, predicate.version)))
        {
            outError = String("Invalid semantic version in tag constraint: ") + String(term);
            return SLANG_FAIL;
        }
        outConstraint.predicates.add(predicate);
    }
    return SLANG_OK;
}

} // namespace PackageTool
} // namespace Slang
