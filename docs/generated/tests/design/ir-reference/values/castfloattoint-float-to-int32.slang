//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=Expansion: `float -> int` truncation lowers to a single `castFloatToInt(%fa)` with `Int` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Isolates the float-to-int direction (the existing smoke test
// bundled it with int-to-float). The doc notes the truncation
// semantics live on this opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(fa);
}

// IR-LABEL: func %main
// IR: castFloatToInt(%fa)
