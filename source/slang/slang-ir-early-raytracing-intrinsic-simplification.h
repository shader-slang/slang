// slang-ir-early-raytracing-intrinsic-simplification.h
#pragma once

#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;
class DiagnosticSink;

/// Replace SPIR-V ray-location operands with the global objects carrying matching decorations.
///
/// The caller should invoke this pass only when `RequiredLoweringPassSet` observed a location
/// operand. Each operand is resolved by role and integer location; a missing matching global is
/// diagnosed and replaced with an error-recovery value so malformed IR cannot reach emission.
void replaceLocationIntrinsicsWithRaytracingObject(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
