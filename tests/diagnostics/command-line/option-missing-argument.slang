// option-missing-argument.slang

// Missing argument for an option:

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1,non-exhaustive):-target -profile ps_4_0
//CHECK1: unknown code generation target '-profile'

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2,non-exhaustive):-profile ps_4_0 -target
//CHECK2: expected an argument for command-line option '-target'
