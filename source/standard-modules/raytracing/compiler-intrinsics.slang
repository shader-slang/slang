implementing raytracing;

// Generated from the compiler's KnownBuiltinDeclName values. These annotations identify library
// declarations that the compiler uses when preparing types for backend code generation. For
// example, Context leads to the Payload and Record types needed by a generated stage function.
// Looking up those requirements by name would also work, but would tie the compiler to their
// spelling. Keeping the StageContext annotation when renaming Context to ShaderContext lets the
// compiler keep the same lookup; shader code using the old name would still need updating.
#include "raytracing-builtins.slang"

namespace rt
{

// Trace's annotation is used only by frontend checking: a structural implementation may call
// native TraceRay without selecting both source API families. Metal dispatch itself is an
// explicit metalTraceRay IR instruction emitted by the internal intrinsic functions.
__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingTrace] : RayTracingTraceAttribute;

// Callable dispatch still carries an operation decoration until it has its own IR instruction.
__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingCallShader] : RayTracingCallShaderAttribute;

} // namespace rt
