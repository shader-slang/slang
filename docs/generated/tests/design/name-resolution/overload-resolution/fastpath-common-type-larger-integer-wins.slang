//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=1c3af19385b1b45e53fc2911de4a5495ab14067ddf0d2925bb4569db9c7887aa
//META: purpose=Among integer operands the larger size wins, so an int + int64_t expression has type int64_t.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "larger size wins" rule of the fast path's common-type
// computation at the 32-bit/64-bit boundary. An overload probe on `int` versus
// `int64_t` observes the type of `a + b`; CHECK asserts the int64_t overload
// fired.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int x)     { return 1; }
int probe(int64_t x) { return 2; }

void main()
{
    int a = 3;
    int64_t b = 4;
    int tag = probe(a + b);
    // CHECK: tag=2
    printf("tag=%d\n", tag);
}
