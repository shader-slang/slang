//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=221e203d328edd373a1da34064baee6a2aab4c19c403c7438de72b93ad6ee5f5
//META: purpose=The unsigned-integer scalar opcodes UInt8, UInt16, UInt, UInt64 appear as struct-field types in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The unsigned-integer scalar opcodes UInt8, UInt16, UInt, UInt64
// appear as struct-field types in the IR dump. A struct whose fields
// cover each unsigned-integer width surfaces all four opcodes in the
// IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct UInts { uint8_t a; uint16_t b; uint c; uint64_t d; }

uniform UInts s;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(s.a) + uint(s.b) + s.c + uint(s.d);
}

// IR-LABEL: struct %UInts
// IR: field(%{{.*}}, UInt8)
// IR: field(%{{.*}}, UInt16)
// IR: field(%{{.*}}, UInt)
// IR: field(%{{.*}}, UInt64)
