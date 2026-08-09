//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=0b34b0fb1813131149d3616a45cc3577cb15e3fd3da49c3edccfff26407d2c0a
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
// CHECK: E38000
void other() { buf[0] = 1; }
