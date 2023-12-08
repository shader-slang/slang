#pragma once

#include "slang-ir.h"

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;

    /// Lower tuple types to ordinary `struct`s.
    void lowerGLSLShaderStorageBufferObjects(
        IRModule* module,
        DiagnosticSink* sink);

}
