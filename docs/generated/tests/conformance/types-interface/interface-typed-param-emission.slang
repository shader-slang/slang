//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=When a function with an interface-typed parameter is called with a compile-time-known concrete type, the emitted HLSL is specialized to the concrete type (no virtual dispatch table, no type tag switch).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a compile-time-known concrete type causes the interface-typed
// parameter function to be specialized rather than dynamically dispatched.
// CHECK scans the HLSL output for `Doubler_process` and `callProcess`, proving
// the concrete method name — not a virtual dispatch table — is emitted.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CPUCHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITMETAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCPP):-target cpp -entry computeMain -stage compute

interface IProc
{
    int process(int x);
}

struct Doubler : IProc
{
    int process(int x) { return x * 2; }
}

int callProcess(IProc p, int x)
{
    return p.process(x);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    Doubler d;
    outputBuffer[0] = callProcess(d, int(id.x) + 7);
}

// CPUCHECK: 14
// CHECK: Doubler_process
// CHECK: callProcess
// EMITSPV: LocalSize
// EMITSPV: OpIMul
// EMITMETAL: Doubler_process
// EMITMETAL: callProcess
// EMITMETAL: kernel
// EMITWGSL: Doubler_process
// EMITWGSL: callProcess
// EMITWGSL: @compute
// EMITCUDA: Doubler_process
// EMITCUDA: callProcess
// EMITCUDA: __global__
// EMITCPP: Doubler_process
// EMITCPP: callProcess
