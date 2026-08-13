//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=Expansion: RWStructuredBuffer<uint16_t> names UInt16 as its element-type operand.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWStructuredBuffer<uint16_t> names UInt16 as its element-type
// operand. A `RWStructuredBuffer<uint16_t>` exposes the narrow
// unsigned scalar as the buffer's element-type operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint16_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint16_t(0xABCD);
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(UInt16{{.*}}= global_param
