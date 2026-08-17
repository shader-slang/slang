//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=d5f0effc9a6866c35191d883a901be38534ac7a715ab357584db096360bfa37e
//META: purpose=Default initialization of a core-module Optional<T> produces the same empty state as the none literal.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that `none` is also what default initialization
// of an `Optional<T>` produces. Three optionals are compared: one left
// default-initialized, one explicitly assigned `none`, and one holding a
// value. CHECK asserts the first two report hasValue == 0 while the third
// reports 1, so the default-initialized one is in the empty state rather
// than in an undefined or value-holding one. Both an `int` and a `float`
// instantiation are probed, because `Optional` is generic and a per-element
// default could differ.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> defaulted;
    Optional<int> explicitNone = none;
    Optional<int> occupied = 5;
    Optional<float> defaultedFloat;
    // CHECK: defaulted=0 explicitNone=0 occupied=1 defaultedFloat=0
    printf("defaulted=%d explicitNone=%d occupied=%d defaultedFloat=%d\n",
           int(defaulted.hasValue),
           int(explicitNone.hasValue),
           int(occupied.hasValue),
           int(defaultedFloat.hasValue));
}
