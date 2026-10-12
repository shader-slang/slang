//TEST_IGNORE_FILE:

module local_operation_library;

// This syntax declaration is an ordinary module member. Its checked AST identity must survive
// serialization even when neither core nor the ray-tracing module supplies its name.
__attributeTarget(FuncDecl)
internal attribute_syntax [__RayTracingCallShader] : RayTracingCallShaderAttribute;

[__RayTracingCallShader]
public void callShader() {}
