//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target hlsl -profile sm_6_5

import slang.raytracing;

void traceOpenSchema<Schema>(
    rt::RayTracer<Schema> tracer,
    Schema.TraceContext.AccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program)
    where Schema : rt::ITraceProgramSchema
    where Schema.TraceContext.AccelerationStructure == rt::AccelerationStructure
    where Schema.TraceContext.Motion == rt::NoMotion
{
    tracer.trace({}, scene, program);
//CHECK:        ^ structural ray-tracing schema is not concrete and closed
//CHECK:        ^ the three-argument 'trace' overload requires a concrete closed schema, but schema 'Schema' has an unresolved hit-group or miss-shader payload type
}
