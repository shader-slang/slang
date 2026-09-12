// unit-test-ir-deferred-body.cpp

#include "core/slang-blob.h"
#include "core/slang-platform.h"
#include "core/slang-riff.h"
#include "core/slang-stream.h"
#include "slang-com-ptr.h"
#include "slang/slang-compiler-api.h"
#include "slang/slang-ir-insts.h"
#include "slang/slang-serialize-ir.h"
#include "slang/slang-serialize-types.h"
#include "slang/slang-serialize.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>

using namespace Slang;

namespace
{

/// What blob `_roundTripWithBlobMode` hands the reader. Only `Matching` permits deferral;
/// the other two must fall back to an eager load and produce an identical module.
enum class BlobMode
{
    /// The blob the bytes were parsed out of.
    Matching,
    /// No blob, which is what a caller reading from its own buffer supplies.
    Null,
    /// An identical copy at a different address -- the shape `addLibraryReference` had.
    Mismatched,
};

Index _countChildrenOf(IRInst* inst)
{
    Index count = 0;
    for (IRInst* child : inst->getChildren())
    {
        SLANG_UNUSED(child);
        count++;
    }
    return count;
}

/// Round-trips a module whose decoration has children of its own. A deferred load that
/// dropped the decoration's subtree shows up as `outActualChildren < outExpectedChildren`.
/// `outBodyWasDeferred` distinguishes a real exercise of the rule from an eager load.
void _roundTripDecorationWithChildren(
    slang::IGlobalSession* globalSession,
    Index& outExpectedChildren,
    Index& outActualChildren,
    bool& outBodyWasDeferred)
{
    outExpectedChildren = 0;
    outActualChildren = 0;
    outBodyWasDeferred = false;

    Session* session = static_cast<Session*>(globalSession);

    // Build a function whose decoration is itself a parent, and which also has a body.
    //
    // The shape is the point, not what the instructions mean. A global value's children
    // are its decorations followed by its body, and deferral cuts between the two -- so a
    // decoration with children of its own puts instructions on the eager side of that cut
    // at the same depth as instructions on the deferred side.
    // `DifferentiableTypeDictionaryDecoration` is used only because it is a decoration
    // declared `parent = true`; nothing here depends on autodiff.
    RefPtr<IRModule> original = IRModule::create(session);
    IRInst* originalFunc = nullptr;
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        originalFunc = builder.createFunc();

        IRInst* dict = builder.addDifferentiableTypeDictionaryDecoration(originalFunc);
        IRInst* floatType = builder.getFloatType();
        builder.addDifferentiableTypeEntry(dict, floatType, floatType);
        builder.addDifferentiableTypeEntry(dict, floatType, floatType);

        // Something to defer. With only decorations, nothing is deferred and the round
        // trip would say nothing about the cut.
        builder.setInsertInto(originalFunc);
        builder.emitBlock();
        builder.emitReturn();
    }

    if (IRInst* dict =
            originalFunc->findDecorationImpl(kIROp_DifferentiableTypeDictionaryDecoration))
    {
        outExpectedChildren = _countChildrenOf(dict);
    }

