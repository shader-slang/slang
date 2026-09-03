//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry ClosestHit -stage closesthit -target metal

// Metal keeps a selected structural stage as a logical entry point until its
// backend adapter is synthesized. Its invoke method is still an executable root.

import slang.raytracing;

interface IShape
{
    uint area();
}

struct Circle : IShape
{
    uint area() { return 3; }
}

struct Square : IShape
{
    uint area() { return 5; }
}

IShape makeShape(uint id)
{
    if (id == 0)
        return Circle();
    return Square();
}

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = ::Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

RWStructuredBuffer<uint> output;

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        IShape shape = makeShape(output[0]);
        output[0] = shape.area();
    }
}

// METAL: uint s_dispatch_IShape_area_0(uint
// METAL: switch(
// METAL: void ClosestHit_invoke_0(
// METAL: s_dispatch_IShape_area_0(
