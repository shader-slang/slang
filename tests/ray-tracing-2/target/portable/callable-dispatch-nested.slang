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

struct TestCallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestCallableData;
    typealias Record = TestCallableRecord;
}

void callSecond(inout TestCallableData data);

struct FirstCallable : rt::ICallableShader
{
    typealias Context = TestCallableContext;
    void invoke(rt::CallableInput<TestCallableContext> input)
    {
        input.data.value += input.record.value;
        callSecond(input.data);
    }
}

struct SecondCallable : rt::ICallableShader
{
    typealias Context = TestCallableContext;
    void invoke(rt::CallableInput<TestCallableContext> input)
    {
        input.data.value *= input.record.value;
        input.data.dispatchIndex = input.dispatchRaysIndex.x;
        input.data.dispatchWidth = input.dispatchRaysDimensions.x;
    }
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders =
        rt::CallableShaderList<FirstCallable, SecondCallable>;
}

rt::TraceProgramDescriptor<TestSchema> program;
RWStructuredBuffer<uint> output;

void callSecond(inout TestCallableData data)
{
    rt::RayTracer<TestSchema> tracer;
    tracer.callShader<TestCallableContext>(1, program, data);
}

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestSchema> tracer;
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

// METAL: metal::visible_function_table<void(TestCallableData_0 thread*, uint3, uint3, uchar device*, uint constant*, uint device*, uchar thread*)> callableFunctions_0;
// METAL: {{\[\[visible\]\]}} void {{.*}}(TestCallableData_0 thread* {{data_[0-9]+}}, uint3 {{dispatchRaysIndex_[0-9]+}}, uint3 {{dispatchRaysDimensions_[0-9]+}}, uchar device* {{recordData_[0-9]+}}, uint constant* {{descriptorResources_[0-9]+}}, uint device* {{descriptorData_[0-9]+}}, uchar thread* {{kernelContext_[0-9]+}})
// METAL: {{\[\[visible\]\]}} void {{.*}}(TestCallableData_0 thread* [[DATA:data_[0-9]+]], uint3 [[INDEX:dispatchRaysIndex_[0-9]+]], uint3 [[DIMENSIONS:dispatchRaysDimensions_[0-9]+]], uchar device* [[RECORD_DATA:recordData_[0-9]+]], uint constant* {{descriptorResources_[0-9]+}}, uint device* {{descriptorData_[0-9]+}}, uchar thread* [[CONTEXT:kernelContext_[0-9]+]])
// METAL: device uchar* [[CALLABLE_RECORD:_slang_callable_record]] =
// METAL: uint [[FUNCTION_INDEX:_slang_callable_function_index]] =
// METAL: callableFunctions_0{{\[}}[[FUNCTION_INDEX]]{{\]}}([[DATA]], [[INDEX]], [[DIMENSIONS]], [[CALLABLE_RECORD]] + 16, {{.*}}(thread uchar*)[[CONTEXT]]);
