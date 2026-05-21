//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Expansion: narrowing conversion from a Vec(Double, 4) to a Vec(Float, 4) yields a floatCast whose result is the narrower-element vector.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Vec` row pairs an element type with an element count.
// Casting a `double4` to a `float4` produces a `floatCast` whose
// result type is `Vec(Float, 4 : Int)` — same element count, new
// element type — confirming that elementwise conversion is encoded
// at the vector level.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double4 v;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float4(v);
}

// IR: {{.*}}: Vec(Float, 4 : Int){{.*}}= floatCast(%v)
