//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic
//META: doc_section_digest=5d4c2952398382bdc9ae3befb591dfdbf6629429f5af8c13daa49fd77c0de113
//META: purpose=A user-written semantic lowers to a userSemantic attribute whose operand 0 is the name string and operand 1 the semantic index.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shared name/index operand shape on the user-semantic side. Two
// parameters carry semantics with explicit trailing indices; the CHECK-DAGs
// show the compiler splits each spelling into a bare name string plus the
// index as a separate integer operand, which is what lets a backend reason
// about the index without parsing the string.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage fragment

struct VOut
{
    float4 pos : SV_Position;
    float3 c   : COLOR3;
};

float4 main(VOut input, float2 uv : TEXCOORD2) : SV_Target
{
    return float4(input.c + float3(uv, 0), 1);
}

// CHECK-DAG: = userSemantic("TEXCOORD", 2 : Int)
// CHECK-DAG: = userSemantic("COLOR", 3 : Int)
