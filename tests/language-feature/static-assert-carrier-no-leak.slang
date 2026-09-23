// A passing `static_assert` outside a function body is lowered into a synthesized carrier function;
// `checkStaticAssert` removes the assertion instruction and `slang-emit.cpp` then deletes the
// now-unreferenced carrier, so nothing from the assertion reaches a backend. This compiles
// successfully (every assertion holds); the SPIR-V emit must then contain exactly one function (the
// entry point) — a leaked carrier would add a second — and the HLSL emit must contain the entry
// point and no `export` (a carrier is retained via `[hlslExport]`, which the HLSL emitter spells as
// the `export` keyword). Carrier deletion happens in `linkAndOptimizeIR` before target emission, so
// the single-function SPIR-V check guards the shared logic for every backend.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

static_assert(sizeof(int) == 4, "module scope holds");

namespace N
{
    static_assert(true, "namespace scope holds");
}

struct S
{
    int a;
    static_assert(sizeof(int) == 4, "aggregate scope holds");
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    //HLSL-NOT: export
    //HLSL: void computeMain
    //SPV: OpEntryPoint
    //SPV-COUNT-1: OpFunctionEnd
    //SPV-NOT: OpFunctionEnd
    outputBuffer[0] = 1;
}
