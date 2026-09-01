//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#type-queries-and-predicates
//META: doc_section_digest=dd5dde08daf5ef4afa93de9a5f62e011969dd2709cf191720b25851af591f755
//META: purpose=Each scalar type predicate (`IsInt`, `IsBool`, `IsFloat`, `IsHalf`, `IsCoopFloat`, `IsUnsignedInt`, `IsSignedInt`) folds to a boolean constant matching the queried scalar type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the answer of every scalar predicate row in one place, including
// the pairs that must disagree: `int` is a signed integer but not an unsigned
// one, `uint` the reverse, and `half` counts as both a float and a packed
// cooperative float. Each predicate contributes a distinct bit to a mask that
// escapes through the output buffer, so the single asserted value 255 pins
// every predicate at once — one wrong answer changes the number.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int mask = 0;
    if (__isInt<int>()) mask |= 1;
    if (__isBool<bool>()) mask |= 2;
    if (__isFloat<float>()) mask |= 4;
    if (__isHalf<half>()) mask |= 8;
    if (__isCoopFloat<half>()) mask |= 16;
    if (__isUnsignedInt<uint>()) mask |= 32;
    if (__isSignedInt<int>()) mask |= 64;
    if (!__isSignedInt<uint>() && !__isUnsignedInt<int>() && !__isInt<float>()) mask |= 128;
    outputBuffer[0] = mask;
}

// CHECK: 255