    OwnedMemoryStream stream(FileAccess::ReadWrite);
    {
        RIFF::Builder riffBuilder;
        RIFF::BuildCursor cursor(riffBuilder);
        // The IR chunk is written as the root, so it can be found again without pulling in
        // the surrounding module-container layout, which this has no use for.
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, PropertyKeys<IRModule>::IRModule);
        writeSerializedModuleIR(cursor, original, nullptr);
        if (SLANG_FAILED(riffBuilder.writeTo(&stream)))
            return;
    }

    // Read back out of a blob, which is what makes deferral possible: the flat table holds
    // spans into these bytes rather than copies, so a body decoded later needs them still
    // alive. `readSerializedModuleIR` loads eagerly when handed null.
    const auto contents = stream.getContents();
    List<uint8_t> bytes;
    bytes.addRange(contents.getBuffer(), contents.getCount());
    ComPtr<ISlangBlob> blob = ListBlob::create(bytes);

    auto rootChunk = RIFF::RootChunk::getFromBlob(blob->getBufferPointer(), blob->getBufferSize());
    if (!rootChunk)
        return;

    // The root here is the `ir  ` list chunk written above, and the module is its first
    // child -- the same step `ModuleChunk::findIR()` takes. Handing the list chunk itself
    // to the reader instead walks the wrong level and corrupts the heap.
    auto irChunk = rootChunk->getFirstChild().get();
    if (!irChunk)
        return;

    RefPtr<IRModule> reloaded;
    if (SLANG_FAILED(readSerializedModuleIR(irChunk, session, nullptr, blob, reloaded)))
        return;

    IRInst* func = nullptr;
    for (IRInst* child : reloaded->getModuleInst()->getChildren())
    {
        if (child->getOp() == kIROp_Func)
        {
            func = child;
            break;
        }
    }
    if (!func)
        return;

    outBodyWasDeferred = func->m_hasDeferredBody;

    // Walking the decoration list does not materialize the body -- that is the access
    // pattern decorations are kept eager to serve, and the one this rule protects.
    if (IRInst* dict = func->findDecorationImpl(kIROp_DifferentiableTypeDictionaryDecoration))
    {
        outActualChildren = _countChildrenOf(dict);
    }
}


/// Serializes `module` and reads it back out of a blob, which is the condition that lets
/// bodies stay encoded. Shared by the two helpers below.
SlangResult _roundTripModule(
    IRModule* module,
    Session* session,
    ComPtr<ISlangBlob>& outBlob,
    RefPtr<IRModule>& outModule,
    BlobMode blobMode = BlobMode::Matching)
{
    OwnedMemoryStream stream(FileAccess::ReadWrite);
    {
        RIFF::Builder riffBuilder;
        RIFF::BuildCursor cursor(riffBuilder);
        SLANG_SCOPED_RIFF_BUILDER_LIST_CHUNK(cursor, PropertyKeys<IRModule>::IRModule);
        writeSerializedModuleIR(cursor, module, nullptr);
        SLANG_RETURN_ON_FAIL(riffBuilder.writeTo(&stream));
    }

    const auto contents = stream.getContents();
    List<uint8_t> bytes;
    bytes.addRange(contents.getBuffer(), contents.getCount());
    outBlob = ListBlob::create(bytes);

    auto rootChunk =
        RIFF::RootChunk::getFromBlob(outBlob->getBufferPointer(), outBlob->getBufferSize());
    if (!rootChunk)
        return SLANG_FAIL;
    // The root is the `ir  ` list chunk written above and the module is its first child --
    // the step `ModuleChunk::findIR()` takes. Handing the list chunk itself to the reader
    // walks the wrong level and corrupts the heap.
    auto irChunk = rootChunk->getFirstChild().get();
    if (!irChunk)
        return SLANG_FAIL;

    ISlangBlob* blobForReader = outBlob;
    ComPtr<ISlangBlob> decoyBlob;
    switch (blobMode)
    {
    case BlobMode::Null:
        blobForReader = nullptr;
        break;
    case BlobMode::Mismatched:
        // Same bytes, different allocation. Deferral must decline: the chunk pointers and
        // spans refer into `outBlob`, so retaining this one would keep the wrong memory
        // alive and leave the views dangling the moment `outBlob` went away.
        decoyBlob = ListBlob::create(bytes);
        blobForReader = decoyBlob;
        break;
    case BlobMode::Matching:
        break;
    }

    return readSerializedModuleIR(irChunk, session, nullptr, blobForReader, outModule);
}

