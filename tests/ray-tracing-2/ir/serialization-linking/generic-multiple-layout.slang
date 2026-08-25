//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/generic-layout-module.slang -o tests/ray-tracing-2/ir/serialization-linking/structural_generic_layout.slang-module
//TEST:SIMPLE(filecheck=UINT): -experimental-feature -target hlsl -profile sm_6_5 -entry mainUint -stage raygeneration
//TEST:SIMPLE(filecheck=FLOAT): -experimental-feature -target spirv-assembly -emit-spirv-directly -entry mainFloat -stage raygeneration

import slang.raytracing;
import structural_generic_layout;

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<GenericProgramLayout<uint>> uintProgram;
rt::TraceProgramDescriptor<GenericProgramLayout<float>> floatProgram;

void traceValue<T>(
    rt::TraceProgramDescriptor<GenericProgramLayout<T>> program,
    inout GenericPayload<T> payload)
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    rt::RayTracer<GenericProgramLayout<T>> tracer;
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

// UINT: [shader("raygeneration")]void mainUint()
// UINT: [shader("miss")]void GenericMiss(
// UINT-NOT: mainFloat
// FLOAT: OpEntryPoint RayGenerationKHR %mainFloat "main"
// FLOAT: OpEntryPoint MissKHR {{.*}} "GenericMiss"
// FLOAT-NOT: "mainUint"
