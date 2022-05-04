// slang-ir-liveness.h
#ifndef SLANG_IR_GLSL_LIVENESS_H
#define SLANG_IR_GLSL_LIVENESS_H

namespace Slang
{

	/// Converts liveness markers into SPIR-V extension
void applyGLSLLiveness(IRModule* module);

}

#endif // SLANG_IR_LIVENESS_H