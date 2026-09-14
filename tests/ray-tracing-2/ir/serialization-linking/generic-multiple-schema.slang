//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/serialization-linking/generic-schema-module.slang -o tests/ray-tracing-2/ir/serialization-linking/structural_generic_schema.slang-module
//TEST:SIMPLE(filecheck=UINT): -experimental-feature -target hlsl -profile sm_6_5 -entry mainUint -stage raygeneration
//TEST:SIMPLE(filecheck=FLOAT): -experimental-feature -target spirv-assembly -emit-spirv-directly -entry mainFloat -stage raygeneration
//TEST:SIMPLE(filecheck=BOTH): -experimental-feature -target hlsl -profile sm_6_5 -entry mainBoth -stage raygeneration
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -target metal -entry mainUint -stage raygeneration
//TEST:SIMPLE(filecheck=METAL_BOTH): -experimental-feature -target metal -entry mainBoth -stage raygeneration
//TEST:SIMPLE(filecheck=BOTH_IR): -experimental-feature -target hlsl -profile sm_6_5 -entry mainBoth -stage raygeneration -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=LOWERED_IR): -experimental-feature -target hlsl -profile sm_6_5 -entry mainBoth -stage raygeneration -dump-ir-after lowerStructuralRayTracingProgramDescriptorTypes

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
    tracer.trace(desc, scene, preserveProgram<T>(program), payload);
}

[shader("raygeneration")]
void mainUint()
{
    GenericPayload<uint> payload = {1};
    traceValue(uintProgram, payload);
}

[shader("raygeneration")]
void mainFloat()
{
    GenericPayload<float> payload = {2.0};
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

    GenericPayload<uint> uintPayload = {1};
    rt::RayTracer<GenericSchema<uint>> uintTracer;
    uintTracer.trace(desc, scene, preserveProgram<uint>(uintProgram), uintPayload);

    GenericPayload<float> floatPayload = {2.0};
    rt::RayTracer<GenericSchema<float>> floatTracer;
    floatTracer.trace(desc, scene, preserveProgram<float>(floatProgram), floatPayload);
}

// UINT: [shader("raygeneration")]void mainUint()
// UINT: [shader("miss")]void GenericMiss_Sh{{[0-9A-Fa-f]+}}(
// UINT-NOT: mainFloat
// FLOAT: OpEntryPoint RayGenerationKHR %mainFloat "main"
// FLOAT: OpEntryPoint MissKHR {{.*}} "GenericMiss_Sh{{[0-9A-Fa-f]+}}"
// FLOAT-NOT: "mainUint"
// BOTH: [shader("raygeneration")]void mainBoth()
// BOTH-COUNT-2: [shader("miss")]void GenericMiss_Sh{{[0-9A-Fa-f]+}}(

// The descriptor-bearing generic helper comes from the serialized module. Its emitted Metal
// signature proves that linking retained the schema operand until the consumer selected the
// schema-specific physical descriptor.
// METAL: struct rt_TraceProgramDescriptorResources_default_0
// METAL: metal::raytracing::intersection_function_table<metal::raytracing::instancing> intersectionFunctions_0;
// METAL: void preserveProgram_0(rt_TraceProgramDescriptorResources_default_0 constant* thread*
// METAL: preserveProgram_0(
// METAL: void mainUint(

// Compiling both specializations on Metal proves the
// serialized helper retained enough schema identity to receive two distinct physical descriptor
// types, even though those types have the same source storage shape.
// METAL_BOTH-DAG: struct rt_TraceProgramDescriptorResources_default_0
// METAL_BOTH-DAG: struct rt_TraceProgramDescriptorResources_default_1
// METAL_BOTH-DAG: void preserveProgram_{{[0-9]+}}(rt_TraceProgramDescriptorResources_default_0 constant* thread*
// METAL_BOTH-DAG: void preserveProgram_{{[0-9]+}}(rt_TraceProgramDescriptorResources_default_1 constant* thread*
// BOTH_IR: StructuralRayTracingProgramDescriptor
// BOTH_IR: StructuralRayTracingProgramDescriptor

// The leading and trailing checks cover wrappers emitted before and after the selected function.
// LOWERED_IR-NOT: StructuralRayTracingProgramDescriptor(
// LOWERED_IR: func %mainBoth
// LOWERED_IR-NOT: StructuralRayTracingProgramDescriptor(