/// Races many threads to first-touch the same deferred bodies. `outMismatches` counts
/// threads that saw a body with the wrong instruction count -- what a torn or partially
/// published body looks like. `outDeferredCount` distinguishes a real race from an eager
/// load, which races nothing.
///
/// Scoped to materialization deliberately: running whole compiles concurrently on a shared
/// global session is documented as unsupported and crashes either way, so a test shaped
/// that way would exercise unsupported usage rather than this mechanism.
void _materializeBodiesConcurrently(
    slang::IGlobalSession* globalSession,
    Index& outDeferredCount,
    Index& outMismatches)
{
    outDeferredCount = 0;
    outMismatches = 0;

    Session* session = static_cast<Session*>(globalSession);

    // Enough functions that the threads spread across bodies rather than all queuing on
    // one, and enough instructions per body that a partially published chain is visible as
    // a short one rather than needing exact timing to catch.
    static const Index kFuncCount = 64;
    static const Index kBodyInstCount = 24;

    RefPtr<IRModule> original = IRModule::create(session);
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        for (Index f = 0; f < kFuncCount; ++f)
        {
            IRInst* func = builder.createFunc();
            // Decorations are required for this to test what it claims. A deferred body is
            // published into the link *after the last decoration*, so with none the body
            // attaches at `first`, the decoration walk starts at null and ends immediately,
            // and the acquire on that link is never exercised.
            builder.addNameHintDecoration(func, UnownedStringSlice("concurrentProbe"));
            builder.setInsertInto(func);
            builder.emitBlock();
            for (Index i = 0; i < kBodyInstCount - 2; ++i)
            {
                IRType* floatType = builder.getFloatType();
                builder.emitAdd(
                    floatType,
                    builder.getFloatValue(floatType, IRFloatingPointValue(i)),
                    builder.getFloatValue(floatType, IRFloatingPointValue(1)));
            }
            builder.emitReturn();
            builder.setInsertInto(original->getModuleInst());
        }
    }

    ComPtr<ISlangBlob> blob;
    RefPtr<IRModule> reloaded;
    if (SLANG_FAILED(_roundTripModule(original, session, blob, reloaded)))
        return;

    List<IRInst*> funcs;
    for (IRInst* child : reloaded->getModuleInst()->getChildren())
    {
        if (child->getOp() == kIROp_Func)
            funcs.add(child);
    }
    if (funcs.getCount() != kFuncCount)
        return;

    for (IRInst* func : funcs)
    {
        if (func->m_hasDeferredBody)
            outDeferredCount++;
    }

    // Counted from the pre-serialization module, so the expectation does not come from the
    // path under test.
    List<Index> expected;
    for (IRInst* child : original->getModuleInst()->getChildren())
    {
        if (child->getOp() != kIROp_Func)
            continue;
        expected.add(_countChildrenOf(child));
    }
    if (expected.getCount() != funcs.getCount())
        return;

    // Released together so every thread arrives at the same untouched body at once. Staggered
    // starts would let each body finish materializing before the next thread reached it,
    // which is the uncontended case the other tests already cover.
    static const int kThreadCount = 8;
    std::atomic<bool> go{false};
    std::atomic<Index> mismatches{0};
    List<std::thread> threads;
    for (int t = 0; t < kThreadCount; ++t)
    {
        threads.add(std::thread(
            [&, threadIndex = t]()
            {
                while (!go.load(std::memory_order_acquire))
                    std::this_thread::yield();
                // Half the threads publish, half walk decorations. Materializing from
                // every thread exercises the mutex but not the barrier that matters most:
                // the decoration walk is the one reader allowed to observe the publication
                // link *without* going through `ensureBodyMaterialized`, which is why
                // `getFirstDecoration`, `getNextDecoration` and
                // `IRDecorationList::Iterator::operator++` load it with acquire. Unless
                // some thread is walking decorations while another publishes into
                // `lastDecoration->next`, that race is never run and dropping those
                // acquires passes every test.
                const bool walksDecorations = (threadIndex % 2) == 1;
                for (Index i = 0; i < funcs.getCount(); ++i)
                {
                    if (walksDecorations)
                    {
                        // Must never run past the decorations into a body that another
                        // thread is publishing. Counting is enough to catch it: a walk
                        // that continues into the body returns more than there are
                        // decorations.
                        Index decorationCount = 0;
                        for (IRDecoration* decoration : funcs[i]->getDecorations())
                        {
                            SLANG_UNUSED(decoration);
                            decorationCount++;
                        }
                        // Exactly the one decoration added above. More than that means the
                        // walk followed a link into a body another thread was publishing
                        // and kept going, counting body instructions as decorations.
                        if (decorationCount != 1)
                            mismatches.fetch_add(1);
                    }
                    else
                    {
                        // The first touch of each body: this is what takes the loader's
                        // mutex and, on the winning thread, publishes the chain with a
                        // release store.
                        funcs[i]->ensureBodyMaterialized();
                        if (_countChildrenOf(funcs[i]) != expected[i])
                            mismatches.fetch_add(1);
                    }
                }
            }));
    }
    go.store(true, std::memory_order_release);
    for (auto& thread : threads)
        thread.join();

    outMismatches = mismatches.load();
}


