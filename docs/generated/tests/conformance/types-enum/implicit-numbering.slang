//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-enum.md#description
//META: doc_section_digest=e19ed1fccf556504153f8dba6968221ac410fe7b67d4058007b45ad9c3710e6f
//META: purpose=Implicit numbering: first enumerator is 0; each subsequent unspecified is previous+1.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red, Green, Blue, Alpha }

void main()
{
    //CHECK: 0 1 2 3
    printf("%d %d %d %d\n",
           int(Color.Red), int(Color.Green),
           int(Color.Blue), int(Color.Alpha));
}
