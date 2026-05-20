//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=9ce4d053661212c26b75e2e5610820fbcc296703c400512972d2afca16087b34
//META: purpose=Requesting an entry-point name that does not exist in the translation unit fires a "no function found matching entry point name 'main'" diagnostic before any IR module can be produced.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: module-level outputs zero-entry-point axis. The doc
// says lowering "records the list of entry-point IR functions
// (each lowered FuncDecl that was registered as an entry point)".
// When `-entry main` is given but no `main` function exists, the
// front-end cannot register any entry point, and the diagnostic
// "no function found matching entry point name 'main'" fires
// (attached to the first source line of the translation unit).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;
/*CHECK:
^^^^^^^ no function found matching entry point name 'main'
*/
void other() { buf[0] = 1; }
