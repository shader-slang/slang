//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=a2958c27179f43de9a6c91173654a84b9300b21241a75525e0ed05ed4d9bc705
//META: purpose=Boundary: Optional<T> composes with a Vec<T> payload; the Optional operand is the Vec(...) IR type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Optional(valueType)` has the value type as its
// single operand. A function returning `Optional<float3>`
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
