//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=A bool-tagged enum is accepted even though no real subtype witness for the integer constraint exists, because conformance synthesis installs a compiler-trusted NoneWitness.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary case the tag-witness claim calls out explicitly:
// `bool` is not an integer type, so the usual subtype witness cannot exist and
// the enum is only accepted if the synthesized marker witness is honored by
// the early-out. CHECK asserts both cases round-trip, proving the declaration
// checked and its cases are usable values.

//TEST:INTERPRET(filecheck=CHECK):

enum Flag : bool
{
    Off = false,
    On = true,
}

void main()
{
    Flag a = Flag.On;
    Flag b = Flag.Off;
    // CHECK: a=1 b=0
    printf("a=%d b=%d\n", (a == Flag.On) ? 1 : 0, (b == Flag.On) ? 1 : 0);
}
