//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Boundary: smallest documented scalar (Bool, single bit) appears as a global_param's IR type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Basic scalar types" table lists `Bool` as the smallest
// documented scalar (a single-bit truth value). A `uniform bool`
// declaration in isolation surfaces `Bool` directly on the
// `global_param` IR instruction, with no widening or struct wrap.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool flag;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = flag ? 1 : 0;
}

// IR: let {{.*}}%flag{{.*}}: Bool{{.*}}= global_param
