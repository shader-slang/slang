//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=`[ForceInline]` on a small helper inlines it into the caller; the helper's name no longer appears in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `[ForceInline]` causes Slang itself to inline the callee, so no
// call survives to the back end. CHECK asserts the entry point is emitted and
// CHECK-NOT asserts no call to `helper(` remains -- its body having been spliced
// into the call site.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[ForceInline]
int helper(int x)
{
    return x * 2 + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = helper(int(tid.x));
}

//CHECK: void main
//CHECK-NOT: helper(
