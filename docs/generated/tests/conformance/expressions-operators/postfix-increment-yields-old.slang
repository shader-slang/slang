//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#postfix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Postfix `x++` yields the **old** value per the doc's bullet list "Read → Increment → Write → Yield the old value".
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's bullet list for the built-in postfix `++` operator says
// the expression yields the OLD (pre-increment) value. The variable
// is still updated (incremented), but the expression's value is the
// pre-increment one. This is the canonical distinction from prefix.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 10;
    int yielded = x++;
    //CHECK: yielded=10 x=11
    printf("yielded=%d x=%d\n", yielded, x);
}
