//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the `struct Name = Type;` alias form requires a terminating semicolon and reports E20001 naming the token found in its place.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `Parser::ParseStruct` accepts an `=` after the struct name
// and, when it sees one, parses a type expression and returns the struct as an
// alias instead of parsing a body. Because it returns immediately it never
// consumes the `;` itself, so it first checks that a `;` is the next token and
// diagnoses if not -- reporting the token type it actually saw against the
// literal expectation "';'". It then returns anyway, which is why the
// following declaration is still parsed and the only diagnostic is this one.
//
// Here the alias is written without its `;`, so the `void` that opens the next
// declaration is what gets named.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct MyAlias = int

[numthreads(1, 1, 1)]
void main() {}
/*CHECK:
^^^^ unexpected token
^^^^ unexpected identifier, expected ';'
*/
