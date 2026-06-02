//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#identifiers
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=Identifiers may start with `_` and contain letters/digits/underscores in subsequent positions, per the doc's identifier grammar.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: identifier "begins with an uppercase or lowercase
// ASCII letter (`A` through `Z`, `a` through `z`), or an underscore
// (`_`). After the first character, ASCII digits (`0` through `9`)
// may also be used in an identifier." Verify a `_`-prefixed name
// with internal digits and underscores compiles and assigns the
// expected value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int _leading = 1;
    int _trailing_underscore_ = 2;
    int with_digits_42_99 = 3;
    int sum = _leading + _trailing_underscore_ + with_digits_42_99;
    //CHECK: sum=6
    printf("sum=%d\n", sum);
}
