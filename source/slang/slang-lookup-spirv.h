#include "../core/slang-string.h"

#include "spirv/unified1/spirv.h"
#include "spirv/unified1/GLSL.std.450.h"

namespace Slang
{
bool lookupSpvOp(const UnownedStringSlice& str, SpvOp& value);
bool lookupGLSLstd450(const UnownedStringSlice& str, GLSLstd450& value);
}
