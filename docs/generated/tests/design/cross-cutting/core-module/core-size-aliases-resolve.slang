//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=d5f0effc9a6866c35191d883a901be38534ac7a715ab357584db096360bfa37e
//META: purpose=The pointer-width aliases size_t, usize_t and ssize_t are in scope without any import and hold integral values.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the size-related aliases from the core module's opening
// alias block are visible to user code with no import statement, and
// that ssize_t is the signed member of the trio. The values are
// round-tripped through int() and printed; the CHECK pins the signed
// alias holding a negative value, which an unsigned alias could not.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    size_t a = size_t(7);
    usize_t b = usize_t(8);
    ssize_t c = ssize_t(-9);
    // CHECK: size=7 usize=8 ssize=-9
    printf("size=%d usize=%d ssize=%d\n", int(a), int(b), int(c));
}
