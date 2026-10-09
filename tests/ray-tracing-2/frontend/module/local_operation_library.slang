//TEST_IGNORE_FILE:

module local_operation_library;

// These syntax declarations are ordinary module members. Their checked AST identities must
// survive serialization even when neither core nor the ray-tracing module supplies their names.
__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingTrace] : RayTracingTraceAttribute;

__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingCallShader] : RayTracingCallShaderAttribute;

__attributeTarget(ParamDecl)
internal attribute_syntax [__RayTracingPayload] : RayTracingPayloadAttribute;

[__RayTracingTrace]
public void traceWithoutPayload() {}

[__RayTracingTrace]
public void traceWithPayload([__RayTracingPayload] inout uint payload) {}

[__RayTracingCallShader]
public void callShader() {}
