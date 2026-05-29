//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#result-optional-conditional-helpers
//META: doc_section_digest=a4ceb599938f068a1086e6ea24a1acb5127549fc407caedc96c472c16ac1c013
//META: purpose=`Optional<T>` constructors (`return x;` and `return none;`) lower to `makeOptionalValue` / `makeOptionalNone`, and accessors `o.hasValue` / `o.value` lower to `optionalHasValue` / `getOptionalValue`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Result / Optional / Conditional helpers table lists four
// related opcodes: `makeOptionalValue`, `makeOptionalNone`,
// `optionalHasValue`, `getOptionalValue`. A function that returns
// `Optional<int>` (with both success and `none` branches) plus a
// caller that inspects `.hasValue` and `.value` surfaces all four.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform bool c;
uniform RWStructuredBuffer<int> buf;

Optional<int> maybeVal(bool ok, int x)
{
    if (ok) return x;
    return none;
}

[numthreads(1, 1, 1)]
void main()
{
    Optional<int> o = maybeVal(c, a);
    if (o.hasValue)
        buf[0] = o.value;
    else
        buf[0] = -1;
}

// IR: makeOptionalValue(%{{.*}})
// IR: makeOptionalNone
// IR-LABEL: func %main
// IR: optionalHasValue(%o)
// IR: getOptionalValue(%o)
