//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:19Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-module-scope-array-constants
//META: doc_section_digest=fbbf1497db1e4a587fdacf892f78df4d81a0f88d121cbeab859a34f4a0623146
//META: purpose=A module-scope static const array indexed by a runtime value emits as var<private> with an inline initializer instead of a WGSL const declaration
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies emitVarKeywordImpl's module-scope-array rule: a WGSL `const`
// array is only indexable by const-expressions, so the emitter must
// declare a `static const` array as an addressable var<private> to keep
// runtime indexing legal. The kernel indexes the array with a
// thread-id-derived value so the access cannot fold; CHECK pins the
// var<private> keyword, the array type, and the inline `= array`
// initializer on the module-scope declaration.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

static const int kLut[4] = { 10, 20, 30, 40 };

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = kLut[tid.x & 3];
}

// CHECK: var<private> kLut_{{[0-9]+}} : array<i32
// CHECK-SAME: = array
