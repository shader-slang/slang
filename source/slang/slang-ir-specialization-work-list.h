// slang-ir-specialization-work-list.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

// Stores pending specialization work in LIFO order. Membership only means that an instruction is
// currently queued, so an intrusive marker avoids the allocation and lookup cost of a HashSet.
class SpecializationWorkList
{
public:
    // Type legalization and autodiff use bits 0 and 1. Use the next available bit for queued
    // specialization membership. Whole-word scratchData users such as DCE and serialization must
    // not overlap queued specialization entries; the DCE calls in processModule() run only after
    // this work list is drained. Every exit path must clear the marker.
    static constexpr UInt64 kQueuedScratchDataBit = UInt64(1) << 2;

    explicit SpecializationWorkList(ContainerPool& containerPool)
        : m_containerPool(containerPool), m_entries(containerPool.getList<IRInst>())
    {
    }

    ~SpecializationWorkList()
    {
        clear();
        m_containerPool.free(m_entries);
    }

    SpecializationWorkList(const SpecializationWorkList&) = delete;
    SpecializationWorkList& operator=(const SpecializationWorkList&) = delete;

    bool add(IRInst* inst)
    {
        if (isQueued(inst))
        {
            // A set marker must belong to this queue. This catches stale ownership or overlapping
            // specialization queues in assertion-enabled builds without adding release overhead.
#ifdef _DEBUG
            SLANG_ASSERT(m_entries->indexOf(inst) != Index(-1));
#endif
            return false;
        }

        // Mark before the caller recursively adds users so cycles and duplicate enqueue attempts
        // observe this instruction as already queued.
        inst->scratchData |= kQueuedScratchDataBit;
        m_entries->add(inst);
        return true;
    }

    IRInst* pop()
    {
        SLANG_ASSERT(m_entries->getCount());
        auto inst = m_entries->getLast();
        m_entries->removeLast();

        // Clear immediately before processing so a transformation may requeue the instruction.
        inst->scratchData &= ~kQueuedScratchDataBit;
        return inst;
    }

    void clear()
    {
        // Error exits and standalone specializeGeneric() can abandon queued follow-up work. Clear
        // every surviving marker before discarding the entries.
        for (auto inst : *m_entries)
            inst->scratchData &= ~kQueuedScratchDataBit;
        m_entries->clear();
    }

    Index getCount() const { return m_entries->getCount(); }

    static bool isQueued(IRInst* inst) { return (inst->scratchData & kQueuedScratchDataBit) != 0; }

private:
    ContainerPool& m_containerPool;
    List<IRInst*>* m_entries;
};

} // namespace Slang
