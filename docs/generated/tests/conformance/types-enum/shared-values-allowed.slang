//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-enum.md#description
//META: doc_section_digest=e19ed1fccf556504153f8dba6968221ac410fe7b67d4058007b45ad9c3710e6f
//META: purpose=Multiple enumerators may share the same numeric value per the doc's "Multiple enumerators may share the same numeric value."
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

enum E { A = 5, B = 5, C = 5, D = 7 }

void main()
{
    //CHECK: 5 5 5 7
    printf("%d %d %d %d\n",
           int(E.A), int(E.B), int(E.C), int(E.D));
}
