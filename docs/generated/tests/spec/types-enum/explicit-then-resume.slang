//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-enum.md#description
//META: doc_section_digest=e19ed1fccf556504153f8dba6968221ac410fe7b67d4058007b45ad9c3710e6f
//META: purpose=Explicit-value-then-resume: after `= V`, subsequent unspecified enumerators continue from V+1.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

enum E { A = 10, B, C, D = 100, E2, F }

void main()
{
    //CHECK: 10 11 12 100 101 102
    printf("%d %d %d %d %d %d\n",
           int(E.A), int(E.B), int(E.C),
           int(E.D), int(E.E2), int(E.F));
}
