//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#comments
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=A line comment extends up to but does not include the next line break; the following line is parsed normally.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a line comment ends at the newline, leaving the next
// line's tokens available to the parser. Two variables declared on
// lines with trailing `//` comments are summed; CHECK on `sum=3`
// proves neither declaration was consumed by the comment.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 1; // comment that runs to end of this line
    int b = 2;     // and another
    int sum = a + b;
    //CHECK: sum=3
    printf("sum=%d\n", sum);
}
