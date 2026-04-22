#include "slang-ir-coverage-instrument.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

namespace
{

static const char kCoverageBufferName[] = "__slang_coverage";

// Reserved register space + binding for the synthesized coverage
// buffer. Chosen to sit high enough that it is unlikely to collide
// with user resources. Will become configurable once the feature
// grows a proper binding-CLI option.
static const UInt kCoverageReservedSpace = 31;
static const UInt kCoverageReservedBinding = 0;

// Key that identifies a counter slot: one slot per (file, line) pair.
// Multiple coverage ops that share the same (file, line) alias onto
// the same slot so that LCOV output matches gcov semantics.
struct CoverageKey
{
    UnownedStringSlice file;
    IRIntegerValue line;

    bool operator==(CoverageKey const& other) const
    {
        return file == other.file && line == other.line;
    }
};

struct CoverageKeyHasher
{
    HashCode operator()(CoverageKey const& key) const
    {
        return combineHash(getHashCode(key.file), getHashCode(key.line));
    }
};

// Return true if any existing module-scope global parameter already
// occupies the reserved `(space, binding)` we would assign to the
// synthesized coverage buffer. Used to warn before synthesis rather
// than aliasing a user resource silently.
static bool hasResourceAtReservedBinding(IRModule* module)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        auto layoutDecor = param->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            continue;
        auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
        if (!varLayout)
            continue;
        auto spaceAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace);
        auto bindingAttr = varLayout->findOffsetAttr(LayoutResourceKind::DescriptorTableSlot);
        if (!spaceAttr || !bindingAttr)
            continue;
        if ((UInt)spaceAttr->getOffset() == kCoverageReservedSpace &&
            (UInt)bindingAttr->getOffset() == kCoverageReservedBinding)
            return true;
    }
    return false;
}

// Collect every IncrementCoverageCounter op in the module across all
// functions. Simple O(n) linear sweep.
static void collectCoverageCounterOps(IRModule* module, List<IRInst*>& out)
{
    auto visitFunc = [&](IRFunc* func)
    {
        for (auto block : func->getBlocks())
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
                if (inst->getOp() == kIROp_IncrementCoverageCounter)
                    out.add(inst);
    };

    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
            visitFunc(func);
        else if (auto generic = as<IRGeneric>(inst))
        {
            if (auto inner = as<IRFunc>(findGenericReturnVal(generic)))
                visitFunc(inner);
        }
    }
}

// Synthesize an implicit `RWStructuredBuffer<uint> __slang_coverage`
// global parameter with a reserved layout. Follows the pattern in
// `lowerDynamicResourceHeap` — we manually build an IRTypeLayout /
// IRVarLayout so the buffer survives later uniform-collection passes.
// Emits a warning if any existing user resource already occupies the
// reserved binding; synthesis proceeds anyway but aliasing risk is
// documented to the user.
static IRGlobalParam* synthesizeCoverageBuffer(IRModule* module, DiagnosticSink* sink)
{
    if (hasResourceAtReservedBinding(module) && sink)
    {
        sink->diagnoseRaw(
            Severity::Warning,
            UnownedStringSlice("-trace-coverage: an existing resource occupies the reserved "
                               "binding (space=31, binding=0); coverage buffer may alias it."));
    }

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    auto uintType = builder.getUIntType();
    IRInst* elemOperand = uintType;
    auto bufferType = (IRType*)builder.getType(kIROp_HLSLRWStructuredBufferType, 1, &elemOperand);

    auto param = builder.createGlobalParam(bufferType);
    builder.addNameHintDecoration(param, UnownedStringSlice(kCoverageBufferName));

    IRTypeLayout::Builder typeLayoutBuilder(&builder);
    typeLayoutBuilder.addResourceUsage(LayoutResourceKind::DescriptorTableSlot, LayoutSize(1));
    auto typeLayout = typeLayoutBuilder.build();

    IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::RegisterSpace)->offset =
        kCoverageReservedSpace;
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::DescriptorTableSlot)->offset =
        kCoverageReservedBinding;
    auto varLayout = varLayoutBuilder.build();
    builder.addLayoutDecoration(param, varLayout);

    return param;
}

// Read the (file, line) for a coverage counter op from its built-in
// `sourceLoc` field, resolving via the source manager. Returns false
// when the location is not valid, e.g. for synthetic/unknown sources.
static bool readCoverageKey(SourceManager* sourceManager, IRInst* counterOp, CoverageKey& outKey)
{
    if (!sourceManager || !counterOp->sourceLoc.isValid())
        return false;
    auto humane = sourceManager->getHumaneLoc(counterOp->sourceLoc, SourceLocType::Emit);
    if (humane.line <= 0)
        return false;
    outKey.file = humane.pathInfo.foundPath.getUnownedSlice();
    outKey.line = (IRIntegerValue)humane.line;
    return true;
}

