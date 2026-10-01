#pragma once

#include "core/slang-smart-pointer.h"
#include "slang-com-helper.h"
#include "slang-ir.h"
#include "slang.h" // `ISlangBlob`, named by `readSerializedModuleIR`

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

/// Reads module metadata without deserializing the IR or checking the semantic module version.
/// `moduleVersion` is required and is written on success. `compilerVersion`, `name`, and
/// `serializationVersion` are optional. `serializationVersion` is written as soon as the metadata
/// record is available, including when an unsupported format causes `SLANG_E_NOT_AVAILABLE`;
/// `compilerVersion` and `name` are written only on success. A well-formed metadata record with a
/// null module pointer returns `SLANG_FAIL`. A non-data chunk or missing Fossil root triggers
/// `SLANG_UNEXPECTED`. The distinct unsupported-format result lets metadata callers issue a
/// specific diagnostic before attempting IR deserialization. This path decodes no instruction
/// bodies, so the on-demand deferral `readSerializedModuleIR` performs never applies here.
[[nodiscard]] Result readSerializedModuleInfo(
    RIFF::Chunk const* chunk,
    String* compilerVersion,
    UInt64& moduleVersion,
    String* name,
    UInt64* serializationVersion = nullptr);

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
