//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#syntax-as-declaration
//META: doc_section_digest=96201e4a01337de99d9651244ec405bb74f7aa94360c94d6d62426600a323482
//META: purpose=Modifier keywords like `nointerpolation` are not hard-coded in the parser but bound to syntax via the SyntaxParseInfo table (populated by populateBaseLanguageModule and the core module); the parser dispatches to the right callback by identifier lookup.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `nointerpolation` is one of the modifier keywords registered through
// the syntax-table mechanism rather than being lexed as a reserved
// word. If the dispatch mechanism worked, this declaration parses and
// `pick` is callable; if it did not, the parser would either reject
// the identifier or attach it as something other than a modifier.

//TEST:INTERPRET(filecheck=CHECK):

float pick(nointerpolation float a, float b) { return a + b; }

void main()
{
    // CHECK: r=7
    printf("r=%d\n", int(pick(3.0, 4.0)));
}