/// Round-trips a module with a chosen blob mode, reporting whether deferral was taken
/// (`outDeferredLoaderInstalled`), what was loaded (`outInstCount`, which must match across
/// modes), and whether the containment check fired (`outSpanMismatchDelta`).
void _roundTripWithBlobMode(
    slang::IGlobalSession* globalSession,
    BlobMode blobMode,
    bool& outDeferredLoaderInstalled,
    Index& outInstCount,
    Index& outSpanMismatchDelta)
{
    outDeferredLoaderInstalled = false;
    outInstCount = 0;
    outSpanMismatchDelta = 0;

    Session* session = static_cast<Session*>(globalSession);

    // A module with several bodies, so a deferred load has something to defer and an
    // eager one has something to get wrong.
    RefPtr<IRModule> original = IRModule::create(session);
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        for (Index f = 0; f < 8; ++f)
        {
            IRInst* func = builder.createFunc();
            builder.setInsertInto(func);
            builder.emitBlock();
            IRType* floatType = builder.getFloatType();
            for (Index i = 0; i < 6; ++i)
            {
                builder.emitAdd(
                    floatType,
                    builder.getFloatValue(floatType, IRFloatingPointValue(i)),
                    builder.getFloatValue(floatType, IRFloatingPointValue(1)));
            }
            builder.emitReturn();
            builder.setInsertInto(original->getModuleInst());
        }
    }

    const Index mismatchBefore = getDeferralDeclinedForSpanMismatchCount();

    ComPtr<ISlangBlob> blob;
    RefPtr<IRModule> reloaded;
    if (SLANG_FAILED(_roundTripModule(original, session, blob, reloaded, blobMode)))
        return;

    outSpanMismatchDelta = getDeferralDeclinedForSpanMismatchCount() - mismatchBefore;
    outDeferredLoaderInstalled = (reloaded->getDeferredBodyLoader() != nullptr);

    // Counting every instruction forces every body to materialize if it was deferred, and
    // reads every body if it was not -- so the same number must come back either way. That
    // is the property the fallbacks exist to preserve: declining deferral may cost time,
    // but it must never change what was loaded.
    Index count = 0;
    for (IRInst* global : reloaded->getModuleInst()->getChildren())
    {
        count++;
        for (IRInst* child : global->getChildren())
        {
            count++;
            for (IRInst* grandchild : child->getChildren())
            {
                SLANG_UNUSED(grandchild);
                count++;
            }
        }
    }
    outInstCount = count;
}

/// Counts the instructions inside a global's blocks -- the part of it a deferred load
/// leaves encoded, and so the part a mishandled splice loses.
Index _countBodyInstsOf(IRInst* global)
{
    Index count = 0;
    for (IRInst* block : global->getChildren())
        count += _countChildrenOf(block);
    return count;
}

/// Which mutation entry point `_mutateGlobalWithDeferredBody` drives.
enum class DeferredParentMutation
{
    /// Splice a block in right after the last decoration -- the exact link a deferred body
    /// is published into, and so the one a stale read lands on.
    InsertAfterLastDecoration,
    /// Append a block to a global whose existing body is still encoded. Where it lands
    /// says whether `getLastDecorationOrChild` decoded the body before answering.
    InsertAtEnd,
    /// Unlink the last decoration of a global whose body is still encoded.
    RemoveLastDecoration,
};

