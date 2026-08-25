//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -no-codegen

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = ::Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ProgramLayout : rt::ITraceProgramLayout
/*CHECK:
       ^^^^^^^^^^^^^ the same module also uses the structural API
*/
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups<::TraceContext>;
    typealias MissGroups = rt::NoMissGroups<::TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<::TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;

void legacyTrace(rt::RayTraversalDesc desc, inout Payload payload)
/*CHECK:
     ^^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^^ this declaration uses the legacy API
*/
{
    TraceRay(scene, 0, 0xff, 0, 0, 0, desc.ray, payload);
}

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};

    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    legacyTrace(desc, payload);
}
