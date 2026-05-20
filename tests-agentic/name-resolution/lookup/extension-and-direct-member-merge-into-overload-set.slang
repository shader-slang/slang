//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=Member lookup walks the inheritance facets including extensions; direct members and same-name extension members of the same type accumulate into one overload set.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The inheritance walk includes extension facets ("a special case
// that keeps extensions targeting the same type in scope" — doc).
// `s.twice(...)` must see both the direct member `twice(int)` and
// the extension member `twice(float)` as candidates of one overload
// set.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int twice(int v) { return v * 2; }
}

extension S
{
    int twice(float v) { return int(v) + 100; }
}

void main()
{
    S s;
    int r = s.twice(3) + s.twice(2.0f);  // 6 + 102
    // CHECK: r=108
    printf("r=%d\n", r);
}
