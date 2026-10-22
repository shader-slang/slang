//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// Internal ray-tracing roles belong to the ray-tracing module.
// A client that has not imported it must not get these declarations from core.
static const int stageContextRole = __RayTracingBuiltin.StageContext;
/*CHECK:
                                    ^^^^^^^^^^^^^^^^^^^ undefined identifier
                                    ^^^^^^^^^^^^^^^^^^^ undefined identifier '__RayTracingBuiltin'.
*/
