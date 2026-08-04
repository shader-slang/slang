//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=180e0aa25246196c89b6dc01c680b751b33a813ef1597e4ac18dd9fef40c0943
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
