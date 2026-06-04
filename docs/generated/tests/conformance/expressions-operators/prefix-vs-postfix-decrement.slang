//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#postfix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Prefix `--x` yields the new value; postfix `x--` yields the old value. Same yield-old-vs-new contract as the increment pair.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Mirror of the increment yield-old-vs-new contract for `--`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 10;
    int preYield = --a;

    int b = 10;
    int postYield = b--;

    //CHECK: preYield=9 a=9 postYield=10 b=9
    printf("preYield=%d a=%d postYield=%d b=%d\n", preYield, a, postYield, b);
}
