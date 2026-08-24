//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=d256dde64890becaf8cd7de87d2e7c15cce6794507c618550a67824d086322d4
//META: purpose=`SubpassInput<T>.SubpassLoad()` reads a fragment-shader input attachment as `SubpassLoad(%input)`, and the `SubpassInputMS` overload adds the optional operand that selects an MSAA sample, giving `SubpassLoad(%inputMS, 1 : Int)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `SubpassLoad` catalog row, including the `sample?` marker in
// its operand column: the operand is optional, so the same opcode has to
// appear with one operand and with two.
//
// Both forms are read in one fragment entry point and summed into the
// return value so neither is dead before the dump, and the label directive
// anchoring at `func %main` keeps the matches in the user body. The
// two-operand form pins the literal sample index, which is the only thing
// that distinguishes it from the nullary form in the dump text.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage fragment -entry main

SubpassInput<float4> input;
SubpassInputMS<float4> inputMS;

float4 main() : SV_Target
{
    return input.SubpassLoad() + inputMS.SubpassLoad(1);
}

// IR-LABEL: func %main
// IR-DAG: SubpassLoad(%input)
// IR-DAG: SubpassLoad(%inputMS, 1 : Int)
