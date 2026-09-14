//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry main -stage raygeneration -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

// Empty callable data is useful when the call itself is the operation and the callable reads only
// global or record state. It still has a semantic type: that type selects the callable ABI even
// though no source field needs transport.
struct EmptyCallableData
{}

// Reuse the callable-data type in a layout-sensitive role. The Metal callable ABI must create its
// own physical sentinel rather than adding storage to EmptyCallableData and shifting `value`.
struct LayoutSensitiveRecord
{
    EmptyCallableData marker;
    int value;
}

ConstantBuffer<LayoutSensitiveRecord> layoutProbe;
RWStructuredBuffer<int> layoutOutput;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = EmptyCallableData;
    typealias Record = void;
}

struct EmptyCallable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::CallableShaderList<EmptyCallable>;
}

rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    EmptyCallableData data = {};
    rt::RayTracer<Schema> tracer;
    tracer.callShader<CallableContext>(0, program, data);
    layoutOutput[0] = layoutProbe.value;
}

// HLSL-DAG: struct EmptyCallableData
// The D3D callable ABI requires one physical parameter. HLSL legalization pads the otherwise
// empty data type so DXC does not see an invalid zero-parameter callable entry point.
// HLSL-DAG: int _slang_dummy
// HLSL: [shader("callable")]void EmptyCallable(inout EmptyCallableData
// DXIL: EmptyCallable

// SPIR-V and Metal still require concrete native storage even though EmptyCallableData contains no
// bits. Their target adapters use role-local scalar backing storage; the source type stays empty.
// SPIRV: OpEntryPoint CallableKHR {{.*}} "EmptyCallable"
// SPIRV: OpExecuteCallableKHR
// METAL: metal::visible_function_table<void(int thread*
// METAL: struct LayoutSensitiveRecord
// METAL-NOT: marker
// METAL: int value{{.*}};
// METAL: {{\[\[visible\]\]}} void
