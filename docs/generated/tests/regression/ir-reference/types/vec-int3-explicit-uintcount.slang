//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Boundary: a int3 vector type combines the signed-int scalar with elementCount 3 in the Vec opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Vec(elementType, elementCount)` is parameterised
// by element type and count. Pairing a signed `Int` scalar with
// elementCount 3 surfaces `Vec(Int, 3 : Int)` — distinct from
// the existing `Vec(UInt, 3 : Int)` covered by the smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int3 v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x + v.y + v.z;
}

// IR: let {{.*}}%v{{.*}}: Vec(Int, 3 : Int){{.*}}= global_param
