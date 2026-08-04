//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#result-optional-conditional-helpers
//META: doc_section_digest=50ea260e6478b155f75a6a10b59f984b9662c43b1424c04c5d8d462fa4d6ec29
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
