//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-02T10:45:10+00:00
//META: source_commit=6815ed82d5e8c95b0e1258ca2eba0ca9c7825d86
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-module-scope-array-constants
//META: doc_section_digest=f896b29e00fc91edb2801172c9cc310b63b1afc7d651c11662f24de1e20b3003
//META: purpose=A module-scope static const array indexed by a runtime value emits as a WGSL const declaration with an inline initializer, with no var<private> fallback
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that emitVarKeywordImpl emits a `static const` array as a
// module-scope WGSL `const`, with no var<private> fallback: a WGSL `const`
// array may be indexed by a runtime value and does not count against the
// private-variable size limit. The kernel indexes the array with a
// thread-id-derived value so the access cannot fold; CHECK pins the
// `const` keyword, the array type, and the inline `= array` initializer
// on the module-scope declaration.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

static const int kLut[4] = { 10, 20, 30, 40 };

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = kLut[tid.x & 3];
}

// CHECK: const kLut_{{[0-9]+}} : array<i32
// CHECK-SAME: = array
