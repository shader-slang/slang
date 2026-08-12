//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=badf6daf5560f5144e7061f1568abcf04bfa7ece00a5e1441996814b2d18964f
//META: purpose=`ModuleHeader` accepts `'module' ';'` with the name omitted, falling back to the current module name.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a module header may omit the module name entirely:
// `module;` starts the file and the rest of the program still parses
// and runs, which is only possible if the nameless header is accepted.

//TEST:INTERPRET(filecheck=CHECK):

module;

void main()
{
    // CHECK: ok=1
    printf("ok=%d\n", 1);
}
