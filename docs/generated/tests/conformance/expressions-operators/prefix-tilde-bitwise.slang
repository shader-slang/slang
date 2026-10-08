//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#prefix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Prefix `~` is bitwise Boolean negation per the operator table; `~0u` is `UINT_MAX` (all bits set).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

uint invert(uint x) { return ~x; }

void main()
{
    uint a = invert(0u);
    uint b = invert(0xFFFFFFFFu);
    //CHECK: a=4294967295 b=0
    printf("a=%u b=%u\n", a, b);
}
