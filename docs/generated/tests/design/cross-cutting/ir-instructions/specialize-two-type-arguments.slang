//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=96937cffa7eab5a700d8fe30b96ad6032aaa2693bb26bdc6db27157e8ac98d94
//META: purpose=Boundary probe of the variadic `args...` operand list on the documented `specialize` opcode: a 2-type-parameter generic call lowers to `specialize(%generic, Int, Float)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `specialize` with operand shape `(base, args...)`,
// where `args` is variadic. The bootstrap test exercises a one-type
// instantiation; this probe exercises a two-type instantiation —
// confirming the variadic-arity axis on `specialize`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float b;
uniform RWStructuredBuffer<int> buf;

T pick<T,U>(T x, U y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pick<int, float>(a, b);
}

// IR-LABEL: func %main
// IR: specialize(%{{.*}}, Int, Float)
