//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#result-optional-conditional-helpers
//META: doc_section_digest=50ea260e6478b155f75a6a10b59f984b9662c43b1424c04c5d8d462fa4d6ec29
//META: purpose=Boundary: a function that unconditionally returns `Optional<int>` from a value (no `none` branch) surfaces `makeOptionalValue(%x)` exactly once — the value-side edge of the Optional value space.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The value-side edge: the function never returns `none`, so the
// IR contains only `makeOptionalValue` (not `makeOptionalNone`).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

Optional<int> always(int x)
{
    return x;
}

[numthreads(1, 1, 1)]
void main()
{
    Optional<int> o = always(a);
    if (o.hasValue)
        buf[0] = o.value;
}

// IR: makeOptionalValue(%x)
// IR-LABEL: func %main
// IR: optionalHasValue(%o)
// IR: getOptionalValue(%o)
