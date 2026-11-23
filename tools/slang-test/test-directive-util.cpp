// test-directive-util.cpp

#include "test-directive-util.h"

#include "core/slang-string-util.h"

namespace Slang
{

// The directive spellings that the file scanner in `_gatherTestsForFile`
// (tools/slang-test/slang-test-main.cpp) recognizes, in both their plain and their
// `DISABLE_`-prefixed forms.
//
// The scanner strips an exact `DISABLE_` prefix before it dispatches, so a stripped command
// can never match a prefixed spelling: the prefixed entries exist to catch a typo in the
// prefix itself. `DISABLED_TEST` does not start with `DISABLE_`, so it survives stripping
// and lands one insertion away from the `DISABLE_TEST` entry, while `DISABLE_TST` strips to
// `TST` and matches the plain `TEST` entry. The list follows the dispatch: a spelling here
// that the scanner does not recognize would flag legitimate lines as errors, and a missing
// one would silently skip its typos, which is the failure this check exists to catch.
// `DISABLE_TEST_CATEGORY` is listed because the scanner's unconditional prefix strip makes it
// reach the `TEST_CATEGORY` branch; `DISABLE_TEST_IGNORE_FILE` is not, because the scanner
// compares `TEST_IGNORE_FILE` before the strip, so the prefixed form is not a directive it
// dispatches on at all.
static const UnownedStringSlice kTestDirectiveSpellings[] = {
    UnownedStringSlice::fromLiteral("TEST"),
    UnownedStringSlice::fromLiteral("DIAGNOSTIC_TEST"),
    UnownedStringSlice::fromLiteral("TEST_CATEGORY"),
    UnownedStringSlice::fromLiteral("TEST_IGNORE_FILE"),
    UnownedStringSlice::fromLiteral("DISABLE_TEST"),
    UnownedStringSlice::fromLiteral("DISABLE_DIAGNOSTIC_TEST"),
    UnownedStringSlice::fromLiteral("DISABLE_TEST_CATEGORY"),
};

/* static */ UnownedStringSlice TestDirectiveUtil::findNearMissDirectiveCommand(
    const UnownedStringSlice& command)
{
    for (const UnownedStringSlice spelling : kTestDirectiveSpellings)
    {
        if (StringUtil::isNearMissSpelling(command, spelling))
        {
            return spelling;
        }
    }
    return UnownedStringSlice();
}

/* static */ String TestDirectiveUtil::getNearMissDirectiveSuggestion(
    const UnownedStringSlice& nearMiss,
    bool hasDisablePrefix)
{
    StringBuilder suggestion;
    if (hasDisablePrefix && nearMiss != UnownedStringSlice::fromLiteral("TEST_IGNORE_FILE"))
    {
        suggestion << "DISABLE_";
    }
    suggestion << nearMiss;
    return suggestion.produceString();
}

} // namespace Slang
