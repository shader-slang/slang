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

[[nodiscard]] Result readSerializedModuleIR(
    RIFF::Chunk const* chunk,
    Session* session,
    SerialSourceLocReader* sourceLocReader,
    RefPtr<IRModule>& outIRModule);

[[nodiscard]] Result readSerializedModuleInfo(
    RIFF::Chunk const* chunk,
    String& compilerVersion,
    UInt& version,
    String& name);

// Enable a mild optimization by putting instructions with payloads at the end
// of the stream to make deserialization slightly faster
const bool kReorderInstructionsForSerialization = true;

// We expose this function here as it's used by the verifyIRSerialize function in
// slang-serialize-container.cpp
template<typename Func>
static void traverseInstsInSerializationOrder(IRInst* moduleInst, Func&& processInst)
{
    const auto go = [&](auto& go, IRInst* inst) -> void
    {
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
            for (const auto c : inst->m_decorationsAndChildren)
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
                    go(go, c);
                }
            }
            for (const auto c : lits)
            {
                go(go, c);
            }
            for (const auto c : strings)
            {
                go(go, c);
            }
        }
        else
        {
            for (const auto c : inst->m_decorationsAndChildren)
            {
                go(go, c);
            }
        }
    };
    go(go, moduleInst);
}

} // namespace Slang
