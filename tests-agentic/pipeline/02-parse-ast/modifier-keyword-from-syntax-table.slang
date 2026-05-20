//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#syntax-as-declaration
//META: doc_section_digest=d55e14c87bae9e097965548fcc3531ed08f6cc4e8b973587ecd544f7084b255d
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
