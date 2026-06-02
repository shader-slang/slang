//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#parenthesized-expression
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=A parenthesized expression evaluates to the same value as the wrapped expression — `(e)` == `e`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

int compute(int x, int y) { return x + y; }

void main()
{
    int bare = compute(3, 4);
    int wrapped = (compute(3, 4));
    int doubled = ((compute(3, 4)));
    //CHECK: bare=7 wrapped=7 doubled=7
    printf("bare=%d wrapped=%d doubled=%d\n", bare, wrapped, doubled);
}
