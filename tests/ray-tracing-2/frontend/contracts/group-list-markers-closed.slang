//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct PayloadData
{
    uint value;
}

struct MyTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct FakeHitGroups : rt::IHitGroupList
//CHECK:                   ^^^^^^^^^^^^^ cannot inherit from type 'rt.IHitGroupList' marked 'sealed' in module 'raytracing'
{
    static const int count = 1;
}

struct FakeMissShaders : rt::IMissShaderList
//CHECK:                     ^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IMissShaderList' marked 'sealed' in module 'raytracing'
{
    static const int count = 1;
}

struct FakeCallableShaders : rt::ICallableShaderList
//CHECK:                         ^^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.ICallableShaderList' marked 'sealed' in module 'raytracing'
{
    static const int count = 1;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = FakeHitGroups;
    typealias MissShaders = FakeMissShaders;
    typealias CallableShaders = FakeCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> descriptor;

[shader("raygeneration")]
void main()
{
    PayloadData payload;
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, descriptor, payload);
}
