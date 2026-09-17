//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=1d384c53a44075a9ff7b3de0f1907c30fb579e521154ac25ad145d433be8dd6a
//META: purpose=CompareLookupResultItems prefers a declaration in the type itself over an equally-applicable member contributed by an extension.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "non-extension decl over an extension member" arm of the
// lookup-result comparison. Both `act` declarations accept the call with
// identical cost, so the tie is broken purely by where each was declared.
// CHECK asserts the struct-body member's tag.

//TEST:INTERPRET(filecheck=CHECK):

struct S { int act(int x) { return 1; } }

extension S { int act(int x) { return 2; } }

void main()
{
    S s;
    int tag = s.act(1);
    // CHECK: tag=1
    printf("tag=%d\n", tag);
}
