// unit-test-harness-selfcheck.cpp
//
// Deliberately failing checks, used to prove the harness reports failures.
//
// Every other test in this suite is written to pass, so the driver's failure side
// never runs during an ordinary run: the `Fail` branch of `recordResult`, the
// `failedTestCount` tally, the `FAIL` line, and the non-zero exit are all only ever
// taken on the passing side. A regression that made the driver always exit 0, or
// that scored a failed `SLANG_CHECK` as a pass, would leave CI green while the suite
// had silently stopped catching anything -- the one contract a test-infrastructure
// change cannot afford to leave untested.
//
// These ignore themselves unless `SLANG_STATIC_UNIT_TEST_SELFCHECK` is set, so an
// ordinary run stays green and reports them as ignored. CI runs the binary again with
// that variable set and the `harnessSelfCheck` filter, and requires the specific
// outcome each one is named for; see the "Run slang-static-unit-test" step in
// ci-slang-static-unit-test.yml.
//
// Order matters here. Tests run in registration order, which within one file is
// declaration order, so `ContinuesAfterAThrow` below is registered last on purpose:
// it is what proves the driver carried on rather than dying at the throwing test.

#include "unit-test/slang-unit-test.h"

#include <cstdlib>
#include <stdexcept>

using namespace Slang;

namespace
{
/// True when CI has armed the self-checks. They ignore themselves otherwise, which
/// keeps an ordinary run green while still exercising `SLANG_IGNORE_TEST` and the
/// driver's `Ignored` tally.
bool selfCheckArmed()
{
    return std::getenv("SLANG_STATIC_UNIT_TEST_SELFCHECK") != nullptr;
}
} // namespace

// A failed `SLANG_CHECK` must be reported as a failure and make the run exit non-zero.
SLANG_UNIT_TEST(harnessSelfCheckReportsAFailingCheck)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    // Deliberate. This is one of two assertions here that are meant to fail.
    SLANG_CHECK(false);
}

// An exception escaping a test body must be converted into a failure rather than
// terminating the process. `SLANG_UNIT_TEST` catches only `AbortTestException`, so a
// `SLANG_ASSERT` firing inside a compiler entry point arrives here as an ordinary
// exception, which is the case the driver's `catch` exists for.
SLANG_UNIT_TEST(harnessSelfCheckSurvivesAThrowingTest)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    // Deliberate, and thrown as a `std::exception` so the driver has a `what()` to
    // print -- which is what makes such a failure diagnosable from a CI log.
    throw std::runtime_error("deliberate self-check exception");
}

// ...and the run must continue past it. This test passing is the assertion: it is
// registered after the throwing test, so if the throw had torn the process down or
// escaped the per-test handler, this line would never be reached and CI would not see
// its `ok`. That carry-on property is the half of the handler's contract that the
// non-zero exit alone does not pin.
SLANG_UNIT_TEST(harnessSelfCheckContinuesAfterAThrow)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK(true);
}
