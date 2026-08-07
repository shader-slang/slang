//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=2da76dd4726862c0040f126154c48b23ed2237c18f88f175659ae9d45c2fec5d
//META: purpose=The Aligned attribute records the access alignment of a load or a store, appearing in the operand tail of those instructions rather than in a type layout.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `Aligned` describes an access, not a type. `loadAligned<8>`
// and `storeAligned<16>` request different alignments for the same pointee
// type, so two distinct attribute instructions must exist; the CHECKs capture
// each and then require the store to carry the 16 one and the load to carry
// the 8 one in its trailing operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry computeMain -stage compute

uniform float4* data;

[numthreads(1, 1, 1)]
void computeMain()
{
    var v = loadAligned<8>(data);
    storeAligned<16>(data + 1, v);
}

// CHECK: %[[A16:[0-9]+]]{{.*}}= Aligned(16 : Int)
// CHECK: %[[A8:[0-9]+]]{{.*}}= Aligned(8 : Int)
// CHECK: store(%{{[a-zA-Z0-9_]+}}, %{{[a-zA-Z0-9_]+}}, %[[A16]])
// CHECK: = load(%{{[a-zA-Z0-9_]+}}, %[[A8]])
