#pragma once

#include "slang-hgbc.h"
#include "slang-ir-insts.h"

namespace Slang
{
class TargetProgram;

/// Returns whether ownership-preserving host representations are required for this target.
bool usesHGBCOwnedErasure(TargetProgram* targetProgram);

/// Resolves the host representation annotation through the generated HGBC opcode table.
bool tryGetHGBCOpaqueTypeOpcode(
    IRInst* inst,
    CapabilitySet const& targetCaps,
    HGBCOpcode& outOpcode);

/// Returns the opaque carrier for an owned concrete host value.
IRType* getHGBCBoxType(IRBuilder& builder);

/// Copies a concrete value into an immutable box, or preserves an existing box.
IRInst* emitHGBCBox(IRBuilder& builder, IRInst* value);

/// Converts an erased host payload without discarding its dynamic ownership information.
IRInst* emitHGBCPayloadConversion(IRBuilder& builder, IRType* type, IRInst* value);

/// Replaces symbolic payload marshalling after the producer's types have been lowered.
void lowerHGBCOwnedPayloads(IRModule* module);

/// Rejects byte packing of ownership-bearing values after semantic erasure has been lowered.
void validateHGBCOrdinaryBytePacking(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);
} // namespace Slang
