//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry main -stage raygeneration -target dxil -profile sm_6_7
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

RWStructuredBuffer<uint> output;

struct TestCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input)
    {
        input.data.value += input.record.value;
    }
}

struct TestCallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = TestCallableContext;
    typealias Callable = TestCallable;
}

struct TestProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups<TestTraceContext>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::CallableGroupList<TestTraceContext, TestCallableGroup>;
}

rt::TraceProgramDescriptor<TestProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestProgramLayout> tracer;
    TestCallableData data = {1};
    tracer.callShader<TestCallableContext>(0, program, data);
    output[0] = data.value;
}

// IR: structuralRayTracingCallShader({{.*}}, %TestProgramLayout, %TestTraceContext,
// IR-SAME: %rtx5FCallableGroupList, TypePack(%TestCallableGroup), %TestCallableContext,
// IR-SAME: %TestCallableData,

// HLSL-DAG: CallShader(0U,
// HLSL-DAG: [shader("callable")]void TestCallable(inout TestCallableData_0
// HLSL-DAG: data_{{[0-9]+}}.value_{{[0-9]+}} = data_{{[0-9]+}}.value_{{[0-9]+}} + record_{{[0-9]+}}.value_{{[0-9]+}};

// DXIL: TestCallable

// SPIRV: OpEntryPoint RayGenerationKHR %main "main"
// SPIRV: OpEntryPoint CallableKHR %TestCallable "TestCallable"
// SPIRV: OpExecuteCallableKHR %uint_0

// METAL: metal::visible_function_table<void(TestCallableData_0 thread*, uint constant*, uint device*)> callableFunctions_0;
// METAL: ((rt_TraceProgramDescriptorResources_default_0 constant*)(program_resources_{{[0-9]+}}))->callableFunctions_0[0U](&data_{{[0-9]+}}, (constant uint*)(program_resources_{{[0-9]+}}), program_resources_{{[0-9]+}}->records_0);
// METAL: {{\[\[visible\]\]}} void TestCallable_0(TestCallableData_0 thread* [[DATA:data_[0-9]+]], uint constant* descriptorResources_{{[0-9]+}}, uint device* [[RECORDS:descriptorData_[0-9]+]])
// METAL: [[DATA]]->value_{{[0-9]+}} = [[DATA]]->value_{{[0-9]+}} +
// METAL-SAME: (*(TestCallableRecord_0 device*)
// METAL-SAME: [[RECORDS]]
