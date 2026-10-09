//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -no-codegen

// Internal ray-tracing roles and operation attributes belong to the ray-tracing module.
// A client that has not imported it must not get these declarations from core.
[__RayTracingTrace]
void traceOperation() {}

[__RayTracingCallShader]
void callableOperation() {}

void payloadParameter([__RayTracingPayload] inout uint value) {}

static const int stageContextRole = __RayTracingBuiltin.StageContext;

// CHECK-DAG: unknown attribute '__RayTracingTrace'
// CHECK-DAG: unknown attribute '__RayTracingCallShader'
// CHECK-DAG: unknown attribute '__RayTracingPayload'
// CHECK-DAG: undefined identifier '__RayTracingBuiltin'
