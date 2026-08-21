//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=A leading `optional` on a subtype constraint makes conformance optional; the body must test it before using the interface's members.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `where optional T : I` admits both conforming and
// non-conforming type arguments. The body guards the interface call with an
// `is` test and returns a sentinel otherwise; the CHECK asserts the
// conformer's value for one call and the sentinel for the other, which
// requires the constraint to be optional rather than mandatory.

//TEST:INTERPRET(filecheck=CHECK):

interface IThing { int thing(); }

struct Yes : IThing { int thing() { return 5; } }
struct No { int x; }

int probe<T>(T t) where optional T : IThing
{
    if (T is IThing) { return t.thing(); }
    return -1;
}

void main()
{
    Yes y;
    No n;
    //CHECK: 5 -1
    printf("%d %d\n", probe(y), probe(n));
}
