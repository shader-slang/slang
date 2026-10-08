//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=06568c8014909b1a1fe61ae1dd314428312f177b6412e33a4e643d370a1fc6d9
//META: purpose=A generic value parameter may have an enumeration type, and the enumerator bound to it is usable as a compile-time constant in the body.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the enumeration case of the value-parameter type rule (the doc
// admits Boolean, integer, and enumeration value parameters). `scale<let M :
// Mode>` compares M against an enumerator to pick a multiplier, and the call
// binds M explicitly to `Mode.High`. CHECK asserts 40 (= 4 * 10), which is only
// produced when the enum-typed value parameter carried `Mode.High` into the
// body's comparison.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

enum Mode
{
    Low = 1,
    High = 2,
}

int scale<let M : Mode>(int x)
{
    return M == Mode.High ? x * 10 : x;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = scale<Mode.High>(4);
}
// CHECK: 40
