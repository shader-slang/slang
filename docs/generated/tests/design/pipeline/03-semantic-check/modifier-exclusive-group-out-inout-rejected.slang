//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=233c54717a586cdba7e515d0d6fbb4c7e6363f272d4b60a4aed8b9997a298ad9
//META: purpose=Two direction modifiers from the shared `out`/`inout`/`ref`/`borrow` group on one parameter are rejected as a conflicting combination (E31202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the genuinely-exclusive (as opposed to merely repeated) half of the
// conflict-group rule: `out` and `inout` are distinct modifiers that map to the
// same conflict group, so the second one lands in a group the first already
// claimed. CHECK pins E31202 and the message naming both modifiers, which is
// what distinguishes a group collision from a plain duplicate.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f(out inout int x)
//CHECK: E31202
//CHECK: modifier 'inout' is redundant or conflicting with existing modifier 'out'
{
    x = 1;
}

void main()
{
    int y = 0;
    f(y);
}
