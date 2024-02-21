// slang-ir-lower-generics.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;
    class TargetProgram;

    /// Lower generic and interface-based code to ordinary types and functions using
    /// dynamic dispatch mechanisms.
    void lowerGenerics(
        TargetProgram*          targetReq,
        IRModule*               module,
        DiagnosticSink*         sink);

}
