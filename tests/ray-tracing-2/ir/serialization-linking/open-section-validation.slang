//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/open-section-context.slang -o tests/ray-tracing-2/ir/serialization-linking/structural_open_section_validation_context.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/open-section-invalid-plugin.slang -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_validation_context.slang-module -o tests/ray-tracing-2/ir/serialization-linking/structural_open_section_invalid_plugin.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_validation_context.slang-module -r tests/ray-tracing-2/ir/serialization-linking/structural_open_section_invalid_plugin.slang-module -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;
import structural_open_section_context;
import structural_open_section_invalid_plugin;

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = SharedTraceContext;
    typealias HitGroups = rt::OpenHitGroups<IHitTag>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::OpenCallableShaders<ICallableTag>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    LinkedPayload payload = {};
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ structural ray-tracing entry uses a different trace context
//CHECK:        ^ entry 'ForeignHitGroup' uses trace context 'ForeignTraceContext', but schema 'Schema' requires 'SharedTraceContext'
//CHECK:        ^ incompatible structural ray-tracing callable data
//CHECK:        ^ callable shader 'SecondCallable' uses data type 'float', which does not match the required callable data type 'uint'
}
