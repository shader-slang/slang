//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/generic-schema-module.slang -o tests/ray-tracing-2/ir/serialization-linking/structural_generic_schema.slang-module
//TEST:SIMPLE(filecheck=UINT): -experimental-feature -target hlsl -profile sm_6_5 -entry mainUint -stage raygeneration
//TEST:SIMPLE(filecheck=FLOAT): -experimental-feature -target spirv-assembly -emit-spirv-directly -entry mainFloat -stage raygeneration
//TEST:SIMPLE(filecheck=BOTH): -experimental-feature -target hlsl -profile sm_6_5 -entry mainBoth -stage raygeneration

import slang.raytracing;
import structural_generic_schema;

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<GenericSchema<uint>> uintProgram;
rt::TraceProgramDescriptor<GenericSchema<float>> floatProgram;

void traceValue<T>(
    rt::TraceProgramDescriptor<GenericSchema<T>> program,
    inout GenericPayload<T> payload)
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    rt::RayTracer<GenericSchema<T>> tracer;
    tracer.trace(desc, scene, program, payload);
}

[shader("raygeneration")]
void mainUint()
{
    GenericPayload<uint> payload = { 1 };
    traceValue(uintProgram, payload);
}

[shader("raygeneration")]
void mainFloat()
{
    GenericPayload<float> payload = { 2.0 };
    traceValue(floatProgram, payload);
}

// Retain two specializations of the same source stage in one linked program. Their canonical
// source identities are distinct even if their storage-free IR types share one shape, so the
// generated entry-point names must also be distinct.
[shader("raygeneration")]
void mainBoth()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    GenericPayload<uint> uintPayload = { 1 };
    rt::RayTracer<GenericSchema<uint>> uintTracer;
    uintTracer.trace(desc, scene, uintProgram, uintPayload);

    GenericPayload<float> floatPayload = { 2.0 };
    rt::RayTracer<GenericSchema<float>> floatTracer;
    floatTracer.trace(desc, scene, floatProgram, floatPayload);
}

// UINT: [shader("raygeneration")]void mainUint()
// UINT: [shader("miss")]void GenericMiss_Sh{{[0-9A-Fa-f]+}}(
// UINT-NOT: mainFloat
// FLOAT: OpEntryPoint RayGenerationKHR %mainFloat "main"
// FLOAT: OpEntryPoint MissKHR {{.*}} "GenericMiss_Sh{{[0-9A-Fa-f]+}}"
// FLOAT-NOT: "mainUint"
// BOTH: [shader("raygeneration")]void mainBoth()
// BOTH-COUNT-2: [shader("miss")]void GenericMiss_Sh{{[0-9A-Fa-f]+}}(
