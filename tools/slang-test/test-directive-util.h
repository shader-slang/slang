#ifndef TEST_DIRECTIVE_UTIL_H
#define TEST_DIRECTIVE_UTIL_H

#include "core/slang-string.h"

namespace Slang
{

// Utilities over the directive-comment vocabulary that slang-test's file scanner recognizes —
// the commands of `//TEST:`, `//DIAGNOSTIC_TEST:`, `//TEST_CATEGORY:`, `//TEST_IGNORE_FILE` and
// their `DISABLE_`-prefixed forms — used to report a mistyped directive instead of silently
// skipping it.
struct TestDirectiveUtil
{
    /// Returns the directive spelling that `command` is a near miss of — one character
    /// inserted, deleted, or transposed relative to a spelling the scanner recognizes — or
    /// an empty slice when there is no such spelling. A single substitution does not count:
    /// a word one substitution away from a directive name is usually a different real word
    /// (a diagnostic test's annotation label, such as `NEST` for `TEST`), not a typo of it.
    static UnownedStringSlice findNearMissDirectiveCommand(const UnownedStringSlice& command);

    /// Returns the spelling to suggest for a near miss found by `findNearMissDirectiveCommand`.
    /// `hasDisablePrefix` says the scanner stripped an exact `DISABLE_` prefix from the command
    /// before the lookup, so the suggestion re-attaches it — except for `TEST_IGNORE_FILE`,
    /// whose `DISABLE_` form the scanner does not dispatch on at all (it compares
    /// `TEST_IGNORE_FILE` before the strip), so suggesting it would tell the author to write a
    /// line the harness silently ignores.
    static String getNearMissDirectiveSuggestion(
        const UnownedStringSlice& nearMiss,
        bool hasDisablePrefix);
};

} // namespace Slang

#endif // TEST_DIRECTIVE_UTIL_H
