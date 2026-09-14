// slang-ir-optix-ray-tracing-abi.h

#pragma once

#include "slang-ir.h"

namespace Slang
{

/// Native OptiX register widths and limits used by payload and hit-attribute transport.
static constexpr IRIntegerValue kOptiXRayTracingRegisterSize = 4;
static constexpr IRIntegerValue kOptiXMaxRayPayloadRegisterCount = 32;
static constexpr IRIntegerValue kOptiXIndirectPayloadRegisterCount = 2;
static constexpr IRIntegerValue kOptiXMinHitAttributeRegisterCount = 2;
static constexpr IRIntegerValue kOptiXMaxHitAttributeRegisterCount = 8;

/// Describes the physical OptiX payload transport selected for one source payload type.
struct OptiXRayTracingPayloadABIInfo
{
    /// Number of 32-bit OptiX payload registers passed to each native stage.
    IRIntegerValue registerCount = 0;

    /// Whether those registers contain a pointer instead of the payload's native bytes.
    bool isIndirect = false;
};

/// Computes the OptiX payload transport selected for `type`.
///
/// A payload whose emitted CUDA representation occupies at most 32 words is passed inline. Larger
/// payloads use the two-word pointer representation implemented by the CUDA prelude. An empty
/// payload is inline and consumes zero registers.
Result getOptiXRayTracingPayloadABIInfo(
    IRBuilder* builder,
    IRType* type,
    OptiXRayTracingPayloadABIInfo* outInfo);

/// Reconstructs an inline payload from the ambient OptiX payload registers.
///
/// The byte offsets are the offsets of the emitted CUDA C++ type. Padding is skipped, but its
/// presence still contributes to the register count returned by
/// `getOptiXRayTracingPayloadABIInfo` and therefore matches the prelude's `sizeof(T)` sender.
Result emitOptiXRayTracingPayloadRead(IRBuilder* builder, IRType* type, IRInst** outValue);

/// Writes an inline payload back to the ambient OptiX payload registers.
///
/// Sub-word fields sharing a register are accumulated in SSA and written once, avoiding an
/// assumption that `optixGetPayload_N()` observes an earlier `optixSetPayload_N()` call.
Result emitOptiXRayTracingPayloadWrite(IRBuilder* builder, IRInst* value);

/// Counts the 32-bit OptiX attribute registers required by `type`.
///
/// Hit attributes use a word stream rather than native aggregate byte layout. Each scalar of at
/// most 32 bits occupies one register; a 64-bit scalar occupies a low/high pair. Aggregate padding
/// never consumes an attribute register.
Result getOptiXRayTracingHitAttributeRegisterCount(
    IRBuilder* builder,
    IRType* type,
    IRIntegerValue* outRegisterCount);

/// Reconstructs `type` from the ambient OptiX hit-attribute registers.
///
/// `outRegisterCount` reports the number of registers consumed. This uses the same aggregate walk
/// and scalar encoding as reflection and report-hit lowering, so the producer and consumers cannot
/// disagree about nested structures, arrays, vectors, matrices, enums, or 64-bit values.
Result emitOptiXRayTracingHitAttributeFetch(
    IRBuilder* builder,
    IRType* type,
    IRInst** outValue,
    IRIntegerValue* outRegisterCount);

/// Appends the uint32 OptiX register arguments that transport `value` through report-intersection.
///
/// The generated list uses the same physical order consumed by
/// `emitOptiXRayTracingHitAttributeFetch` and counted by
/// `getOptiXRayTracingHitAttributeRegisterCount`.
Result emitOptiXRayTracingHitAttributeReportArguments(
    IRBuilder* builder,
    IRInst* value,
    List<IRInst*>& outArguments);

} // namespace Slang
