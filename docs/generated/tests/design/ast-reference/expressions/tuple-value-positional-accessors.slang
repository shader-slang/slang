//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A tuple value exposes its members through the positional `._0` / `._1` accessors.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a tuple value built by `makeTuple` exposes its components
// through the positional `._i` accessors. Two components of different types
// are stored and read back separately, so a mis-ordered or mistyped component
// fails a CHECK.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Tuple<int, float> t = makeTuple(7, 1.5f);
    // CHECK: a=7
    printf("a=%d\n", t._0);
    // CHECK: b=1
    printf("b=%d\n", int(t._1));
}
