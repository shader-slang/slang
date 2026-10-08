//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#static-member-expressions
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=Static member access via `.` and `::` are equivalent per the doc's "These are equivalent: Color.Red and Color::Red."
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red, Green, Blue }

void main()
{
    Color a = Color.Red;
    Color b = Color::Red;
    Color c = Color.Green;
    Color d = Color::Green;
    bool eqRed = (a == b);
    bool eqGreen = (c == d);
    //CHECK: a=0 b=0 c=1 d=1 eqRed=1 eqGreen=1
    printf("a=%d b=%d c=%d d=%d eqRed=%d eqGreen=%d\n",
           int(a), int(b), int(c), int(d),
           eqRed ? 1 : 0, eqGreen ? 1 : 0);
}
