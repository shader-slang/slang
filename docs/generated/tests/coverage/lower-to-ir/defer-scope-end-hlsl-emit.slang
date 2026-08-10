//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a block-scoped `defer` lowers so its body executes at the close of that block, before code following the block.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the emitted-HLSL shape of `visitDeferStmt`. The defer writes
// outputBuffer[1] = x; the enclosing block mutates x to tid.x+10 before
// closing. The lowering moves the deferred store to the scope-end, so in
// the emitted HLSL the store to index 1 (deferred) appears after x has been
// updated and BEFORE the trailing store to index 0. CHECK asserts the
// index-1 store precedes the index-0 store, both reading the updated x.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
RWStructuredBuffer<int> outputBuffer;
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    {
        defer { outputBuffer[1] = x; }
        x = x + 10;
    }
    outputBuffer[0] = x;
}
// CHECK: int x_{{[0-9]+}} = int(tid_{{[0-9]+}}.x) + int(10);
// CHECK: outputBuffer_{{[0-9]+}}[int(1)] = x_{{[0-9]+}};
// CHECK: outputBuffer_{{[0-9]+}}[int(0)] = x_{{[0-9]+}};
