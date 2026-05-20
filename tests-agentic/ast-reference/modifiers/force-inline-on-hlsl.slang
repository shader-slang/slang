//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels
//META: doc_section_digest=bb776d675adb7c4e2a9412ec7d3dac41d5686e57228edbe14f81c6b4581f05ae
//META: purpose=`[ForceInline]` on a small helper inlines it into the caller; the helper's name no longer appears in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ForceInlineAttribute` forces inlining of the target function. After
// inlining, the helper's body appears at the call site and the helper
// itself is dropped from emitted HLSL.

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

// The doubled-and-incremented expression appears inlined.
//CHECK: main
//CHECK-NOT: helper(
