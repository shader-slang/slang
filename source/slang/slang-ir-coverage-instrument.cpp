#include "slang-ir-coverage-instrument.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

#include <cstdio>

namespace Slang
{

namespace
{

// The coverage buffer is synthesized as a global-scope `VarDecl` at
// AST-check time by `maybeSynthesizeCoverageBufferDecl` (see
// slang-check-synthesize-coverage.cpp). By the time this IR pass
// runs, the buffer is a normal `IRGlobalParam` indistinguishable
// from a user-declared one; this pass only locates it by name and
// rewrites `IncrementCoverageCounter` placeholders against it.
static const char kCoverageBufferName[] = "__slang_coverage";

// Key that identifies a counter slot in the host-facing manifest:
// one slot per (file, line) pair. Multiple placeholder UIDs that
// share the same (file, line) alias onto the same slot so that LCOV
// output matches gcov semantics.
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

// Locate the module-scope `RWStructuredBuffer<uint> __slang_coverage`
// declared by the user, if any. Returns nullptr when no such buffer
// exists. If a buffer with the reserved name is present but has the
// wrong type (not a `RWStructuredBuffer<uint>`), that is a user error:
// diagnose via `sink` and treat it as absent so the pass is a no-op
// rather than generating invalid IR.
static IRGlobalParam* findCoverageBuffer(IRModule* module, DiagnosticSink* sink)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        auto nameHint = param->findDecoration<IRNameHintDecoration>();
        if (!nameHint)
            continue;
        if (nameHint->getName() != UnownedStringSlice(kCoverageBufferName))
            continue;
        auto bufferType = as<IRHLSLRWStructuredBufferType>(param->getDataType());
        if (!bufferType)
        {
            if (sink)
                sink->diagnoseRaw(
                    Severity::Warning,
                    UnownedStringSlice("'__slang_coverage' is reserved for -trace-coverage; "
                                       "declaration must be 'RWStructuredBuffer<uint>'. "
                                       "Ignoring and synthesizing the coverage buffer."));
            return nullptr;
        }
        // Validate the element type is specifically `uint`. Anything
        // else (float, int, struct) would produce bad IR once we emit
        // atomic-add on the slot pointer.
        auto elementType = as<IRBasicType>(bufferType->getElementType());
        if (!elementType || elementType->getBaseType() != BaseType::UInt)
        {
            if (sink)
                sink->diagnoseRaw(
                    Severity::Warning,
                    UnownedStringSlice("'__slang_coverage' element type must be 'uint' for "
                                       "-trace-coverage. Ignoring and synthesizing the "
                                       "coverage buffer."));
            return nullptr;
        }
        return param;
    }
    return nullptr;
}

// Collect every IncrementCoverageCounter placeholder in the module
// across all functions. The pass is designed to be a simple, O(n)
// linear sweep — placeholders are opaque to the optimizer so if any
// survived they're all still present at this point.
static void collectPlaceholders(IRModule* module, List<IRInst*>& out)
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

struct CoverageInstrumenter
{
    IRModule* module;
    IRGlobalParam* coverageBuffer;
    IRType* uintType;
    IRType* uintPtrType;
    IRType* intType;

    Dictionary<CoverageKey, UInt, CoverageKeyHasher> keyToIndex;
    List<CoverageKey> orderedKeys;

    CoverageInstrumenter(IRModule* m, IRGlobalParam* buf)
        : module(m), coverageBuffer(buf)
    {
        IRBuilder tmpBuilder(module);
        uintType = tmpBuilder.getUIntType();
        uintPtrType = tmpBuilder.getPtrType(uintType);
        intType = tmpBuilder.getIntType();
    }

    struct BufferLayoutInfo
    {
        bool hasLayout = false;
        IRIntegerValue space = -1;
        IRIntegerValue binding = -1;
        IRIntegerValue descriptorSet = -1;
        IRIntegerValue uavRegister = -1;
    };

