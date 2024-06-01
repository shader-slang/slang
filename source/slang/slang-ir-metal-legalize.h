#pragma once

#include "slang-ir.h"

namespace Slang
{
    class DiagnosticSink;

    void legalizeDispatchMeshPayloadForMetal(IRModule* module);

    void legalizeIRForMetal(IRModule* module, DiagnosticSink* sink);
}
