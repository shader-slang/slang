//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=73a8b7d64d9596c97c899fba163502910f89289de9adea3eac1da3e0599596e6
//META: purpose=A `uniform half` lowers with IR type `Half`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: storage axis — `half` as scalar `global_param`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform half a;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

// CHECK: [nameHint("a")]
// CHECK: let {{.*}}a{{.*}}: Half{{.*}}= global_param
