//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=`[noinline]` keeps the helper as its own function in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The contrast case for `[ForceInline]`: `[noinline]` asks that the callee not
// be folded into the caller. CHECK asserts a function named after the helper is
// still present in the emitted HLSL (modulo the mangling suffix), so the two
// attributes are shown to select opposite outcomes for the same shader shape.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[noinline]
int helperNoInline(int x)
{
    return x * 3;
}

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = helperNoInline(int(tid.x));
}

//CHECK: helperNoInline
