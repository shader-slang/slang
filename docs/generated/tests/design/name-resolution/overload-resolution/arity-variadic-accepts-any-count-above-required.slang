//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=211bc03cd9a092841d1719f62552fc549d768b7b9dc4ea9b6c978e0acc8daa2b
//META: purpose=An allowed count of -1 makes a variadic callable accept any argument count at or above required, so zero, one and many arguments all pass the arity step.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unbounded arm of the arity step across its whole range: the
// pack parameter imposes no upper bound and nothing is required, so the empty
// call, the single-argument call and a five-argument call must all survive.
// CHECK asserts all three calls resolved.

//TEST:INTERPRET(filecheck=CHECK):

int count<each T>(T args) { return 7; }

void main()
{
    // CHECK: a=7 b=7 c=7
    printf("a=%d b=%d c=%d\n", count(), count(1), count(1, 2, 3, 4, 5));
}
