//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=d5f0effc9a6866c35191d883a901be38534ac7a715ab357584db096360bfa37e
//META: purpose=Optional<T> and Tuple<T...> are core-module types, usable from user code with no import.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies Optional and Tuple come from the core module rather than
// from an opt-in library, by using both in a file that imports
// nothing. The occupied and empty Optional states are probed through
// hasValue, and a two-element Tuple is destructured through its
// positional members; the CHECK pins all four observations.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> some = 5;
    Optional<int> empty = none;
    Tuple<int, float> pair = makeTuple(3, 2.5f);
    // CHECK: some=1 value=5 empty=0 t0=3 t1=2.5
    printf("some=%d value=%d empty=%d t0=%d t1=%.1f\n",
           int(some.hasValue), some.value, int(empty.hasValue), pair._0, pair._1);
}
