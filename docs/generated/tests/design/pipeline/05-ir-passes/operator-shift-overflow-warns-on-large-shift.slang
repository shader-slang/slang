//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=The operator-shift-overflow pass diagnoses a literal shift count that exceeds the operand bit width.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Operator shift overflow" row of the SSA / cleanup category
// (`slang-ir-operator-shift-overflow.cpp`) diagnoses shift counts
// that exceed the operand's bit width. A `1 << 40` shift on a 32-bit
// integer produces a warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;
uniform int a;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = a;
    x = x << 40;
//CHECK:  ^^ left shift overflow
// CHECK: W41030
    buf[0] = x;
}
