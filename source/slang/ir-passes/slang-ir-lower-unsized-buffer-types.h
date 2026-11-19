// slang-ir-lower-unsized-buffer-types.h
#pragma once

namespace Slang
{
struct IRModule;

/// Replaces unsized buffer IR types with their sized counterparts on non-CPU/CUDA targets.
/// For example, HLSL emits StructuredBuffer<T> for both, while Metal emits device pointers.
/// Sized-to-unsized conversion becomes an identity on these targets. CPU/CUDA must retain
/// distinct types because their unsized buffers omit the count field.
void lowerUnsizedBufferTypesToResources(IRModule* module);

} // namespace Slang
