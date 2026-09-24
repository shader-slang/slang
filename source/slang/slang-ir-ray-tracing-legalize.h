// slang-ir-ray-tracing-legalize.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{

class TargetProgram;
struct IRModule;
struct IRSPIRVAsmInst;
struct IRStructType;

/// Record D3D payload boundaries before general type legalization.
///
/// Resolve the existing struct-only argument markers and retain their payload role on native
/// intrinsic parameters. Record receiving payload parameters from entry-point signatures too,
/// and normalize access qualifiers on nonempty D3D ray-payload structs. This pass does not predict
/// whether a source type is empty or pad it: general type legalization decides which logical
/// values disappear and materializes a physical payload only at a required target boundary.
/// Ordinary helper parameters and nested empty fields remain free to disappear.
void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram);

/// Create the target representation for a payload whose logical data legalized to none.
///
/// Direct SPIR-V uses a zero-member struct; HLSL/DXIL and GLSL use one dummy int. The result is
/// marked as a physical payload so later legalization does not erase it again. Ray payloads also
/// receive D3D ray-payload and SM 6.7 access qualifiers when required. Callers cache these types
/// by payload role; this function neither changes the source type nor preserves any logical data.
/// CUDA/OptiX does not require this representation and must not call this helper.
IRStructType* createEmptyRayTracingPayloadType(
    IRModule* module,
    TargetProgram* targetProgram,
    bool isRayPayload);

/// Return the IR operand index of a required SPIR-V ray/callable payload, or -1 for other opcodes.
/// On success, isRayPayload distinguishes ray payloads from callable data. This classifies the
/// actual dispatch instruction, so empty arguments in arbitrary ordinary helpers may disappear
/// without moving physical payload storage through their signatures.
Index getSPIRVRayTracingPayloadOperandIndex(IRSPIRVAsmInst* inst, bool& isRayPayload);

} // namespace Slang
