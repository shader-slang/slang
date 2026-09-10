//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `for` initializer that parses as something other than a declaration or an expression statement is reported with E20001, "unexpected statement, expected expression".
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `Parser::ParseForStatement` does not restrict the
// initializer grammar up front -- it calls the general statement parser and
// then checks afterwards whether what came back was a `DeclStmt` or an
// `ExpressionStmt`. A block statement is neither, so the check fails and the
// parser reports the literal words "statement" and "expression" as the actual
// and expected tokens.
//
// Note the parser does not stop there: it carries on looking for the `;`
// separators it never got, which is where the two trailing syntax errors come
// from. They are pinned too so the test stays exhaustive.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    for ({ int q = 1; } ; ; ) { break; }
//CHECK:   ^^^ unexpected token
//CHECK:   ^^^ unexpected statement, expected expression
//CHECK:                  ^ syntax error
//CHECK:                  ^ syntax error.
//CHECK:                    ^ syntax error
//CHECK:                    ^ syntax error.
}
