//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#prefix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Prefix `-` is arithmetic negation per the operator table; `-x` equals `0 - x`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

int negate(int x) { return -x; }

void main()
{
    int a = negate(7);
    int b = negate(-3);
    int c = negate(0);
    //CHECK: a=-7 b=3 c=0
    printf("a=%d b=%d c=%d\n", a, b, c);
}
