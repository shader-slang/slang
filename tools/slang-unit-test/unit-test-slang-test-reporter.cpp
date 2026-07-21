#include "slang-test/options.h"
#include "slang-test/test-reporter.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression guard for shader-slang/slang#12177: a per-worker reporter in a parallel run must pick
// up verbosity/dump-on-failure/hide-ignored/output-mode from the parsed Options, not silently keep
// the constructor defaults. The bug was that runTestsInParallel's worker reporters only called
// init() and never received these settings, so passing tests printed under `-v failure`. Pinning
// the propagation here means dropping any of the assignments in init() fails a test instead of
// re-introducing the bug.
SLANG_UNIT_TEST(slangTestReporterInitFromOptions)
{
    Options options;
    options.outputMode = TestOutputMode::TeamCity;
    options.verbosity = VerbosityLevel::Failure;
    options.dumpOutputOnFailure = true;
    options.hideIgnored = true;
    options.expectedFailureList.add(String("tests/some/expected-failure.slang"));

    // isSubReporter=true exercises the worker/parallel path, which is where the settings were lost.
    TestReporter reporter;
    SLANG_CHECK(SLANG_SUCCEEDED(reporter.init(options, /*isSubReporter*/ true)));

    SLANG_CHECK(reporter.m_outputMode == TestOutputMode::TeamCity);
    SLANG_CHECK(reporter.m_verbosity == VerbosityLevel::Failure);
    SLANG_CHECK(reporter.m_dumpOutputOnFailure == true);
    SLANG_CHECK(reporter.m_hideIgnored == true);
    SLANG_CHECK(reporter.m_isSubReporter == true);
    SLANG_CHECK(
        reporter.m_expectedFailureList.contains(String("tests/some/expected-failure.slang")));
}
