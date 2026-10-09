implementing raytracing;

// Generated from the compiler's KnownBuiltinDeclName values. The enum is exported so another
// module can declare its own stage contract using the same annotation roles, without copying IDs.
#include "raytracing-builtins.slang"

namespace rt
{

// Structural operations carry their identity through ordinary AST and IR serialization.
// These internal attributes do not require a special compilation mode or import hook.
__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingTrace] : RayTracingTraceAttribute;

__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingCallShader] : RayTracingCallShaderAttribute;

__attributeTarget(ParamDecl)
internal attribute_syntax [__RayTracingPayload] : RayTracingPayloadAttribute;

} // namespace rt
