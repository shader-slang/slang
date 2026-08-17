//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=b3e088ac3dc9706ad8749cb8f9a4f8033e0bd510147eb7d3d1d0a3563b2dd6ca
//META: purpose=The signed-integer scalar opcodes Int8, Int16, Int, Int64 appear as struct-field types in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The signed-integer scalar opcodes Int8, Int16, Int, Int64 appear as
// struct-field types in the IR dump. A struct whose fields cover each
// of these scalar widths surfaces all four opcodes in one IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Ints { int8_t a; int16_t b; int c; int64_t d; }

uniform Ints s;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(s.a) + int(s.b) + int(s.c) + int(s.d);
}

// IR-LABEL: struct %Ints
// IR: field(%{{.*}}, Int8)
// IR: field(%{{.*}}, Int16)
// IR: field(%{{.*}}, Int)
// IR: field(%{{.*}}, Int64)
