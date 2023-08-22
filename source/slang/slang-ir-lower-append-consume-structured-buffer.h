// slang-ir-lower-append-consume-structured-buffer.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;
    class TargetRequest;

    /// For non-hlsl targets, lower append- and consume- structured buffers into `struct` types
    /// that contains two RWStructuredBuffer typed fields, one to store the elements, and one
    /// for the atomic buffer.
    void lowerAppendConsumeStructuredBuffers(TargetRequest* target, IRModule* module, DiagnosticSink* sink);

}
