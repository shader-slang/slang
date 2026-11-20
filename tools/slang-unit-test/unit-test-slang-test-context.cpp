#include "core/slang-io.h"
#include "core/slang-list.h"
#include "core/slang-string-util.h"
#include "core/slang-writer.h"
#include "slang-test/options.h"
#include "slang-test/test-context.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Every per-thread slot of a freshly sized TestContext must start out empty.
//
// A non-null requirements slot switches the test runner into "collect requirements" mode, where
// runTest() only records what a test needs and returns Pass without running it. The retry pass
// runs on the main thread, whose thread index is left at the last worker it spawned a test server
// for, so it reads a slot that no worker may ever have touched. When that slot held uninitialized
// memory, a failing test's retry either crashed slang-test or reported the failure as a pass.
SLANG_UNIT_TEST(slangTestContextThreadSlotsStartEmpty, NoRenderApi)
{
    static const int kThreadCount = 8;

    TestContext context;

    // List::setCount does not initialize pointer elements, so an uninitialized slot is only
    // non-null if the allocator hands back dirty memory. We recycle a few pointer arrays filled
    // with a non-null pattern first so that the check does not depend on the allocator's debug
    // fill, which Release builds do not have.
    {
        TestRequirements* const dirty = reinterpret_cast<TestRequirements*>(uintptr_t(1));
        List<TestRequirements*> recycled[4];
        for (auto& list : recycled)
        {
            list.setCount(kThreadCount);
            for (auto& element : list)
                element = dirty;
        }
        for (auto& list : recycled)
            list.clearAndDeallocate();
    }

    context.setMaxTestRunnerThreadCount(kThreadCount);

    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
    {
        context.setThreadIndex(threadIndex);
        SLANG_CHECK(context.getTestRequirements() == nullptr);
        SLANG_CHECK(!context.isCollectingRequirements());
        SLANG_CHECK(context.getTestReporter() == nullptr);
    }

    // The thread index is thread-local state of the unit-test runner thread, so restore it.
    context.setThreadIndex(0);
}

// A test runs when all the APIs that it needs are enabled, and a test that needs no API runs unless
// `-compile-test false` is set.
SLANG_UNIT_TEST(slangTestContextRunsTestsByEnabledApis, NoRenderApi)
{
    TestContext context;
    context.options.enabledApis = RenderApiFlag::CPU | RenderApiFlag::LLVM;

    SLANG_CHECK(context.canRunTestWithRenderApiFlags(0));
    SLANG_CHECK(context.canRunTestWithRenderApiFlags(RenderApiFlag::CPU));
    SLANG_CHECK(!context.canRunTestWithRenderApiFlags(RenderApiFlag::Vulkan));
    SLANG_CHECK(!context.canRunTestWithRenderApiFlags(RenderApiFlag::CPU | RenderApiFlag::Vulkan));
}

// `-compile-test` is true by default. With `-compile-test false`, a test that needs no API is a
// compile test and does not run, unless the caller says that GPU tests need it: a COMPILE line that
// prepares the input of a line with an API, or a line that uses a tool that only the GPU hosts
// have. The tests that need an API are not affected.
SLANG_UNIT_TEST(slangTestContextCompileTestFalseKeepsOnlyWhatGpuTestsNeed, NoRenderApi)
{
    TestContext context;
    SLANG_CHECK(context.options.compileTest);

    context.options.compileTest = false;
    context.options.enabledApis = RenderApiFlag::Vulkan | RenderApiFlag::CUDA;

    SLANG_CHECK(!context.canRunTestWithRenderApiFlags(0));
    SLANG_CHECK(context.canRunTestWithRenderApiFlags(0, /* isNeededByGpuTests */ true));
    SLANG_CHECK(context.canRunTestWithRenderApiFlags(RenderApiFlag::Vulkan));
    SLANG_CHECK(!context.canRunTestWithRenderApiFlags(RenderApiFlag::D3D12));
    SLANG_CHECK(!context.canRunTestWithRenderApiFlags(RenderApiFlag::D3D12, true));
}

