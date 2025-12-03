// slang-ir-early-raytracing-intrinsic-simplification.h
#pragma once

#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;
class DiagnosticSink;
class TargetProgram;

void replaceLocationIntrinsicsWithRaytracingObject(
    IRModule* module,
    TargetProgram* target,
    DiagnosticSink* sink);
} // namespace Slang