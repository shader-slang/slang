// option-missing-argument.slang

// Missing argument for an option:

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target -profile ps_4_0

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-profile ps_4_0 -target
