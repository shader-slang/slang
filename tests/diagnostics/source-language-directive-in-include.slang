// A semantic `__include` is parsed after its translation unit's primary sources. It may confirm
// that source language, but it cannot retroactively select another parser mode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

module "source-language-directive-in-include";

__include "source-language-directive-in-include-helper.slang";

//CHECK: source-language directive conflicts with the effective source language of its translation unit
//CHECK: the translation unit has already been parsed using a different source language