/// What a mutation left behind, measured on the reloaded global after it ran.
struct DeferredParentMutationResult
{
    /// Body instructions counted on the module before it was serialized, and on the
    /// reloaded global after the mutation. They agree only if the body survived.
    Index expectedBodyInsts = 0;
    Index actualBodyInsts = 0;
    /// The global's children after the mutation, and where the spliced block landed among
    /// them (-1 when the mutation spliced nothing).
    Index childCount = 0;
    Index splicedChildIndex = -1;
    Index decorationCount = 0;
    /// Whether the body really was still encoded when the mutation ran. Without this an
    /// eager load would satisfy every check above while testing nothing.
    bool bodyWasDeferred = false;
};

/// Runs `mutation` against a global whose body is still encoded, and reports what it left.
///
/// Both the surviving body and the spliced block's final position matter. The body says
/// the mutation did not destroy what it could not see; the position says the mutation saw
/// the whole list rather than the decorations alone, which is the difference between
/// appending after the body and appending in front of it.
DeferredParentMutationResult _mutateGlobalWithDeferredBody(
    slang::IGlobalSession* globalSession,
    DeferredParentMutation mutation)
{
    DeferredParentMutationResult result;

    Session* session = static_cast<Session*>(globalSession);

    RefPtr<IRModule> original = IRModule::create(session);
    IRInst* originalFunc = nullptr;
    {
        IRBuilder builder(original);
        builder.setInsertInto(original->getModuleInst());
        originalFunc = builder.createFunc();
        // A decoration is required for this to test what it claims. A deferred body is
        // published into the link *after the last decoration*, so a global with none has
        // no stale link for a splice to land on.
        builder.addNameHintDecoration(originalFunc, UnownedStringSlice("mutationProbe"));
        builder.setInsertInto(originalFunc);
        builder.emitBlock();
        IRType* floatType = builder.getFloatType();
        for (Index i = 0; i < 10; ++i)
        {
            builder.emitAdd(
                floatType,
                builder.getFloatValue(floatType, IRFloatingPointValue(i)),
                builder.getFloatValue(floatType, IRFloatingPointValue(1)));
        }
        builder.emitReturn();
    }
    result.expectedBodyInsts = _countBodyInstsOf(originalFunc);

    ComPtr<ISlangBlob> blob;
    RefPtr<IRModule> reloaded;
    if (SLANG_FAILED(_roundTripModule(original, session, blob, reloaded)))
        return result;

    IRInst* func = nullptr;
    for (IRInst* global : reloaded->getModuleInst()->getChildren())
    {
        if (global->getOp() == kIROp_Func)
        {
            func = global;
            break;
        }
    }
    if (!func)
        return result;

    // Read before anything below can decode it: past this point the test only means
    // something if this was true.
    result.bodyWasDeferred = func->m_hasDeferredBody;

    // The block to splice has to be built somewhere other than `func`. Every way of adding
    // one to `func` directly goes through an accessor that decodes it first, which is
    // precisely the state this test needs it to still be in.
    IRBuilder builder(reloaded);
    builder.setInsertInto(reloaded->getModuleInst());
    IRInst* scratch = builder.createFunc();
    builder.setInsertInto(scratch);
    IRInst* splicedBlock = builder.emitBlock();
    bool splicedIntoFunc = true;

    // `getLastDecoration` walks with `peekNextInst` and so does not decode anything, which
    // is what leaves each mutation below reaching a parent whose body is still encoded.
    switch (mutation)
    {
    case DeferredParentMutation::InsertAfterLastDecoration:
        splicedBlock->insertAfter(func->getLastDecoration());
        break;
    case DeferredParentMutation::InsertAtEnd:
        splicedBlock->insertAtEnd(func);
        break;
    case DeferredParentMutation::RemoveLastDecoration:
        func->getLastDecoration()->removeFromParent();
        splicedIntoFunc = false;
        break;
    }

    for (IRInst* child : func->getChildren())
    {
        if (splicedIntoFunc && child == splicedBlock)
            result.splicedChildIndex = result.childCount;
        result.childCount++;
        // The spliced block is empty, so this counts the original body alone.
        result.actualBodyInsts += _countChildrenOf(child);
    }
    for (IRDecoration* decoration : func->getDecorations())
    {
        SLANG_UNUSED(decoration);
        result.decorationCount++;
    }

    return result;
}

} // namespace


