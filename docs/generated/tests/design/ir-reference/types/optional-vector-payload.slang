//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=4be4c15ea47b77fa5c11493f84e33669d7c20c2d179310e3570700407b2ebc9f
//META: purpose=Boundary: Optional<T> composes with a Vec<T> payload; the Optional operand is the Vec(...) IR type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Optional<T> composes with a Vec<T> payload; the Optional operand is
// the Vec(...) IR type. A function returning `Optional<float3>`
// surfaces `Optional(Vec(Float, 3 : Int))` in the Func result-type
// slot.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float3 a;
uniform int g;
uniform RWStructuredBuffer<float> buf;

Optional<float3> trySome(int gate, float3 v)
{
    if (gate > 0) return v;
    return none;
}

[numthreads(1, 1, 1)]
void main()
{
    Optional<float3> o = trySome(g, a);
    buf[0] = o.value.x;
}

// IR-LABEL: func %trySome
// IR-SAME: Optional(Vec(Float, 3 : Int))
