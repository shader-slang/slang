//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=18390b63de1003330aaf1c938cc3b9a195ccaebbb584bbb2edb3db5df8f75c02
//META: purpose=Expansion: RWStructuredBuffer<int16_t> names Int16 as its element-type operand.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWStructuredBuffer<int16_t> names Int16 as its element-type operand.
// A `RWStructuredBuffer<int16_t>` exposes the narrow signed scalar as
// the buffer's element-type operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<int16_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int16_t(-1);
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(Int16{{.*}}= global_param
