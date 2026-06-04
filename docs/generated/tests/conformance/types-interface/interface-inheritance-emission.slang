//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:30:00+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A struct conforming to a child interface that inherits from a parent interface emits both parent and child method functions on every target; all requirements from both interfaces are compiled into concrete implementations.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that interface inheritance produces concrete method functions for all
// inherited requirements. `Impl` satisfies `IDerived : IBase`, so both `Impl_getA`
// (from IBase) and `Impl_getB` (from IDerived) must appear in emitted code on every target.
// Thread-ID inputs defeat constant folding so SPIRV shows OpIAdd for both fields.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CPUCHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=EMITHLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITMETAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCPP):-target cpp -entry computeMain -stage compute

interface IBase
{
    int getA();
}

interface IDerived : IBase
{
    int getB();
}

struct Impl : IDerived
{
    int _a;
    int _b;
    int getA() { return _a; }
    int getB() { return _b; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    Impl obj;
    obj._a = int(id.x) + 10;
    obj._b = int(id.y) + 20;
    outputBuffer[0] = obj.getA();
    outputBuffer[1] = obj.getB();
}

// CPUCHECK: 10
// CPUCHECK: 20
// EMITHLSL: Impl_getA
// EMITHLSL: Impl_getB
// EMITSPV: LocalSize
// EMITSPV: OpIAdd
// EMITMETAL: Impl_getA
// EMITMETAL: Impl_getB
// EMITMETAL: kernel
// EMITWGSL: Impl_getA
// EMITWGSL: Impl_getB
// EMITWGSL: @compute
// EMITCUDA: Impl_getA
// EMITCUDA: Impl_getB
// EMITCUDA: __global__
// EMITCPP: Impl_getA
// EMITCPP: Impl_getB
