//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#prefix-operator-expressions
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=Prefix `!` is Boolean negation per the operator table; `!true` is `false` and `!false` is `true`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two probe overloads to distinguish bool values from int values
// (and to dodge the slangi VM bool-arg bug we filed as #11402:
// we never pass the bool LITERAL as a bool argument — we pass an
// `int` ?: expression instead).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool x = true;
    bool y = false;
    bool nx = !x;
    bool ny = !y;
    int nxInt = nx ? 1 : 0;
    int nyInt = ny ? 1 : 0;
    //CHECK: nxInt=0 nyInt=1
    printf("nxInt=%d nyInt=%d\n", nxInt, nyInt);
}