// A unit test can use several APIs one after another, so it runs when at least one of them is
// enabled. With `-compile-test false`, a unit test that uses no API is a compile test and does not
// run.
SLANG_UNIT_TEST(slangTestContextRunsUnitTestsByUsedApis, NoRenderApi)
{
    TestContext context;
    context.options.enabledApis = RenderApiFlag::CPU | RenderApiFlag::LLVM;

    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(0));
    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(RenderApiFlag::CPU));
    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(RenderApiFlag::CPU | RenderApiFlag::Vulkan));
    SLANG_CHECK(!context.canRunUnitTestUsingRenderApis(RenderApiFlag::Vulkan));
    SLANG_CHECK(!context.canRunUnitTestUsingRenderApis(RenderApiFlag::D3D12 | RenderApiFlag::CUDA));

    context.options.compileTest = false;
    context.options.enabledApis = RenderApiFlag::Vulkan | RenderApiFlag::CUDA;

    SLANG_CHECK(!context.canRunUnitTestUsingRenderApis(0));
    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(RenderApiFlag::Vulkan));
    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(RenderApiFlag::D3D12 | RenderApiFlag::CUDA));
    SLANG_CHECK(!context.canRunUnitTestUsingRenderApis(RenderApiFlag::D3D12));
    // A test that declares all the APIs runs in every job.
    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(RenderApiFlag::AllOf));

    context.options.enabledApis = RenderApiFlag::CPU | RenderApiFlag::LLVM;
    SLANG_CHECK(context.canRunUnitTestUsingRenderApis(RenderApiFlag::AllOf));
}

extern "C" IUnitTestModule* slangUnitTestGetModule();

// The test below declares APIs only to check that the registry keeps them. The declaration does not
// make slang-test skip the test in a job that has no GPU, because the test checks nothing there.
SLANG_UNIT_TEST(slangUnitTestRegistryProbe, Vulkan | CUDA)
{
    SLANG_CHECK(true);
}

// A unit test declares the APIs that it uses with SLANG_UNIT_TEST, and the registry
// reports them to slang-test. A test defined with SLANG_UNIT_TEST declares none.
SLANG_UNIT_TEST(slangUnitTestRegistryReportsUsedApis, NoRenderApi)
{
    IUnitTestModule* module = slangUnitTestGetModule();
    SLANG_CHECK_ABORT(module != nullptr);

    bool foundProbe = false;
    bool foundSelf = false;
    for (SlangInt i = 0; i < module->getTestCount(); ++i)
    {
        const UnownedStringSlice name(module->getTestName(i));
        if (name == "slangUnitTestRegistryProbe")
        {
            foundProbe = true;
            SLANG_CHECK(
                module->getTestUsedApis(i) == (RenderApiFlag::Vulkan | RenderApiFlag::CUDA));
        }
        else if (name == "slangUnitTestRegistryReportsUsedApis")
        {
            foundSelf = true;
            SLANG_CHECK(module->getTestUsedApis(i) == 0);
        }
    }
    SLANG_CHECK(foundProbe);
    SLANG_CHECK(foundSelf);
}

// CI selects the GPU APIs with `-api all-cpu-llvm`, which is easier to read than a list of all the
// GPU APIs. The expression has to stay equal to that list, so that a job that runs `-compile-test
// false` runs the tests of every GPU API and no test of the CPU-only job. `llvm` is the tests that
// use
// `-emit-cpu-via-llvm`.
SLANG_UNIT_TEST(renderApiFlagsAllWithoutCpuAndLlvmAreTheGpuApis, NoRenderApi)
{
    RenderApiFlags flags = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(RenderApiUtil::parseApiFlags(
        UnownedStringSlice("all-cpu-llvm"),
        RenderApiFlag::AllOf,
        &flags)));
    SLANG_CHECK(
        flags == (RenderApiFlag::Vulkan | RenderApiFlag::D3D12 | RenderApiFlag::D3D11 |
                  RenderApiFlag::Metal | RenderApiFlag::CUDA | RenderApiFlag::WebGPU));
}

namespace
{
// Runs Options::parse on `args`, as slang-test does with its command line. The output of the parse
// goes to `outText`, and the result of the parse is returned.
SlangResult parseTestOptions(const List<String>& args, Options& outOptions, StringBuilder& outText)
{
    List<char*> argv;
    argv.add(const_cast<char*>("slang-test"));
    for (const auto& arg : args)
    {
        argv.add(const_cast<char*>(arg.getBuffer()));
    }
    TestCategorySet categorySet;
    StringWriter writer(&outText, WriterFlags(0));
    WriterHelper helper(&writer);
    return Options::parse(
        (int)argv.getCount(),
        argv.getBuffer(),
        &categorySet,
        helper,
        helper,
        &outOptions);
}

// Writes a test list file with the entries and removes the file when it goes out of scope.
struct TemporaryListFile
{
    explicit TemporaryListFile(const char* text)
    {
        if (SLANG_SUCCEEDED(File::generateTemporary(UnownedStringSlice("slang-test-list"), path)))
        {
            File::writeAllText(path, text);
        }
    }
    ~TemporaryListFile()
    {
        if (path.getLength())
            File::remove(path);
    }
    String path;
};
} // namespace

