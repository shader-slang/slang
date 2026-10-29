#include "core/slang-list.h"
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
SLANG_UNIT_TEST(slangTestContextThreadSlotsStartEmpty)
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
