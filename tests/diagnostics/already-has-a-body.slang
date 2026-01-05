//DIAGNOSTIC_TEST:SIMPLE(): 

// Use a regular test here rather than a DIAGNOSTIC_TEST because the test
// runner can't yet parse the new diagnostic style
//TEST:SIMPLE(): -enable-experimental-rich-diagnostics

int foo(){ /* original */ }
int foo(){ /* redefinition */ }
