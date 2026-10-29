// A Slang source directive takes precedence over an explicitly requested GLSL language. The
// empty tuple below requires Slang 2026, proving that both the parser mode and module-wide Slang
// version were installed from the directive before parsing began.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/source-language-directive-overrides-explicit-glsl.vert -no-codegen

#language slang 2026
//CHECK: source-language directive selects slang and overrides the explicitly requested glsl source language
//CHECK: the source directive selects slang instead of glsl

void main()
{
    let emptyTuple = ();
}
