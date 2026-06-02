// unknown-optimization-level.slang
//
// Regression coverage for the prefix-style option case: `-O...` should
// report the canonical `-O` rather than the full token `-Obogus`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-Obogus
//CHECK: unknown value for option '-O'. Valid values are '0, none, 1, default, 2, high, 3, maximal'
