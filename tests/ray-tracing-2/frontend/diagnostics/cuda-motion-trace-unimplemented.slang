//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -target cuda -whole-program
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -target cuda -whole-program -DTEST_NO_PAYLOAD

import slang.raytracing;

// An exported generic helper reaches the real trace implementation without selecting a structural
// entry point or requiring a concrete descriptor's target layout. This lets the pending CUDA
// motion branch report its own assertion. Check each overload independently: the no-payload
// wrapper must forward to the same implementation instead of silently doing nothing.
#ifdef TEST_NO_PAYLOAD
[require(cuda, structural_raytracing_trace)]
export void traceWithoutPayload<Schema : rt::ITraceProgramSchema>(
    rt::RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program)
    where Schema.TraceContext.Motion == rt::InstanceMotion
{
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program);
}
#else
[require(cuda, structural_raytracing_trace)]
export void traceWithPayload<Schema : rt::ITraceProgramSchema, Payload>(
    rt::RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program,
    inout Payload payload)
    where Schema.TraceContext.Motion == rt::InstanceMotion
{
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}
#endif

// The packaged module does not retain the assertion message.
// CHECK: error[E41400]: static assertion failed
