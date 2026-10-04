//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `do { }` body followed by neither `while` nor `catch` is rejected with E20004 naming both accepted keywords.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `Parser::ParseDoStatement` parses the body first and only
// then decides what kind of statement it was: `while` makes it a do-while
// loop, `catch` makes it a do-catch. Anything else falls to the `Unexpected`
// call, whose expectation string is written as the single literal
// "while' or 'catch" so that the quoting in the rendered message comes out as
// `expected 'while' or 'catch'`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    do { } notwhile;
//CHECK:   ^^^^^^^^ unexpected token
//CHECK:   ^^^^^^^^ unexpected identifier, expected 'while' or 'catch'
}
