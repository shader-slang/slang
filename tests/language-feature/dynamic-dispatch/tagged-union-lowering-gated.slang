// Regression for https://github.com/shader-slang/slang/issues/11917.
//
// `lowerTaggedUnionTypes` is gated on `RequiredLoweringPassSet::taggedUnion`. This test
// locks in that the gate FIRES when the module contains tagged-union IR: dispatching a
// call through an interface-typed value whose dynamic type is one of several concrete
// impls forces the typeflow specialization pass to represent the value as a tagged union
// (a tag selecting the witness table, plus a payload). The pass then lowers those
// tagged-union insts to a tuple of a tag and the payload. If the gate predicate were too
// narrow (a false-negative), the un-lowered tagged-union insts would survive to a later
// pass or the emitter and compilation would fail, so the CHECK lines below would not
// match.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

[anyValueSize(16)]
interface IShape
{
    int area();
}

struct Square : IShape
{
    int side;
    int area() { return side * side; }
}

struct Rect : IShape
{
    int w;
    int h;
    int area() { return w * h; }
}

// Passing the value through an `IShape`-typed local defeats static specialization at the
// call site, so the compiler must build a dynamic (tagged-union) representation that can
// hold either `Square` or `Rect`.
int areaOf(IShape s)
{
    return s.area();
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    IShape s;
    if (outBuf[0] > 0)
    {
        Square sq = { 3 };
        s = sq;
    }
    else
    {
        Rect r = { 2, 4 };
        s = r;
    }
    outBuf[i] = areaOf(s);
}

// The two concrete impls must both survive to the output; a working tagged-union lowering
// keeps both areas reachable through the dynamic dispatch. (These names are the mangled
// impl methods and are stable identifiers in the emitted code.)
// CHECK-DAG: Square
// CHECK-DAG: Rect

// SPIR-V is likewise a non-HLSL textual target that runs the same lowering; the module
// must produce valid SPIR-V with an entry point. If the gate wrongly skipped the pass,
// the un-lowered tagged-union insts would reach the SPIR-V emitter and compilation would
// fail, so this positive anchor would not match.
// SPV: OpEntryPoint
