// unit-test-command-line-option-classification.cpp
//
// Enforces that `classifyCommandLineOption` classifies every `CompilerOptionName`. This has to be a
// test rather than a compiler-checked exhaustive switch because the project builds with
// `-Wno-switch`, so a missing case produces no warning.

#include "slang/slang-compiler-options.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(commandLineOptionClassificationIsExhaustive)
{
    // `CompilerOptionName` is a densely, contiguously numbered enum (0 .. CountOf-1), so iterating
    // the half-open integer range visits every enumerator exactly once; the assumption is checked
    // implicitly, since a hole would classify as `Unclassified` and fail below.
    const int countOf = (int)CompilerOptionName::CountOf;

    // Guard against a vacuous pass: if the range were empty the loop below would assert nothing.
    SLANG_CHECK(countOf > 0);

    for (int i = 0; i < countOf; ++i)
    {
        const bool classified = classifyCommandLineOption((CompilerOptionName)i) !=
                                CommandLineOptionClass::Unclassified;
        // Report a single result carrying the offending option index. `SLANG_CHECK_MSG` only
        // accepts a string literal (it concatenates it with the stringized condition), so call the
        // reporter directly to include the dynamic value.
        StringBuilder message;
        message << "CompilerOptionName value " << i
                << " is not classified in classifyCommandLineOption; add it to the appropriate "
                   "group.";
        getTestReporter()
            ->addResultWithLocation(classified, message.getBuffer(), __FILE__, __LINE__);
    }
}