struct CoverageInstrumenter
{
    IRModule* module;
    IRGlobalParam* coverageBuffer;
    SourceManager* sourceManager;
    IRType* uintType;
    IRType* uintPtrType;
    IRType* intType;

    Dictionary<CoverageKey, UInt, CoverageKeyHasher> keyToIndex;

    // Owns the backing storage for each distinct key's file slice —
    // the humane path string is transient, so we copy it to a stable
    // buffer before storing in the dictionary.
    List<String> ownedFiles;

    CoverageInstrumenter(IRModule* m, IRGlobalParam* buf, SourceManager* sm)
        : module(m), coverageBuffer(buf), sourceManager(sm)
    {
        IRBuilder tmpBuilder(module);
        uintType = tmpBuilder.getUIntType();
        uintPtrType = tmpBuilder.getPtrType(uintType);
        intType = tmpBuilder.getIntType();
    }

    // Assign counter slots to unique (file, line) keys in
    // lexicographic order so slot indices are stable across unrelated
    // source edits (inserting a statement in one file doesn't shift
    // counter slots for other files).
    void assignSlots(List<IRInst*> const& counterOps)
    {
        Dictionary<CoverageKey, bool, CoverageKeyHasher> seen;
        List<CoverageKey> keys;
        for (auto op : counterOps)
        {
            CoverageKey key;
            if (!readCoverageKey(sourceManager, op, key))
                continue;
            if (seen.addIfNotExists(key, true))
            {
                // Copy the file path into a stable String so the slice
                // remains valid after humaneLoc's arena is reclaimed.
                ownedFiles.add(String(key.file));
                key.file = ownedFiles.getLast().getUnownedSlice();
                keys.add(key);
            }
        }
        keys.sort(
            [](CoverageKey const& a, CoverageKey const& b)
            {
                int cmp = compare(a.file, b.file);
                if (cmp != 0)
                    return cmp < 0;
                return a.line < b.line;
            });
        for (auto const& k : keys)
            keyToIndex[k] = (UInt)keyToIndex.getCount();
    }

    // Lower a single IncrementCoverageCounter op to an atomic add on
    // the coverage buffer slot for its (file, line). Always removes
    // the op afterwards.
    void lowerCounterOp(IRInst* counterOp)
    {
        CoverageKey key;
        if (!readCoverageKey(sourceManager, counterOp, key))
        {
            counterOp->removeAndDeallocate();
            return;
        }
        UInt counterIdx = 0;
        if (!keyToIndex.tryGetValue(key, counterIdx))
        {
            counterOp->removeAndDeallocate();
            return;
        }

        IRBuilder builder(module);
        builder.setInsertBefore(counterOp);

        IRInst* getElemArgs[] = {
            coverageBuffer,
            builder.getIntValue(intType, (IRIntegerValue)counterIdx),
        };
        IRInst* slotPtr = builder.emitIntrinsicInst(
            uintPtrType,
            kIROp_RWStructuredBufferGetElementPtr,
            2,
            getElemArgs);

        // Emit `AtomicAdd(slotPtr, 1, relaxed)` — lowered by each
        // backend emitter to its native atomic-increment idiom
        // (InterlockedAdd on HLSL, atomicAdd on GLSL, OpAtomicIAdd on
        // SPIR-V, etc.). Correct under GPU concurrency.
        IRInst* atomicArgs[] = {
            slotPtr,
            builder.getIntValue(uintType, 1),
            builder.getIntValue(intType, (IRIntegerValue)kIRMemoryOrder_Relaxed),
        };
        builder.emitIntrinsicInst(uintType, kIROp_AtomicAdd, 3, atomicArgs);

        counterOp->removeAndDeallocate();
    }

    void run(List<IRInst*> const& counterOps)
    {
        assignSlots(counterOps);
        for (auto op : counterOps)
            lowerCounterOp(op);
    }
};

} // anonymous namespace

void instrumentCoverage(IRModule* module, DiagnosticSink* sink, bool enabled)
{
    // Always collect any counter ops so stale ones from cached modules
    // can't leak into the backend when the flag is off.
    List<IRInst*> counterOps;
    collectCoverageCounterOps(module, counterOps);

    if (!enabled)
    {
        // Flag off: drop any counter ops without emitting atomics.
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    if (counterOps.getCount() == 0)
        return;

    // Always synthesize the coverage buffer. Intentionally does not
    // look for a user-declared `__slang_coverage` — a reserved name
    // plus name-hint discovery is unreliable (e.g. after debug-info
    // stripping or name mangling), and binding conflicts are better
    // handled via a dedicated CLI option in a follow-up.
    auto buffer = synthesizeCoverageBuffer(module, sink);
    if (!buffer)
    {
        for (auto op : counterOps)
            op->removeAndDeallocate();
        return;
    }

    CoverageInstrumenter instrumenter(module, buffer, sink ? sink->getSourceManager() : nullptr);
    instrumenter.run(counterOps);
}

} // namespace Slang
