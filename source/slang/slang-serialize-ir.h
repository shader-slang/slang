#pragma once

#include "core/slang-smart-pointer.h"
#include "slang-com-helper.h"
#include "slang-ir.h"

namespace Slang
{

struct IRModule;
class Session;
class SerialSourceLocReader;
class SerialSourceLocWriter;
class String;
namespace RIFF
{
struct BuildCursor;
struct Chunk;
} // namespace RIFF

void writeSerializedModuleIR(
    RIFF::BuildCursor& cursor,
    IRModule* moduleDecl,
    SerialSourceLocWriter* sourceLocWriter);

/// Reads an IR module out of `chunk`.
///
/// `blobHoldingSerializedData` is the blob those bytes live in, or null if the caller
/// read them from storage it owns itself. It matters because instruction bodies can be
/// left encoded and decoded on demand, out of spans that point into these bytes rather
/// than copies of them: when a blob is supplied it is retained for as long as bodies can
/// still be decoded, and when it is not, bodies are loaded eagerly instead. Passing null
/// is therefore always safe, and never wrong -- only slower.
[[nodiscard]] Result readSerializedModuleIR(
    RIFF::Chunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    ISlangBlob* blobHoldingSerializedData,
    RefPtr<IRModule>& outIRModule);

/// True if instruction bodies are left encoded until something reads them.
///
/// On by default; `SLANG_ONDEMAND_IR=0` forces the eager load. Exported so that tests
/// can ask the same question the loader asks instead of reimplementing the rule — three
/// copies of "on unless explicitly 0" had already appeared, and a test whose copy drifts
/// from this one stops testing the mode it believes it is testing.
SLANG_API bool isOnDemandIRLoadEnabled();

//
// Test-only entry points.
//
// These live here rather than in `slang-ir.h` because they are about serialization, and
// they exist at all because the shapes they exercise cannot be reached from the unit-test
// tool: the IR builders and the serialization entry points are not exported, and two of
// these shapes do not occur in any module the compiler produces. Exporting all of that to
// test three rules would be a far larger surface than three functions.
//

/// What blob `_testDeferralFallback` hands the reader. Only `Matching` permits deferral;
/// the other two must fall back to an eager load and produce an identical module.
enum class TestBlobMode
{
    /// The blob the bytes were parsed out of.
    Matching,
    /// No blob, which is what a caller reading from its own buffer supplies.
    Null,
    /// An identical copy at a different address -- the shape `addLibraryReference` had.
    Mismatched,
};

/// Round-trips a module with a chosen blob mode, reporting whether deferral was taken
/// (`outDeferredLoaderInstalled`), what was loaded (`outInstCount`, which must match across
/// modes), and whether the containment check fired (`outSpanMismatchDelta`).
SLANG_API void _testDeferralFallback(
    slang::IGlobalSession* globalSession,
    TestBlobMode blobMode,
    bool& outDeferredLoaderInstalled,
    Index& outInstCount,
    Index& outSpanMismatchDelta);

/// Round-trips a module whose decoration has children of its own. A deferred load that
/// dropped the decoration's subtree shows up as `outActualChildren < outExpectedChildren`.
/// `outBodyWasDeferred` distinguishes a real exercise of the rule from an eager load.
SLANG_API void _testRoundTripDecorationWithChildren(
    slang::IGlobalSession* globalSession,
    Index& outExpectedChildren,
    Index& outActualChildren,
    bool& outBodyWasDeferred);

/// Races many threads to first-touch the same deferred bodies. `outMismatches` counts
/// threads that saw a body with the wrong instruction count -- what a torn or partially
/// published body looks like. `outDeferredCount` distinguishes a real race from an eager
/// load, which races nothing.
///
/// Scoped to materialization deliberately: running whole compiles concurrently on a shared
/// global session is documented as unsupported and crashes either way, so a test shaped
/// that way would exercise unsupported usage rather than this mechanism.
SLANG_API void _testConcurrentBodyMaterialization(
    slang::IGlobalSession* globalSession,
    Index& outDeferredCount,
    Index& outMismatches);

[[nodiscard]] Result readSerializedModuleInfo(
    RIFF::Chunk const* chunk,
    String& compilerVersion,
    UInt& version,
    String& name);

// Enable a mild optimization by putting instructions with payloads at the end
// of the stream to make deserialization slightly faster
const bool kReorderInstructionsForSerialization = true;

// Recursive IR tree traversal is used on both write and read. This matches the
// existing IR specialization depth budget and is shared so round-trips stay symmetric.
const Int64 kMaxIRSerializationDepth = 512;

// We expose this function here as it's used by the verifyIRSerialize function in
// slang-serialize-container.cpp
template<typename Func>
static void traverseInstsInSerializationOrder(IRInst* moduleInst, Func&& processInst)
{
    const auto go = [&](auto& go, IRInst* inst, Int64 depth) -> void
    {
        SLANG_RELEASE_ASSERT(depth < kMaxIRSerializationDepth);

        // Process the current instruction
        processInst(inst);

        //
        // Process the children
        //
        // To make things slightly easier for the branch predictor, if this
        // is a module instruction move all the special case
        // instructions (bool/int/float literals and string literals)
        // to the end. It is semantically the same, but it means that
        // the control flow when reading will be easier to predict.
        //
        if (kReorderInstructionsForSerialization && inst->m_op == kIROp_ModuleInst) [[unlikely]]
        {
            List<IRInst*> lits;
            List<IRInst*> strings;
            for (const auto c : inst->getDecorationsAndChildren())
            {
                if (c->m_op == kIROp_BoolLit || c->m_op == kIROp_IntLit ||
                    c->m_op == kIROp_FloatLit || c->m_op == kIROp_PtrLit ||
                    c->m_op == kIROp_VoidLit)
                {
                    lits.add(c);
                }
                else if (c->m_op == kIROp_StringLit || c->m_op == kIROp_BlobLit)
                {
                    strings.add(c);
                }
                else
                {
                    go(go, c, depth + 1);
                }
            }
            for (const auto c : lits)
            {
                go(go, c, depth + 1);
            }
            for (const auto c : strings)
            {
                go(go, c, depth + 1);
            }
        }
        else
        {
            for (const auto c : inst->getDecorationsAndChildren())
            {
                go(go, c, depth + 1);
            }
        }
    };
    go(go, moduleInst, 0);
}

} // namespace Slang
