// All source files in one translation unit share one effective language. Content directives are
// allowed to override the request-level fallback only when they agree with one another.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language 2026
#version 450
//CHECK: conflicting source-language directives in one translation unit
//CHECK: this directive selects a different source language
//CHECK: the translation unit's first source-language directive is here
