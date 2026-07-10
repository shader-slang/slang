//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=d1b896b6a6adec252014d2d12ba87c258c20d4783523aeec02b3767c1a892991
//META: purpose=`isDeclVisibleFromScope` treats an extension on the same type as inside the aggregate; a private member is reachable from that extension.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The predicate `isDeclVisibleFromScope` resolves the requesting
// scope's enclosing aggregate, including extensions whose target
// type matches. A private struct member is therefore reachable from
// a method defined in `extension S { ... }`.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    private int secret;
    __init(int x) { secret = x; }
}

extension S
{
    int viaExtension() { return secret + 1; }
}

void main()
{
    S s = S(10);
    // CHECK: viaExtension=11
    printf("viaExtension=%d\n", s.viaExtension());
}
