//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#result--optional--conditional-helpers
//META: doc_section_digest=2070b804ffe429adfe07337c383110c2410beb4263e104117d5308624fabdfed
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
