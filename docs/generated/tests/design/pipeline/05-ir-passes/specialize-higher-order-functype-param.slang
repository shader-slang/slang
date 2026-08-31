//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=2f10610b6a8497f9b2cec62d411162a4585b32e9698a95e5288845967ffba16d
//META: purpose=A call passing a global function to a `functype` parameter is rewritten into a call to a specialized variant that references the global directly, so no function-typed value survives to emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `specializeHigherOrderParameters` on the surface form that
// produces an `IRFuncType` parameter: the `functype` type constructor.
// `apply` is declared with two parameters, one of them function-typed, and
// is called once with the global `twice`. The CHECKs assert the emitted
// `apply` has lost the function-typed parameter (it takes a single `int`)
// and calls `twice` by name from inside its body — which is only true if
// the pass produced a specialized variant that closed over the global
// rather than passing a function value at runtime.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

RWStructuredBuffer<int> outBuf;

int apply(functype (int) -> int f, int x)
{
    return f(x);
}

int twice(int x)
{
    return 2 * x;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = apply(twice, int(tid.x));
}

// The specialized `apply` keeps only the value parameter ...
// HLSL: int apply_{{[0-9]+}}(int
// ... and names the global callee directly inside its body.
// HLSL: twice_{{[0-9]+}}(

// GLSL: int apply_{{[0-9]+}}(int
// GLSL: twice_{{[0-9]+}}(
