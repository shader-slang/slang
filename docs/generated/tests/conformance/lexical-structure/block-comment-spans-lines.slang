//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#comments
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=Block comments may contain line breaks and arbitrary text without disturbing surrounding code.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a block comment may span multiple lines, including
// arbitrary tokens, without disturbing the surrounding expression.
// `int a = /* ... multi-line ... */ 7;` must compile and assign 7;
// CHECK on `a=7` proves the block was treated as whitespace.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = /*
              this comment
              spans multiple lines
              and may contain {weird} tokens; like ##(((
            */ 7;
    //CHECK: a=7
    printf("a=%d\n", a);
}