// Checks that a decoration's own children survive on-demand loading.
//
// This guards the `inEagerDecoration` rule in `_computeEagerSkeleton`. Decorations are kept
// eager because the symbol index reads them without materializing anything, and a
// decoration that is itself a parent means keeping the decoration is not enough: its
// children are reachable only through it, so nothing on that path would ever trigger the
// materialization that would supply them. Keeping only the decoration inst gives back a
// decoration that silently has no children.
//
// The module under test is built directly rather than compiled from source, and the
// building happens inside slang because that is where the IR builders live. The shape does
// not occur in any module the compiler produces -- a scan over every serialized decoration
// in the builtin modules finds zero with children, and a precompiled module built from
// autodiff source has none either -- so a test driven by a shader would pass whether this
// rule were implemented or not. The IR permits the shape (decorations can be declared
// `parent = true`), which is what makes the rule worth having and worth testing, and
// building the module directly is the only way to test it that cannot go vacuous.
SLANG_UNIT_TEST(irDeferredBodyKeepsDecorationChildren)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    Index expectedChildren = 0;
    Index actualChildren = 0;
    bool bodyWasDeferred = false;
    _roundTripDecorationWithChildren(
        globalSession,
        expectedChildren,
        actualChildren,
        bodyWasDeferred);

    // Guards the premise: if the decoration ever stops being built with children, the
    // comparison below would hold trivially and this test would check nothing.
    SLANG_CHECK_ABORT(expectedChildren == 2);

    // Likewise: an eager load keeps everything, so it says nothing about the rule.
    if (isOnDemandIRLoadEnabled())
        SLANG_CHECK(bodyWasDeferred);

    // The assertion the rule is about. Under the bug this replaces, the decoration comes
    // back with no children at all: they sit at the same depth as body instructions and
    // were skipped along with them, and materializing the body later does not supply them,
    // because the body's encoding starts after the decorations.
    SLANG_CHECK(actualChildren == expectedChildren);
}

// Checks that concurrent first-touch materialization of a deferred body is safe.
//
// A global session is shared across threads and holds the modules whose bodies are
// deferred, so two compiles can reach the same body at once. That is what the loader's
// mutex and the acquire/release publication of a body exist for: a body is built as a
// detached chain and attached with a single release store, and every list traversal loads
// those links with acquire, so a walker sees either no body or a complete one.
//
// The other tests here are single-threaded, which leaves that protocol unexercised.
//
// Deliberately scoped to materialization rather than to whole compiles. Compiling
// concurrently against one shared global session is documented as unsupported --
// `include/slang.h` states a global session is not thread-safe and that front-end work
// requires external synchronization -- and measurably crashes, with on-demand loading
// either on or off. A test shaped that way would be exercising unsupported usage rather
// than this mechanism, and would fail no matter what this PR did.
//
// The concurrency Slang does support is the serial-frontend/parallel-backend workflow in
// docs/user-guide/08-compiling.md, and that is clean here at 16 threads in both modes.
SLANG_UNIT_TEST(irDeferredBodyConcurrentMaterialization)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    Index deferredCount = 0;
    Index mismatches = 0;
    _materializeBodiesConcurrently(globalSession, deferredCount, mismatches);

    // An eager load races nothing, so it would make the assertion below meaningless.
    if (isOnDemandIRLoadEnabled())
        SLANG_CHECK(deferredCount > 0);

    // Every thread must have seen a complete body every time. A body published before its
    // instructions were fully linked shows up here as a short child list.
    SLANG_CHECK(mismatches == 0);
}

