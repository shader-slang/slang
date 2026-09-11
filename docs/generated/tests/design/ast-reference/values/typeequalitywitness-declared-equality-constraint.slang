//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#typeequalitywitness
//META: doc_section_digest=c16dad74c952cb93b7b85512859a8d1adcb70dbac3706ab1b925f60f97892662
//META: purpose=A declared equality constraint such as where T == U is its own spelling of equality evidence, and it discharges the subtype obligation in both directions inside the generic body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `where T == U` constraint supplies equality evidence
// usable symmetrically. Neither T nor U is known inside the generic, so
// returning a T where a U is expected — and a U where a T is expected —
// can only check through the declared equality. CHECK asserts each
// direction returns the argument it was handed, so both obligations
// were discharged rather than one direction being coerced.

//TEST:INTERPRET(filecheck=CHECK):

struct Tagged { int v; }

U asU<T, U>(T t, U u) where T == U { return t; }
T asT<T, U>(T t, U u) where T == U { return u; }

void main()
{
    Tagged first;
    first.v = 5;
    Tagged second;
    second.v = 8;
    // CHECK: asU=5 asT=8
    printf("asU=%d asT=%d\n", asU(first, second).v, asT(first, second).v);
}
