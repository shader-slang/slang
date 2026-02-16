//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-E

// Undefining a macro that is not defined within C/C++ is defined as *not* an error or a warning.
// On checking with DXC/FXC they also have this behavior (ie they don't output anything)
// It's arguable if Slang should match this behavior - at least it is a warning.

#undef C
//diag:^ undefined macro
//diag:^ macro 'C' is not defined
