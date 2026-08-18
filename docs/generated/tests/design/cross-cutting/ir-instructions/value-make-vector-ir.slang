//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=31b4dd520f90edc4b02037b40da6ae302b38c57ff5b86d114951a278fb8a52e7
//META: purpose=Constructing a vector from scalar components lowers to a `makeVector` aggregate-constructor opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "aggregate constructors" value-instruction entry.
// A `float3(a, b, c)` built from non-constant `uniform` scalars lowers
// to the `makeVector` opcode in the IR dump rather than a folded vector
// constant.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform float b;
uniform float c;
uniform RWStructuredBuffer<float3> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = float3(a, b, c);
}

// IR-LABEL: func %main
// IR: makeVector(%{{.*}}, %{{.*}}, %{{.*}})
