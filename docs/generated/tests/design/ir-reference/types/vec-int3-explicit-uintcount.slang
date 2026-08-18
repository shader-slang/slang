//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=5a7818957c25646b5df9f0a0213f5663d921d65b0099abf4ef8c860d93e0d5ae
//META: purpose=Boundary: a int3 vector type combines the signed-int scalar with elementCount 3 in the Vec opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a int3 vector type combines the signed-int scalar with elementCount
// 3 in the Vec opcode. Pairing a signed `Int` scalar with elementCount
// 3 surfaces `Vec(Int, 3 : Int)` — distinct from the existing
// `Vec(UInt, 3 : Int)` covered by the smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int3 v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x + v.y + v.z;
}

// IR: let {{.*}}%v{{.*}}: Vec(Int, 3 : Int){{.*}}= global_param