// Checks that the concurrency Slang actually supports is where deferred bodies get
// materialized, and that racing on them yields identical output.
//
// This is the counterpart to the test above: that one drives `ensureBodyMaterialized`
// directly on a synthetic module, which proves the protocol works but not that anything
// real depends on it. This one runs the documented serial-frontend/parallel-backend
// workflow from docs/user-guide/08-compiling.md -- load, specialize and `link()` on one
// thread, then call `getEntryPointCode()` from many -- and asserts that the parallel phase
// is where first touches happen.
//
// That assertion is the one that keeps the loader's mutex honest. If linking ever starts
// materializing everything eagerly, the concurrent first touch stops occurring, and the
// justification for the lock quietly becomes false without any test noticing. Measured
// when written: zero materializations during the front end, and 38 (1 thread) rising to 57
// (16 threads) during the backend, the excess being threads that all observed the deferred
// flag before any had finished.
SLANG_UNIT_TEST(irDeferredBodyMaterializesOnTheSupportedConcurrentPath)
{
    if (!isOnDemandIRLoadEnabled())
        return; // Nothing is deferred, so there is nothing to observe.

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // ---- serial front end: everything up to and including link() ----
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    static const char* kSource = R"(
interface IScale { float apply(float v); }
struct Doubler : IScale { float apply(float v) { return v * 2.0f; } }
float scaleAll<T : IScale>(T s, float v) { return s.apply(v); }
RWStructuredBuffer<float> gOut;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m = float4x4(1.0f);
    float3 v = normalize(float3(1.0f, 2.0f, 3.0f));
    Doubler d;
    gOut[tid.x] = scaleAll(d, dot(v, mul(m, float4(v, 1.0f)).xyz)) + sqrt(abs(v.y));
}
)";

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "supportedConcurrentPath",
        "supportedConcurrentPath.slang",
        kSource,
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composed;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(components, 2, composed.writeRef()) == SLANG_OK);
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(composed->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    const Index afterLink = getDeferredBodyMaterializationCount();

    // ---- parallel back end: the one concurrent use the API documents as supported ----
    const int kThreadCount = 8;
    List<String> outputs;
    outputs.setCount(kThreadCount);
    List<uint8_t> succeeded;
    succeeded.setCount(kThreadCount);
    ::memset(succeeded.getBuffer(), 0, size_t(kThreadCount));

    std::atomic<bool> go{false};
    List<std::thread> threads;
    for (int i = 0; i < kThreadCount; i++)
    {
        threads.add(std::thread(
            [&, i]()
            {
                while (!go.load(std::memory_order_acquire))
                    std::this_thread::yield();
                ComPtr<slang::IBlob> code;
                ComPtr<slang::IBlob> diag;
                if (linked->getEntryPointCode(0, 0, code.writeRef(), diag.writeRef()) != SLANG_OK ||
                    !code)
                {
                    return;
                }
                outputs[i] = String((const char*)code->getBufferPointer());
                succeeded[i] = 1;
            }));
    }
    go.store(true, std::memory_order_release);
    for (auto& t : threads)
        t.join();

    const Index duringBackend = getDeferredBodyMaterializationCount() - afterLink;

    for (int i = 0; i < kThreadCount; i++)
    {
        SLANG_CHECK(succeeded[i] != 0);
        SLANG_CHECK(outputs[i] == outputs[0]);
    }
    SLANG_CHECK(outputs[0].getLength() > 0);

    // The point of the test: first touches happen on the concurrent side, so the loader's
    // lock is guarding a path that is really taken.
    SLANG_CHECK(duringBackend > 0);
}

