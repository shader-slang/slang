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
// declaration order, and `ContinuesAfterAThrow` must be declared *after* the tests that
// throw: reaching it is what proves the driver carried on rather than dying at one of
// them. It is written last so that adding a self-check in the ordinary place -- at the
// end of the file -- is what breaks the requirement, which is the mistake this note
// exists to catch. A new self-check belongs above it.

#include "core/slang-exception.h"
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

    // Deliberate. Every self-check in this file but the last one fails on purpose.
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

// A `Slang::Exception` must be reported with its message. This is the case the
// driver's handler exists for and the one a `std::exception`-only catch would miss:
// `SLANG_ASSERT` routes through `handleSignal`, which throws `InternalError` and
// friends -- all derived from `Slang::Exception`, which does not derive from
// `std::exception` and carries its text in `Message` rather than `what()`.
//
// Thrown directly rather than by firing an assert, because what `SLANG_ASSERT` does
// depends on the `SLANG_ASSERT` environment variable (see the table in CLAUDE.md) and
// a self-check should not vary with it. The type thrown is the type an assert throws.
SLANG_UNIT_TEST(harnessSelfCheckReportsASlangExceptionMessage)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    throw Slang::InternalError("deliberate self-check slang exception");
}

// A failure reported through `message()` rather than `addResult` must still fail the
// test. The driver classifies `RunError` and `TestFailure` as failures, which is a
// deliberate choice nothing else in the suite drives.
SLANG_UNIT_TEST(harnessSelfCheckFailsOnAReportedRunError)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    getTestReporter()->message(TestMessageType::RunError, "deliberate self-check run error");
}

// A `TestFailure` message must fail the test too, not only `RunError`. The driver
// classifies both as failures -- the enum defines `TestFailure` as describing how a
// failure took place -- and that choice deserves a driver of its own rather than being
// covered by inspection.
SLANG_UNIT_TEST(harnessSelfCheckFailsOnAReportedTestFailure)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    getTestReporter()->message(TestMessageType::TestFailure, "deliberate self-check test failure");
}

// ...and the run must continue past the failures above. This test passing is the
// assertion: it is registered last, so if a throw had torn the process down or escaped
// the per-test handler, this line would never be reached and CI would not see its `ok`.
// That carry-on property is the half of the handler's contract that the non-zero exit
// alone does not pin.
SLANG_UNIT_TEST(harnessSelfCheckContinuesAfterAThrow)
{
    if (!selfCheckArmed())
    {
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK(true);
}
