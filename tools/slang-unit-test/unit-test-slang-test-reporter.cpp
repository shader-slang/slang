#include "slang-test/options.h"
#include "slang-test/test-reporter.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A sub-reporter (the per-worker reporter used by parallel runs) must receive the same
// Options-derived display configuration as the main reporter, so that verbosity, dump-on-failure,
// hide-ignored, output mode, and the expected-failure list all take effect on every worker.
SLANG_UNIT_TEST(slangTestReporterInitFromOptions)
{
    Options options;
    options.outputMode = TestOutputMode::TeamCity;
    options.verbosity = VerbosityLevel::Failure;
    options.dumpOutputOnFailure = true;
    options.hideIgnored = true;
    options.expectedFailureList.add(String("tests/some/expected-failure.slang"));

    // isSubReporter=true selects the worker/parallel configuration path.
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
