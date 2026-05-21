//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Expansion: vector add on Vec(Half, 3 : Int) operands produces an add whose result type stays Vec(Half, 3 : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Vec` row is parameterised by element type. Adding
// two `half3` globals produces a vector `add` whose result is
// itself a `Vec(Half, 3 : Int)` value — arithmetic on the
// narrow-element vector preserves both element type and width.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half3 a;
uniform half3 b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    half3 c = a + b;
    buf[0] = float(c.x) + float(c.y) + float(c.z);
}

// IR: {{.*}}: Vec(Half, 3 : Int){{.*}}= add(%a, %b)
