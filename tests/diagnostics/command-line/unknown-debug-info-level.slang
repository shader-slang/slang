// unknown-debug-info-level.slang
//
// Coverage for the multi-category `_getValue` path: `-g...` accepts
// values from two `ValueCategory`s (DebugLevel + DebugInfoFormat) and
// joins their names for the diagnostic. Pin the canonical option name
// here as well.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-gbogus
//CHECK: unknown value for option '-g'. Valid values are '0, none, 1, minimal, 2, standard, 3, maximal, default-format, c7, pdb, stabs, coff, dwarf'
