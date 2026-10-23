#pragma once

#include "slang-ir.h"

namespace Slang
{
struct CodeGenContext;
struct IRModule;
class DiagnosticSink;

IRTextureTypeBase* isCombinedTextureSamplerType(IRInst* typeInst);

// Lower combined texture sampler types to structs.
void lowerCombinedTextureSamplers(
    IRModule* module,
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink);
} // namespace Slang
