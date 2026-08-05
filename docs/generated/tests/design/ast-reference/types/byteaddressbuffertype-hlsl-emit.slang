//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=The untyped byte-address buffer types are addressed by byte offset through Load/Store and keep the ByteAddressBuffer and RWByteAddressBuffer spellings through HLSL emission.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the untyped buffer family, which unlike the structured
// family carries no element type: bytes are reached by explicit offset.
// A read-only buffer is loaded and a read-write one stored, and CHECK
// matches both spellings in the emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ByteAddressBuffer g_in;
RWByteAddressBuffer g_out;

//CHECK-DAG: {{^}}ByteAddressBuffer
//CHECK-DAG: RWByteAddressBuffer
[numthreads(1, 1, 1)]
void main()
{
    uint v = g_in.Load(0);
    g_out.Store(0, v + 1);
}
