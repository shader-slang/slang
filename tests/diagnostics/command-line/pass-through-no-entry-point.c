// pass-through-no-entry-point.c

// Pass-through hands the source of the translation unit of a single entry point to the
// downstream compiler. A host-callable target compiles whole programs and accepts a command
// line without `-entry`, so with pass-through there is no translation unit to hand over — and
// with more than one entry point there is no single source to hand over either. Both used to
// reach `getSingleEntryPointIndex`, which asserts exactly one; they must diagnose instead.

// clang-format keeps `// CHECK:` comments spaced in C files, and the diagnostic-annotation
// parser only recognizes an unspaced `//CHECK:` prefix, so keep the directive formatting off.
// clang-format off
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=2):-target host-callable -pass-through clang
//CHECK: pass-through compilation requires exactly one entry point

//DIAGNOSTIC_TEST:SIMPLE(diag=TWO,exit-code=2):-target host-callable -pass-through clang -entry add -entry sub
//TWO: pass-through compilation requires exactly one entry point
// clang-format on

int add(int a, int b)
{
    return a + b;
}
