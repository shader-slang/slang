//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry ClosestHit -stage closesthit -target metal

// Metal emits a separately selected structural stage as a logical source helper because a
// physical function-table signature requires a linked schema. Its invoke method is nevertheless
// an executable root: dynamic dispatch and the address spaces of its resource accesses must both
// be legalized.

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
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

RWStructuredBuffer<uint> output;

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        IShape shape = makeShape(output[0]);
        output[0] = shape.area();
    }
}

// METAL-DAG: uint Circle_area_0()
// METAL-DAG: uint Square_area_0()
// METAL: struct KernelContext_0
// METAL: uint device* output_0;
// METAL: void ClosestHit_invoke_0(
// METAL: switch(
// METAL: _S{{[0-9]+}} = Circle_area_0();
// METAL: _S{{[0-9]+}} = Square_area_0();
