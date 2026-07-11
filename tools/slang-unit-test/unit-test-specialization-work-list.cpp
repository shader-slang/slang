// unit-test-specialization-work-list.cpp

#include "slang/slang-ir-specialization-work-list.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(specializationWorkListDeduplicatesDuplicateEnqueue)
{
    ContainerPool pool;
    SpecializationWorkList workList(pool);
    IRInst inst = {};

    SLANG_CHECK(workList.add(&inst));
    SLANG_CHECK(!workList.add(&inst));
    SLANG_CHECK(workList.getCount() == 1);
    SLANG_CHECK(workList.pop() == &inst);
    SLANG_CHECK(!SpecializationWorkList::isQueued(&inst));
}

SLANG_UNIT_TEST(specializationWorkListAllowsRequeueAfterPop)
{
    ContainerPool pool;
    SpecializationWorkList workList(pool);
    IRInst inst = {};

    SLANG_CHECK(workList.add(&inst));
    SLANG_CHECK(workList.pop() == &inst);
    SLANG_CHECK(workList.add(&inst));
    SLANG_CHECK(workList.getCount() == 1);
    SLANG_CHECK(workList.pop() == &inst);
}

SLANG_UNIT_TEST(specializationWorkListDeduplicatesCyclesAndPreservesLifo)
{
    ContainerPool pool;
    SpecializationWorkList workList(pool);
    IRInst first = {};
    IRInst second = {};

    auto addWithUsers = [&](auto&& self, IRInst* inst) -> void
    {
        if (!workList.add(inst))
            return;
        self(self, inst == &first ? &second : &first);
    };

    addWithUsers(addWithUsers, &first);
    SLANG_CHECK(workList.getCount() == 2);
    SLANG_CHECK(workList.pop() == &second);
    SLANG_CHECK(workList.pop() == &first);
}

SLANG_UNIT_TEST(specializationWorkListClearsEveryMarkerOnEarlyExit)
{
    ContainerPool pool;
    SpecializationWorkList workList(pool);
    IRInst first = {};
    IRInst second = {};
    constexpr UInt64 kUnrelatedBits = (UInt64(1) << 63) | (UInt64(1) << 62) | (UInt64(1) << 1);
    first.scratchData = kUnrelatedBits;
    second.scratchData = kUnrelatedBits;

    SLANG_CHECK(workList.add(&first));
    SLANG_CHECK(workList.add(&second));
    workList.clear();

    SLANG_CHECK(workList.getCount() == 0);
    SLANG_CHECK(first.scratchData == kUnrelatedBits);
    SLANG_CHECK(second.scratchData == kUnrelatedBits);
}

SLANG_UNIT_TEST(specializationWorkListDestructionCleansDiscardedFollowUpWork)
{
    ContainerPool pool;
    IRInst inst = {};
    constexpr UInt64 kUnrelatedBits = UInt64(1) << 62;
    inst.scratchData = kUnrelatedBits;

    {
        SpecializationWorkList firstContext(pool);
        SLANG_CHECK(firstContext.add(&inst));
        SLANG_CHECK(SpecializationWorkList::isQueued(&inst));
        // Model standalone specializeGeneric(): queued follow-up work is intentionally discarded
        // when its temporary SpecializationContext is destroyed.
    }

    SLANG_CHECK(inst.scratchData == kUnrelatedBits);

    {
        SpecializationWorkList secondContext(pool);
        SLANG_CHECK(secondContext.add(&inst));
        SLANG_CHECK(secondContext.pop() == &inst);
    }

    SLANG_CHECK(inst.scratchData == kUnrelatedBits);
}
