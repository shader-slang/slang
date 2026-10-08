//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#prefix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Prefix `+` is identity per the operator table — `+x` yields the same value as `x`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

int passthrough(int x) { return +x; }

void main()
{
    int a = passthrough(42);
    int b = passthrough(-17);
    int c = passthrough(0);
    //CHECK: a=42 b=-17 c=0
    printf("a=%d b=%d c=%d\n", a, b, c);
}
