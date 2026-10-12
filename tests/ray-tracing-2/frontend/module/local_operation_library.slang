//TEST_IGNORE_FILE:

module local_operation_library;

// These syntax declarations are ordinary module members. Their checked AST identities must
// survive serialization even when neither core nor the ray-tracing module supplies their names.
__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingTrace] : RayTracingTraceAttribute;

__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingCallShader] : RayTracingCallShaderAttribute;

internal struct EmptyPayload
{}

// An ordinary wrapper must keep the annotated operation reachable, including after serialization.
public void trace()
{
    EmptyPayload payload = {};
    trace(payload);
}

[__RayTracingTrace]
public void trace<Payload>(inout Payload payload) {}

[__RayTracingCallShader]
public void callShader() {}
