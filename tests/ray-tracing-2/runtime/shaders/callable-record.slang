//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=METAL_CALLABLE): -experimental-feature -entry RuntimeCallable -stage callable -target metal

// HLSL-DAG: [shader("raygeneration")]void main()
// HLSL-DAG: CallShader(0U,
// HLSL-DAG: [shader("callable")]void RuntimeCallable(
// SPIRV-DAG: OpEntryPoint RayGenerationKHR {{.*}} "main"
// SPIRV-DAG: OpEntryPoint CallableKHR {{.*}} "RuntimeCallable"
// SPIRV: OpExecuteCallableKHR %uint_0
// METAL-DAG: metal::visible_function_table<void(RuntimeCallableData_0 thread*, uint3, uint3, uint constant*, uint device*)> callableFunctions_0;
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeCallable_0(
// METAL_CALLABLE: void RuntimeCallable_invoke_0(RuntimeCallableRecord_0 thread* record_0, RuntimeCallableData_0 thread* data_0)

import slang.raytracing;

struct RuntimePayload
{
    uint value;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias Payload = RuntimePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct RuntimeCallableData
{
    uint value;
    uint dispatchIndex;
    uint dispatchWidth;
}

struct RuntimeCallableRecord
{
    uint factor;
}

struct RuntimeCallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias CallableData = RuntimeCallableData;
    typealias Record = RuntimeCallableRecord;
}

struct RuntimeCallable : rt::ICallableShader<RuntimeCallableContext>
{
    void invoke(rt::CallableInput<RuntimeCallableContext> input)
    {
        input.data.value = input.data.value * input.record.factor + 1;
        input.data.dispatchIndex = input.dispatchRaysIndex.x;
        input.data.dispatchWidth = input.dispatchRaysDimensions.x;
    }
}

struct CallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = RuntimeCallableContext;
    typealias Callable = RuntimeCallable;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::NoHitGroups<RuntimeTraceContext>;
    typealias MissGroups = rt::NoMissGroups<RuntimeTraceContext>;
    typealias CallableGroups = rt::CallableGroupList<RuntimeTraceContext, CallableGroup>;
}

struct RuntimeResult
{
    uint value;
    uint dispatchWidth;
}

rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void main()
{
    RuntimeCallableData data = { 3, 0, 0 };
    rt::RayTracer<ProgramLayout> tracer;
    tracer.callShader<RuntimeCallableContext>(0, program, data);

    results[0].value = data.value;
    results[0].dispatchWidth = data.dispatchWidth;
}
