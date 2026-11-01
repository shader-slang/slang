//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-module-scope-array-constants
//META: doc_section_digest=c0e1734545a83082eaa2a430271a932e672569e0bbf49f8ad49b06ef278bca3c
//META: purpose=A nested module-scope static const array folds its inner array constituents inline into the outer module-scope const initializer rather than emitting separately named inner declarations
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shared aggregate-constituent fold rule (`getFoldPolicy` and
// `isInnerGlobalAggregate` in slang-emit-c-like.cpp): the inner rows of a 2D
// `static const` array are module-scope aggregates used only as constituents
// of the outer array, so they are inlined into the outer module-scope `const`
// initializer instead of being emitted as separately named declarations. The
// runtime row/column indices keep the array from folding away; CHECK pins the
// `const` declaration with the nested array<array<i32 type and both inner
// `array<i32` constructors inline in its initializer, and the CHECK-NOT lines
// reject any `var<private>` in the output.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

static const int kGrid[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = kGrid[tid.x & 1][tid.x % 3];
}

// CHECK-NOT: var<private>
// CHECK: {{^}}const kGrid_{{[0-9]+}} : array<array<i32
// CHECK-SAME: = array
// CHECK-SAME: ( array<i32
// CHECK-SAME: ), array<i32
// CHECK-NOT: var<private>
