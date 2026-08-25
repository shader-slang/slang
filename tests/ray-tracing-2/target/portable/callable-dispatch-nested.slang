//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -profile glsl_460 -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestCallableData
{
    uint value;
    uint dispatchIndex;
    uint dispatchWidth;
}

struct TestCallableRecord
{
    uint value;
}

struct TestCallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestCallableData;
    typealias Record = TestCallableRecord;
}

void callSecond(inout TestCallableData data);

struct FirstCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input)
    {
        input.data.value += input.record.value;
        callSecond(input.data);
    }
}

struct SecondCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input)
    {
        input.data.value *= input.record.value;
        input.data.dispatchIndex = input.dispatchRaysIndex.x;
        input.data.dispatchWidth = input.dispatchRaysDimensions.x;
    }
}

struct FirstCallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = TestCallableContext;
    typealias Callable = FirstCallable;
}

struct SecondCallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<1>;
    typealias Context = TestCallableContext;
    typealias Callable = SecondCallable;
}

struct TestProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups<TestTraceContext>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups =
        rt::CallableGroupList<TestTraceContext, FirstCallableGroup, SecondCallableGroup>;
}

rt::TraceProgramDescriptor<TestProgramLayout> program;
RWStructuredBuffer<uint> output;

void callSecond(inout TestCallableData data)
{
    rt::RayTracer<TestProgramLayout> tracer;
    tracer.callShader<TestCallableContext>(1, program, data);
}

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestProgramLayout> tracer;
    TestCallableData data = {1, 0, 0};
    tracer.callShader<TestCallableContext>(0, program, data);
    output[0] = data.value;
}

// HLSL-DAG: CallShader(1U,
// HLSL-DAG: [shader("callable")]void FirstCallable
// HLSL-DAG: [shader("callable")]void SecondCallable
// HLSL-DAG: DispatchRaysIndex()
// HLSL-DAG: DispatchRaysDimensions()

// SPIRV-DAG: OpEntryPoint CallableKHR %FirstCallable "FirstCallable"
// SPIRV-DAG: OpEntryPoint CallableKHR %SecondCallable "SecondCallable"
// SPIRV: OpExecuteCallableKHR %uint_1

// METAL: metal::visible_function_table<void(TestCallableData_0 thread*, uint3, uint3, uint constant*, uint device*)> callableFunctions_0;
// METAL: {{\[\[visible\]\]}} void SecondCallable_0(TestCallableData_0 thread* {{data_[0-9]+}}, uint3 {{dispatchRaysIndex_[0-9]+}}, uint3 {{dispatchRaysDimensions_[0-9]+}}, uint constant* {{descriptorResources_[0-9]+}}, uint device* {{descriptorData_[0-9]+}})
// METAL: {{\[\[visible\]\]}} void FirstCallable_0(TestCallableData_0 thread* [[DATA:data_[0-9]+]], uint3 [[INDEX:dispatchRaysIndex_[0-9]+]], uint3 [[DIMENSIONS:dispatchRaysDimensions_[0-9]+]], uint constant* [[RESOURCES:descriptorResources_[0-9]+]], uint device* [[RECORDS:descriptorData_[0-9]+]])
// METAL: ((rt_TraceProgramDescriptorResources_default_0 constant*)([[RESOURCES]]))->callableFunctions_0[1U]([[DATA]], [[INDEX]], [[DIMENSIONS]], (constant uint*)([[RESOURCES]]), [[RECORDS]]);
