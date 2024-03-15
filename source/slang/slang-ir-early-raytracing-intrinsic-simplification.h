// slang-ir-early-raytracing-intrinsic-simplification.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-util.h"

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;
    class DiagnosticSink;
    class TargetProgram;

    void replaceLocationIntrinsicsWithRaytracingObject(TargetProgram* target, IRModule* module, DiagnosticSink* sink);
}