//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=An HLSLByteAddressBufferType / HLSLRWByteAddressBufferType is accepted at a uniform and bytes are addressable by Load/Store; the HLSL emit renders the ByteAddressBuffer spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the untyped byte-address buffer family: a ByteAddressBuffer
// is read via Load and an RWByteAddressBuffer written via Store, and
// the HLSL backend renders both spellings. CHECK matches the two
// buffer type spellings in the emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ByteAddressBuffer g_in;
RWByteAddressBuffer g_out;

//CHECK-DAG: ByteAddressBuffer
//CHECK-DAG: RWByteAddressBuffer
[numthreads(1, 1, 1)]
void main()
{
    uint v = g_in.Load(0);
    g_out.Store(0, v + 1);
}
