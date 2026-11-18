// pass-through-no-entry-point.c

// Pass-through hands one translation unit's source to the downstream compiler: the unit of the
// (single) selected entry point. A host-callable target compiles whole programs and accepts a
// command line without `-entry`, so with pass-through there is no translation unit to hand
// over — and with more than one entry point there is no single source to hand over either.
// Every variant below must produce E52017: the CHECK and TWO variants on the binary
// host-callable path, the CPP variants on the source-target path, and the IDS variant pins
// the E52017 id.

// clang-format keeps `// CHECK:` comments spaced in C files, and the diagnostic-annotation
// parser only recognizes an unspaced `//CHECK:` prefix, so keep the directive formatting off.
// clang-format off
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=2):-target host-callable -pass-through clang
//CHECK: pass-through compilation hands over the source of exactly one entry point

//DIAGNOSTIC_TEST:SIMPLE(diag=TWO,exit-code=2):-target host-callable -pass-through clang -entry add -entry sub
//TWO: pass-through compilation hands over the source of exactly one entry point

//DIAGNOSTIC_TEST:SIMPLE(diag=CPPSRC,exit-code=2):-target cpp -pass-through clang -o out.cpp
//CPPSRC: pass-through compilation hands over the source of exactly one entry point

//DIAGNOSTIC_TEST:SIMPLE(diag=CPPTWO,exit-code=2):-target cpp -pass-through clang -entry add -entry sub -o out.cpp
//CPPTWO: pass-through compilation hands over the source of exactly one entry point

//DIAGNOSTIC_TEST:SIMPLE(diag=IDS,exit-code=2):-target host-callable -pass-through clang
//IDS: E52017
// clang-format on

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}
