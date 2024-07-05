#pragma once

#include "slang-ir.h"
#include "slang-compiler.h"

namespace Slang
{
    class DiagnosticSink;

    void legalizeExtractFromTextureAccess(IRModule* module);
}
