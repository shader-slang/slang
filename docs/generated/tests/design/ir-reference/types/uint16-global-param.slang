//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: a scalar UInt16 surfaces as the IR type of a uniform uint16_t global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Basic scalar types table lists `UInt16` as the 16-bit
// unsigned integer. A bare `uniform uint16_t` exercises the
// scalar storage form for the narrow unsigned type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint16_t v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(v);
}

// IR: let {{.*}}%v{{.*}}: UInt16{{.*}}= global_param
