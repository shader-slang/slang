//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#container-level-cap
//META: doc_section_digest=17336538ec29e57920f3b7003dd39a6ad2e2af3018c615ad8a7112f747ce1b92
//META: purpose=The higher-than-parent cap searches for the nearest enclosing aggregate starting at the decl itself, so a nested `public` aggregate inside an `internal` aggregate is compared against itself and accepted.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that makes the cap bite only on non-aggregate
// members. `Inner` is `public` inside an `internal` `Outer`, which is the
// same shape that E30601 rejects for a `public` method; because the search
// for the nearest enclosing `AggTypeDeclBase` starts at `Inner` itself,
// `Inner` is compared against `Inner` and no violation is reported. The
// CHECK asserts the program checked and ran, returning the nested value.

//TEST:INTERPRET(filecheck=CHECK):

internal struct Outer
{
    public struct Inner { int x; }

    Inner make(int v) { Inner i; i.x = v; return i; }
}

void main()
{
    Outer o;
    Outer.Inner i = o.make(9);
    // CHECK: x=9
    printf("x=%d\n", i.x);
}
