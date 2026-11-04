//DIAGNOSTIC_TEST:SIMPLE(diag=PAYLOAD): -experimental-feature -target cuda -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=NO_PAYLOAD): -experimental-feature -target cuda -whole-program -DTEST_NO_PAYLOAD

import slang.raytracing.portable;

// An exported generic helper reaches the real trace implementation without selecting a structural
// entry point or requiring a concrete descriptor's target layout. This lets the pending CUDA
// motion branch report its own assertion. Check each overload independently: the no-payload
// wrapper must forward to the same implementation instead of silently doing nothing.
// The packaged library reports these errors without source locations, so each message check
// is placed beside the call that reaches the assertion.
#ifdef TEST_NO_PAYLOAD
[require(cuda, portable_raytracing_trace)]
export void traceWithoutPayload<Schema : rt::ITraceProgramSchema>(
    rt::RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program)
    where Schema.TraceContext.Motion == rt::InstanceMotion
{
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program);
//NO_PAYLOAD: error E41400
//NO_PAYLOAD: CUDA structural ray-tracing motion is not implemented
}
#else
[require(cuda, portable_raytracing_trace)]
export void traceWithPayload<Schema : rt::ITraceProgramSchema, Payload>(
    rt::RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program,
    inout Payload payload)
    where Schema.TraceContext.Motion == rt::InstanceMotion
{
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
//PAYLOAD: error E41400
//PAYLOAD: CUDA structural ray-tracing motion is not implemented
}
#endif
