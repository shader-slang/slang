//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void borrowedPack<each T>(__constref expand each T values) {}
//CHECK: '__constref' parameter packs are not supported
