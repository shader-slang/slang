// pass-through-no-entry-point.c

// Pass-through hands the source of a single entry point's translation unit to the downstream
// compiler. A host-callable target compiles whole programs and accepts a command line without
// `-entry`, so with pass-through there is no translation unit to hand over. This used to
// crash indexing the empty entry-point list; it must diagnose instead.

// clang-format off
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=2):-target host-callable -pass-through clang
//CHECK: pass-through compilation requires an entry point
// clang-format on

int add(int a, int b)
{
    return a + b;
}