    BufferLayoutInfo readLayout() const
    {
        BufferLayoutInfo info;
        auto layoutDecor = coverageBuffer->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            return info;
        auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
        if (!varLayout)
            return info;
        info.hasLayout = true;
        if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
            info.space = (IRIntegerValue)a->getOffset();
        if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::DescriptorTableSlot))
        {
            info.binding = (IRIntegerValue)a->getOffset();
            info.descriptorSet = info.space;
        }
        if (auto a = varLayout->findOffsetAttr(LayoutResourceKind::UnorderedAccess))
            info.uavRegister = (IRIntegerValue)a->getOffset();
        return info;
    }

    UInt getOrAssignCounter(CoverageKey const& key)
    {
        UInt idx = 0;
        if (keyToIndex.tryGetValue(key, idx))
            return idx;
        idx = (UInt)orderedKeys.getCount();
        keyToIndex[key] = idx;
        orderedKeys.add(key);
        return idx;
    }

    // Replace an IncrementCoverageCounter placeholder with the actual
    // counter write on `coverageBuffer[counterIdx]`.
    void lowerPlaceholder(IRInst* placeholder)
    {
        auto loc = placeholder->findDecoration<IRDebugLocationDecoration>();
        if (!loc)
        {
            placeholder->removeAndDeallocate();
            return;
        }
        auto debugSource = as<IRDebugSource>(loc->getSource());
        if (!debugSource)
        {
            placeholder->removeAndDeallocate();
            return;
        }
        auto fileLit = as<IRStringLit>(debugSource->getFileName());
        if (!fileLit)
        {
            placeholder->removeAndDeallocate();
            return;
        }

        CoverageKey key;
        key.file = fileLit->getStringSlice();
        key.line = getIntVal(loc->getLine());
        UInt counterIdx = getOrAssignCounter(key);

        IRBuilder builder(module);
        builder.setInsertBefore(placeholder);

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

        placeholder->removeAndDeallocate();
    }

    void run()
    {
        List<IRInst*> placeholders;
        collectPlaceholders(module, placeholders);
        for (auto ph : placeholders)
            lowerPlaceholder(ph);
    }

    void printStderrManifest()
    {
        for (Index i = 0; i < orderedKeys.getCount(); ++i)
        {
            auto const& key = orderedKeys[i];
            fprintf(
                stderr,
                "slang-coverage-manifest: %u,%.*s,%lld\n",
                (unsigned)i,
                (int)key.file.getLength(),
                key.file.begin(),
                (long long)key.line);
        }
    }

    // Write a minimal JSON manifest describing the counter layout.
    bool writeJsonManifest(char const* path)
    {
        FILE* f = fopen(path, "w");
        if (!f)
            return false;
        fprintf(f, "{\n");
        fprintf(f, "  \"version\": 1,\n");
        fprintf(f, "  \"counters\": %lld,\n", (long long)orderedKeys.getCount());

        // The coverage buffer is always reflection-visible:
        // AST-synthesized when `-trace-coverage` is on, or user-
        // declared by the shader itself. The binding reported below
        // is whatever the parameter-binding pass assigned; hosts
        // use this to bind the counter UAV at runtime.
        auto layout = readLayout();
        fprintf(f, "  \"buffer\": {\n");
        fprintf(f, "    \"name\": \"%s\",\n", kCoverageBufferName);
        fprintf(f, "    \"element_type\": \"uint32\",\n");
        fprintf(f, "    \"element_stride\": 4");
        if (layout.hasLayout)
        {
            if (layout.space >= 0)
                fprintf(f, ",\n    \"space\": %lld", (long long)layout.space);
            if (layout.binding >= 0)
                fprintf(f, ",\n    \"binding\": %lld", (long long)layout.binding);
            if (layout.descriptorSet >= 0)
                fprintf(f, ",\n    \"descriptor_set\": %lld", (long long)layout.descriptorSet);
            if (layout.uavRegister >= 0)
                fprintf(f, ",\n    \"uav_register\": %lld", (long long)layout.uavRegister);
        }
        fprintf(f, "\n  },\n");

        fprintf(f, "  \"entries\": [");
        for (Index i = 0; i < orderedKeys.getCount(); ++i)
        {
            auto const& key = orderedKeys[i];
            fprintf(f, "%s\n    {\"index\": %lld, \"file\": \"", i == 0 ? "" : ",", (long long)i);
            // JSON requires escaping backslash, double-quote, and all
            // control characters (U+0000..U+001F). Source file paths can
            // come from user `#line` directives or synthetic locations
            // and may legally contain tabs, newlines, etc. — emit
            // specific escape sequences where defined and `\uXXXX`
            // elsewhere to keep the manifest parseable.
            for (auto c : key.file)
            {
                unsigned char uc = (unsigned char)c;
                switch (uc)
                {
                case '\\':
                    fputs("\\\\", f);
                    break;
                case '"':
                    fputs("\\\"", f);
                    break;
                case '\b':
                    fputs("\\b", f);
                    break;
                case '\f':
                    fputs("\\f", f);
                    break;
                case '\n':
                    fputs("\\n", f);
                    break;
                case '\r':
                    fputs("\\r", f);
                    break;
                case '\t':
                    fputs("\\t", f);
                    break;
                default:
                    if (uc < 0x20)
                        fprintf(f, "\\u%04x", (unsigned)uc);
                    else
                        fputc((int)uc, f);
                    break;
                }
            }
            fprintf(f, "\", \"line\": %lld}", (long long)key.line);
        }
        fprintf(f, "\n  ]\n");
        fprintf(f, "}\n");
        fclose(f);
        return true;
    }

    Index counterCount() const { return orderedKeys.getCount(); }
};

