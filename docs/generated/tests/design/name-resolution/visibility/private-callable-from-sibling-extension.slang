//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=c54ab9dab59a428876c20360e9e45d6480dca1c6fd84d84d999b4927fa66945f
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
