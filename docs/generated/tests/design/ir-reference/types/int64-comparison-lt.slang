//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: less-than compare on two Int64 operands yields a cmpLT with Int64 inputs and Bool result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc separates `Int64` from the platform-default `Int`.
// Comparing two `int64_t` globals produces a `cmpLT` with the
// wider operand type preserved at the IR level.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int64_t a;
uniform int64_t b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a < b) ? 1 : 0;
}

// IR: {{.*}}: Bool{{.*}}= cmpLT(%a, %b)
