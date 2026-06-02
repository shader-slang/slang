//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=Pointer arithmetic on a local array emits `OpPtrAccessChain` in SPIR-V when the offset is a runtime value.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states integer add/subtract on a pointer-to-array-element
// yields an offset pointer.  Verify the SPIR-V backend emits
// OpPtrAccessChain for this pattern when the offset is a runtime value.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

[numthreads(1,1,1)]
void main(RWStructuredBuffer<int> inputBuf, RWStructuredBuffer<int> outputBuf)
{
    int arr[4];
    arr[0] = inputBuf[0];
    arr[1] = inputBuf[1];
    arr[2] = inputBuf[2];
    arr[3] = inputBuf[3];
    int* p = &arr[0];
    p = p + inputBuf[4];  // runtime offset — forces OpPtrAccessChain
    outputBuf[0] = *p;
}
// CHECK: OpPtrAccessChain
