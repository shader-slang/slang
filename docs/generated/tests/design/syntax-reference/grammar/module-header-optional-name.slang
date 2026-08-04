//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=341ea855b4cc01bfc612ef139eae7725ad7a366b921a54029383d73daf47f18f
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