// Strip any leftover IncrementCoverageCounter placeholders from the
// module. Called when the pass is a no-op (neither flag nor user
// buffer) so the backend emitter never sees the opaque op.
static void stripPlaceholders(IRModule* module)
{
    List<IRInst*> placeholders;
    collectPlaceholders(module, placeholders);
    for (auto ph : placeholders)
        ph->removeAndDeallocate();
}

} // anonymous namespace

void instrumentCoverage(IRModule* module, DiagnosticSink* sink, bool enabled)
{
    // With the flag off: guarantee a true no-op. Any placeholders that
    // might have made it in via a stale cached module are dropped so
    // the backend emitter never sees them, but we do not print any
    // info messages or write a manifest.
    if (!enabled)
    {
        stripPlaceholders(module);
        return;
    }

    // Flag is on. The AST synthesizer (see
    // slang-check-synthesize-coverage.cpp) has already injected the
    // `__slang_coverage` decl unless the user declared it themselves,
    // so `findCoverageBuffer` always returns a valid buffer at this
    // point. If it doesn't, some earlier phase has failed; bail.
    auto buffer = findCoverageBuffer(module, sink);
    if (!buffer)
    {
        stripPlaceholders(module);
        return;
    }

    CoverageInstrumenter instrumenter(module, buffer);
    instrumenter.run();

    // Always announce the counter count on stderr — the host needs
    // this to allocate the counter buffer at the right size.
    fprintf(stderr, "slang-coverage-info: counters=%lld\n", (long long)instrumenter.counterCount());

    if (auto path = getenv("SLANG_COVERAGE_MANIFEST_PATH"))
    {
        if (!instrumenter.writeJsonManifest(path))
        {
            fprintf(stderr, "slang-coverage-info: failed to write manifest to '%s'\n", path);
        }
        else
        {
            fprintf(stderr, "slang-coverage-info: manifest written to '%s'\n", path);
        }
    }

    if (getenv("SLANG_COVERAGE_DUMP_MANIFEST"))
        instrumenter.printStderrManifest();
}

} // namespace Slang