// `-exclude-list` is another name of `-skip-list`, and both read the same format: path prefixes,
// with comments and empty lines ignored.
SLANG_UNIT_TEST(slangTestOptionsExcludeListIsSkipList, NoRenderApi)
{
    TemporaryListFile list(
        "# a comment\ntests/neural/\n\n  tests/cooperative-matrix/  # trailing\n");
    SLANG_CHECK_ABORT(list.path.getLength() > 0);

    for (const char* option : {"-skip-list", "-exclude-list"})
    {
        Options options;
        StringBuilder text;
        SLANG_CHECK(SLANG_SUCCEEDED(parseTestOptions({option, list.path}, options, text)));
        SLANG_CHECK_ABORT(options.skipList.getCount() == 2);
        SLANG_CHECK(options.skipList[0] == "tests/neural");
        SLANG_CHECK(options.skipList[1] == "tests/cooperative-matrix");
        SLANG_CHECK(options.includeList.getCount() == 0);
        SLANG_CHECK(options.testPrefixes.getCount() == 0);
    }
}

// `-include-list` selects tests in the same way as the test prefixes do, so it adds to them.
SLANG_UNIT_TEST(slangTestOptionsIncludeListAddsTestPrefixes, NoRenderApi)
{
    TemporaryListFile list("tests/neural/\ntests/cooperative-matrix/\n");
    SLANG_CHECK_ABORT(list.path.getLength() > 0);

    Options options;
    StringBuilder text;
    SLANG_CHECK(SLANG_SUCCEEDED(
        parseTestOptions({"-include-list", list.path, "tests/compute/"}, options, text)));
    SLANG_CHECK(options.includeList.getCount() == 2);
    SLANG_CHECK_ABORT(options.testPrefixes.getCount() == 3);
    SLANG_CHECK(options.testPrefixes[0] == "tests/compute");
    SLANG_CHECK(options.testPrefixes[1] == "tests/neural");
    SLANG_CHECK(options.testPrefixes[2] == "tests/cooperative-matrix");
    SLANG_CHECK(options.skipList.getCount() == 0);
}

// An include list that is missing or has no entries would select every test, so it is an error.
SLANG_UNIT_TEST(slangTestOptionsIncludeListMustHaveEntries, NoRenderApi)
{
    TemporaryListFile empty("# only a comment\n\n");
    SLANG_CHECK_ABORT(empty.path.getLength() > 0);
    {
        Options options;
        StringBuilder text;
        SLANG_CHECK(SLANG_FAILED(parseTestOptions({"-include-list", empty.path}, options, text)));
        SLANG_CHECK(text.toString().indexOf("no entries") != -1);
    }
    {
        Options options;
        StringBuilder text;
        SLANG_CHECK(SLANG_FAILED(parseTestOptions(
            {"-include-list", "tests/this-list-does-not-exist.txt"},
            options,
            text)));
    }
}

// `-compile-test` is true by default. `-api-only` turns it off, unless it is on the command line,
// whatever the order of the options is.
SLANG_UNIT_TEST(slangTestOptionsCompileTest, NoRenderApi)
{
    struct Case
    {
        List<String> args;
        bool expectedCompileTest;
    };
    const Case cases[] = {
        {{}, true},
        {{"-compile-test", "false"}, false},
        {{"-compile-test", "true"}, true},
        {{"-api-only"}, false},
        {{"-api-only", "-compile-test", "true"}, true},
        {{"-compile-test", "true", "-api-only"}, true},
        {{"-compile-test", "false", "-api-only"}, false},
    };
    for (const auto& c : cases)
    {
        Options options;
        StringBuilder text;
        SLANG_CHECK(SLANG_SUCCEEDED(parseTestOptions(c.args, options, text)));
        SLANG_CHECK(options.compileTest == c.expectedCompileTest);
    }

    Options options;
    StringBuilder text;
    SLANG_CHECK(SLANG_FAILED(parseTestOptions({"-compile-test"}, options, text)));
}
