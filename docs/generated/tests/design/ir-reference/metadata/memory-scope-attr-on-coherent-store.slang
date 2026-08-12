//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=2da76dd4726862c0040f126154c48b23ed2237c18f88f175659ae9d45c2fec5d
//META: purpose=The MemoryScope attribute records the memory scope of a coherent store, sitting after the Aligned attribute in the store's operand tail.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the memory-scope attribute reaches the access it describes.
// `storeCoherent<4, MemoryScope::Device>` requests both an alignment and a
// scope; the CHECKs capture the two attribute definitions and then require the
// emitted `store` to carry both ids, in that order, after its pointer and
// value operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry computeMain -stage compute -capability vk_mem_model

uniform int* inputBuffer;
uniform int* outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Ptr<int> ptrIn = inputBuffer;
    Ptr<int> ptrOut = outputBuffer;
    storeCoherent<4, MemoryScope::Device>(ptrOut, loadCoherent<4, MemoryScope::Device>(ptrIn));
}

// CHECK: %[[MS:[0-9]+]]{{.*}}= MemoryScope({{[0-9]+}} : Int)
// CHECK: %[[AL:[0-9]+]]{{.*}}= Aligned(4 : Int)
// CHECK: store(%{{[a-zA-Z0-9_]+}}, %{{[a-zA-Z0-9_]+}}, %[[AL]], %[[MS]])
