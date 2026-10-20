//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Internal ray-tracing roles and operation attributes belong to the ray-tracing module.
// A client that has not imported it must not get these declarations from core.
[__RayTracingCallShader]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^^^ unknown attribute
 ^^^^^^^^^^^^^^^^^^^^^^ unknown attribute '__RayTracingCallShader'
*/
void callableOperation() {}

static const int stageContextRole = __RayTracingBuiltin.StageContext;
/*CHECK:
                                    ^^^^^^^^^^^^^^^^^^^ undefined identifier
                                    ^^^^^^^^^^^^^^^^^^^ undefined identifier '__RayTracingBuiltin'.
*/
