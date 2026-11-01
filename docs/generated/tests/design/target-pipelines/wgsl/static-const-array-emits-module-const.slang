//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-module-scope-array-constants
//META: doc_section_digest=c0e1734545a83082eaa2a430271a932e672569e0bbf49f8ad49b06ef278bca3c
//META: purpose=A module-scope static const array indexed by a runtime value emits as a module-scope WGSL const with an inline initializer, not as a var<private>
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies emitVarKeywordImpl's rule for module-scope constants: a
// `static const` array is emitted as a WGSL `const` like any other
// module-scope constant, because a WGSL `const` array may be indexed by a
// runtime value and a `const` does not count against WGSL's 8192-byte limit
// on the combined size of the private-address-space variables a shader
// accesses. The kernel indexes the array with a thread-id-derived value so
// the access cannot fold; CHECK pins the `const` keyword at the start of the
// module-scope declaration, the array type, and the inline `= array`
// initializer, and the CHECK-NOT lines before and after it reject any
// `var<private>` in the output.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

static const int kLut[4] = { 10, 20, 30, 40 };

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = kLut[tid.x & 3];
}

// CHECK-NOT: var<private>
// CHECK: {{^}}const kLut_{{[0-9]+}} : array<i32
// CHECK-SAME: = array
// CHECK-NOT: var<private>
