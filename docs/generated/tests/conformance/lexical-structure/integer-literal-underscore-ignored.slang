//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#integer-literals
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=Underscores embedded in an integer-literal digit sequence are ignored per the doc: "The digits of an integer literal may also include underscore (`_`) characters, which are ignored and have no semantic impact."
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 1_000_000;
    int b = 1000000;
    int c = 0x_FF_FF;
    bool eq = (a == b) && (c == 0xFFFF);
    //CHECK: a=1000000 b=1000000 c=65535 eq=1
    printf("a=%d b=%d c=%d eq=%d\n", a, b, c, eq ? 1 : 0);
}
