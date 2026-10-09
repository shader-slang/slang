// slang-ir-optix-entry-point-uniforms.h
#pragma once

namespace Slang
{

class DiagnosticSink;
struct IRModule;
void collectOptiXEntryPointUniformParams(IRModule* module);

/// Replace each module-scope shader-record constant buffer, such as
/// `layout(shaderRecordEXT) ConstantBuffer<T> g;`, with reads of the running OptiX program's shader
/// binding table (SBT) record through `optixGetSbtDataPointer()`. Report an error for each entry
/// point that reaches such a buffer but is not a ray tracing stage, because only ray tracing
/// programs have an SBT record.
///
/// The module must already be specialized, and `static` initializers must already have been moved
/// into the entry points by `moveGlobalVarInitializationToEntryPoints`, so the reference graph
/// includes the shader-record reads each entry point runs. Reads through a buffer handle that
/// escapes into a value are kept off `__ldg` by `lowerImmutableBufferLoadForCUDA`.
void lowerShaderRecordGlobalParamsForOptiX(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
