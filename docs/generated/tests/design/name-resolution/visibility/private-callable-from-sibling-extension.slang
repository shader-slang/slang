//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=d1b896b6a6adec252014d2d12ba87c258c20d4783523aeec02b3767c1a892991
//META: purpose=A private member declared in one `extension S` is reachable from another `extension S` because both extensions resolve to the same target type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `isDeclVisibleFromScope` walks the requesting scope's parents and
// matches against the candidate's container type. A private member
// in one extension can be called from a sibling extension on the
// same type.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int base;
    __init(int b) { base = b; }
}

extension S
{
    private int hidden() { return base * 3; }
}

extension S
{
    int viaSibling() { return hidden() + 1; }
}

void main()
{
    S s = S(5);
    // CHECK: viaSibling=16
    printf("viaSibling=%d\n", s.viaSibling());
}
