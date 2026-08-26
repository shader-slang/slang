//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct PayloadData {}

struct MyTraceContext : rt::ITraceContext
{
    typealias Payload = PayloadData;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct FakeHitGroups : rt::IHitGroupList<MyTraceContext>
//CHECK:                   ^^^^^^^^^^^^^ cannot inherit from type 'rt.IHitGroupList<MyTraceContext>' marked 'sealed' in module 'raytracing'
{
    static const int count = 1;
}

struct FakeMissGroups : rt::IMissGroupList<MyTraceContext>
//CHECK:                    ^^^^^^^^^^^^^^ cannot inherit from type 'rt.IMissGroupList<MyTraceContext>' marked 'sealed' in module 'raytracing'
{
    static const int count = 1;
}

struct FakeCallableGroups : rt::ICallableGroupList<MyTraceContext>
//CHECK:                        ^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.ICallableGroupList<MyTraceContext>' marked 'sealed' in module 'raytracing'
{
    static const int count = 1;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = FakeHitGroups;
    typealias MissGroups = rt::NoMissGroups<TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> descriptor;

[shader("raygeneration")]
void main()
{
    PayloadData payload;
    rt::RayTracer<Layout> tracer;
    tracer.trace({}, scene, descriptor, payload);
}
