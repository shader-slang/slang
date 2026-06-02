//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#escaped-line-breaks
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=An escaped line break (`\` immediately before a newline) is eliminated during phase 2 of compilation, joining the two physical lines into one logical line.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The classic exercise: a preprocessor macro whose body spans
// multiple physical lines via `\<newline>`. The expansion must
// behave as if the body were one line.

//TEST:INTERPRET(filecheck=CHECK):

#define SUM_THREE(a, b, c) \
    ((a) + \
     (b) + \
     (c))

void main()
{
    int v = SUM_THREE(1, 2, 4);
    //CHECK: v=7
    printf("v=%d\n", v);
}
