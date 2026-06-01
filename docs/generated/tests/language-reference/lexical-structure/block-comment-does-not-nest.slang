//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#comments
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=Block comments do not nest per the explicit doc note "Block comments do not nest." Verify by relying on the OUTER `*/` to close even when an inner `/*` precedes.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "Block comments do not nest." That means the
// first `*/` terminates the surrounding block comment, regardless
// of intervening `/*` tokens. If nesting WERE supported, the
// program below would parse differently (the inner `*/` would
// close the inner comment, and a stray `*/` would remain).
//
// We test by writing `/* /* */ int valid = 1;` — under no-nesting,
// the first `*/` closes the comment, then `int valid = 1;` is a
// real declaration. Under nesting, the `int valid = 1;` would still
// be inside the comment and the parser would see only the trailing
// `*/` as a stray token.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    /* /* */ int valid = 42;
    //CHECK: valid=42
    printf("valid=%d\n", valid);
}
