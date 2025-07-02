#include "core/slang-basic.h"
#include "slang-ir.h"

namespace Slang
{

struct IROpMapEntry
{
    IROp op;
    IROpInfo info;
};

// TODO: We should ideally be speeding up the name->inst
// mapping by using a dictionary, or even by pre-computing
// a hash table to be stored as a `static const` array.
//
// NOTE! That this array is now constructed in such a way that looking up
// an entry from an op is fast, by keeping blocks of main, and pseudo ops in same order
// as the ops themselves. Care must be taken to keep this constraint.
static const IROpMapEntry kIROps[] = {

// Main ops in order

#if 0 // FIDDLE TEMPLATE:
% require("source/slang/slang-ir.h.lua").instInfoEntries()
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-ir-insts-info.cpp.fiddle"
#endif // FIDDLE END

    // Invalid op sentinel value comes after all the valid ones
    {kIROp_Invalid, {"invalid", 0, 0}},
};

IROpInfo getIROpInfo(IROp opIn)
{
    const int op = opIn & kIROpMask_OpMask;
    if (op < kIROpCount)
    {
        // It's a main op
        const auto& entry = kIROps[op];
        SLANG_ASSERT(entry.op == op);
        return entry.info;
    }

    // Don't know what this is
    SLANG_ASSERT(!"Invalid op");
    SLANG_ASSERT(kIROps[kIROpCount].op == kIROp_Invalid);
    return kIROps[kIROpCount].info;
}

IROp findIROp(const UnownedStringSlice& name)
{
    for (auto ee : kIROps)
    {
        if (name == ee.info.name)
            return ee.op;
    }

    return IROp(kIROp_Invalid);
}

} // namespace Slang
