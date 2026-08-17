//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
//META: purpose=Negative: the descriptor-heap `__subscript` repeats its `glsl_hlsl_spirv_wgsl` availability gate, so a target outside that set is diagnosed at the indexing site rather than silently emitting.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other half of the descriptor-heap gate exercised by
// `builtin-resource-descriptor-heap-multitarget.slang`: Metal is not
// in the admitted target set, so the same shader must be rejected
// instead of emitted. CHECK pins the capability error code E36107, the
// rejected target name, and the note that points back at the heap
// indexing expression — the location the gate is documented to
// diagnose.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    RWStructuredBuffer<uint> heapBuf = ResourceDescriptorHeap[0];
    heapBuf[0] = 5;
    outputBuffer[0] = 1;
}

// CHECK-DAG: E36107
// CHECK-DAG: 'metal' compilation target
// CHECK-DAG: ResourceDescriptorHeap[0]
