//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Test: 'expand' term that captures no type packs produces error 30105.

void badExpandNoPacks()
{
    int x = 1;
    let y = expand x;
//CHECK:    ^^^^^^ 'expand' term captures no type packs
}
