//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#prefix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Prefix `++x` yields the **new** value per the doc's explicit bullet list "Read → Increment → Write → Yield the new value".
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's bullet list for the built-in prefix `++` operator says
// the expression yields the NEW (post-increment) value. Compared to
// the postfix sibling (which yields the OLD value), this is the
// distinguishing contract. Verify both the yielded value and the
// updated variable.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 10;
    int yielded = ++x;
    //CHECK: yielded=11 x=11
    printf("yielded=%d x=%d\n", yielded, x);
}
