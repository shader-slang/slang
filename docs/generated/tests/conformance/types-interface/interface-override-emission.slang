//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:30:00+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=Inheriting types may override a default method using `override`; emitted code contains the default-impl function for the non-overriding struct and the concrete override function for the overriding struct on every target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `override` modifier causes target code to emit two distinct
// functions: the default impl for DefaultUser and a concrete override for OverrideUser.
// Non-constant `id.x` input defeats constant folding so SPIRV shows both OpIAdd (default
// path: v+100) and OpIMul (override path: v*3).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CPUCHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=EMITHLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITMETAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCPP):-target cpp -entry computeMain -stage compute

interface IComputable
{
    int base(int v);
    int compute(int v) { return base(v) + 100; }
}

struct DefaultUser : IComputable
{
    int base(int v) { return v; }
}

struct OverrideUser : IComputable
{
    int base(int v) { return v; }
    override int compute(int v) { return v * 3; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    DefaultUser du;
    OverrideUser ou;
    outputBuffer[0] = du.compute(int(id.x) + 1);
    outputBuffer[1] = ou.compute(int(id.x) + 1);
}

// CPUCHECK: 101
// CPUCHECK: 3
// EMITHLSL: defaultImpl
// EMITHLSL: OverrideUser_compute
// EMITSPV: LocalSize
// EMITSPV: OpIAdd
// EMITSPV: OpIMul
// EMITMETAL: defaultImpl
// EMITMETAL: OverrideUser_compute
// EMITMETAL: kernel
// EMITWGSL: defaultImpl
// EMITWGSL: OverrideUser_compute
// EMITWGSL: @compute
// EMITCUDA: defaultImpl
// EMITCUDA: OverrideUser_compute
// EMITCUDA: __global__
// EMITCPP: defaultImpl
// EMITCPP: OverrideUser_compute
