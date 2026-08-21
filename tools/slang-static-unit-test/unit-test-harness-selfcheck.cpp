// unit-test-harness-selfcheck.cpp
//
// A deliberately failing check, used to prove that the harness reports failures.
//
// Every other test in this suite is written to pass, so the failure side of the
// driver never runs during an ordinary run: the `Fail` branch of `recordResult`,
// the `failedTestCount` tally, the `FAIL` line, and the non-zero exit are all
// only ever taken on the passing side. A regression that made the driver always
// exit 0, or that classified a failed `SLANG_CHECK` as a pass, would leave CI
// green while the suite had silently stopped catching anything -- which is the
// one contract a test-infrastructure change cannot afford to leave untested.
//
// The check ignores itself unless `SLANG_STATIC_UNIT_TEST_SELFCHECK` is set in
// the environment, so an ordinary run stays green and reports it as ignored. CI
// runs the binary a second time with that variable set and requires a non-zero
// exit and a `FAIL` line; see the "Run slang-static-unit-test" step in
// ci-slang-static-unit-test.yml.

#include "unit-test/slang-unit-test.h"

#include <cstdlib>

using namespace Slang;

SLANG_UNIT_TEST(harnessReportsAFailingCheck)
{
    if (!std::getenv("SLANG_STATIC_UNIT_TEST_SELFCHECK"))
    {
        // Also the only use of `SLANG_IGNORE_TEST` in the suite, so it keeps the
        // driver's `Ignored` classification and the "N ignored" summary honest.
        SLANG_IGNORE_TEST;
    }

    // Deliberate. This is the only assertion here that is meant to fail.
    SLANG_CHECK(false);
}
