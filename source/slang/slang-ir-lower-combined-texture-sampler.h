#pragma once

#include "slang-ir.h"

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;

    // Lower combined texture sampler types to structs.
    void lowerCombinedTextureSamplers(
        IRModule* module,
        DiagnosticSink* sink
    );
}