// Checks the two paths that decline deferral, and that declining changes nothing but cost.
//
// Deferral is skipped when the caller supplies no blob, and when the blob it supplies does
// not back the flat table's spans. Neither was exercised by anything that asserted the
// outcome, and the second is the one that matters: it is what stands between a caller
// passing the wrong buffer and a use-after-free surfacing somewhere unrelated. It is also
// invisible from outside, so `getDeferralDeclinedForSpanMismatchCount()` exists to make the
// decision observable — a check that silently stopped rejecting would otherwise look
// exactly like one that had nothing to reject.
SLANG_UNIT_TEST(irDeferralDeclinesWhenTheBlobDoesNotBackTheSpans)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    struct Case
    {
        BlobMode blobMode;
        const char* what;
        bool expectDeferral;
        bool expectMismatchCounted;
    };
    const Case cases[] = {
        {BlobMode::Matching, "matching blob", true, false},
        {BlobMode::Null, "no blob", false, false},
        {BlobMode::Mismatched, "mismatched blob", false, true},
    };

    Index referenceInstCount = 0;
    for (const Case& testCase : cases)
    {
        bool deferred = false;
        Index instCount = 0;
        Index mismatchDelta = 0;
        _roundTripWithBlobMode(
            globalSession,
            testCase.blobMode,
            deferred,
            instCount,
            mismatchDelta);

        SLANG_CHECK_ABORT(instCount > 0);
        if (testCase.blobMode == BlobMode::Matching)
        {
            referenceInstCount = instCount;
            // Guards the premise: if deferral stopped happening for the matching blob, the
            // other two cases would agree with it trivially and prove nothing.
            if (isOnDemandIRLoadEnabled())
                SLANG_CHECK(deferred == testCase.expectDeferral);
        }
        else
        {
            SLANG_CHECK(!deferred);
            // The whole point of the fallback: declining costs time, never contents.
            SLANG_CHECK(instCount == referenceInstCount);
        }

        // Only the mismatched case should trip the containment check. A null blob is
        // refused earlier, before there is anything to compare.
        if (isOnDemandIRLoadEnabled())
            SLANG_CHECK((mismatchDelta > 0) == testCase.expectMismatchCounted);
    }
}


// Checks that a mutation reaching a global whose body is still encoded neither destroys
// that body nor mistakes the decorations for the whole child list.
//
// Decoding a deferred body happens in exactly one kind of place: the accessors that hand
// back a link -- `getNextInst`, `getPrevInst`, `getFirstDecorationOrChild`,
// `getLastDecorationOrChild`. Nothing in the mutation path decodes, deliberately, and
// nothing exercised either half of that arrangement.
//
// Both halves fail silently. Decode too late -- read a neighbour off an undecoded parent,
// then decode in the middle of the splice -- and the last decoration's `next` reads as
// null, the splice republishes itself as the parent's last child, and the body decoded a
// moment earlier is orphaned with no crash and no diagnostic; that is the shape this test
// was written against, and removing the decode from `getNextInst`/`getPrevInst` while
// leaving one in `_insertAt` makes it fail. Never decode at all and the body survives, but
// `insertAtEnd` appends after the last *decoration* instead of after the body, which is
// why the position of the spliced block is checked and not only the body's size.
//
// Built rather than compiled, for the reason the neighbouring tests give: the mutation has
// to run inside the window where the body is still encoded, and reaching that window
// through a compile means racing whichever pass decodes first.
SLANG_UNIT_TEST(irDeferredBodySurvivesMutationOfItsParent)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    struct Case
    {
        DeferredParentMutation mutation;
        /// The global's children afterwards, and where the spliced block belongs among
        /// them: at the head for a splice after the last decoration, and past the body --
        /// not in front of it -- for an append.
        Index expectedChildCount;
        Index expectedSplicedChildIndex;
        Index expectedDecorationCount;
    };
    static const Case kCases[] = {
        {DeferredParentMutation::InsertAfterLastDecoration, 2, 0, 1},
        {DeferredParentMutation::InsertAtEnd, 2, 1, 1},
        {DeferredParentMutation::RemoveLastDecoration, 1, -1, 0},
    };

    for (const auto& testCase : kCases)
    {
        const DeferredParentMutationResult result =
            _mutateGlobalWithDeferredBody(globalSession, testCase.mutation);

        // Guards the premise: with no body to lose, every check below holds trivially.
        SLANG_CHECK_ABORT(result.expectedBodyInsts > 0);
        // Likewise, an eager load has nothing still encoded when the mutation runs, so it
        // says nothing about any of this.
        if (isOnDemandIRLoadEnabled())
            SLANG_CHECK(result.bodyWasDeferred);

        SLANG_CHECK(result.actualBodyInsts == result.expectedBodyInsts);
        SLANG_CHECK(result.childCount == testCase.expectedChildCount);
        SLANG_CHECK(result.splicedChildIndex == testCase.expectedSplicedChildIndex);
        SLANG_CHECK(result.decorationCount == testCase.expectedDecorationCount);
    }
}
