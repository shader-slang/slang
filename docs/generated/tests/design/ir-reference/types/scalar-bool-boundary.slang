//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=221e203d328edd373a1da34064baee6a2aab4c19c403c7438de72b93ad6ee5f5
//META: purpose=Boundary: smallest documented scalar (Bool, single bit) appears as a global_param's IR type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// smallest documented scalar (Bool, single bit) appears as a
// global_param's IR type. A `uniform bool` declaration in isolation
// surfaces `Bool` directly on the `global_param` IR instruction, with
// no widening or struct wrap.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool flag;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = flag ? 1 : 0;
}

// IR: let {{.*}}%flag{{.*}}: Bool{{.*}}= global_param
