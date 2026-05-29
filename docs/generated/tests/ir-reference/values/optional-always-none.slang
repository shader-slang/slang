//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#result-optional-conditional-helpers
//META: doc_section_digest=a4ceb599938f068a1086e6ea24a1acb5127549fc407caedc96c472c16ac1c013
//META: purpose=Boundary: `Optional<int> o = none;` (bare `none` literal at the construction site) lowers to `makeOptionalNone` — the empty-side edge of the Optional value space.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The smoke test exercises both Optional branches at once. This
// boundary probe takes the always-none case in isolation, so the
// IR shows exactly one `makeOptionalNone` for the `Optional<T>`
// construction and a `getOptionalValue(%o)` on the (statically
// dead) value-read path. `optionalHasValue` is constant-folded
// away because the operand is statically known to be `none`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Optional<int> o = none;
    if (o.hasValue)
        buf[0] = o.value;
    else
        buf[0] = -1;
}

// IR-LABEL: func %main
// IR: makeOptionalNone
// IR: getOptionalValue(%o)
