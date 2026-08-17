// Discriminator for shader-slang/slang#12006's fix.
//
// The #12006 exemption keys off `__extern_cpp` (host-provided, unmangled name) -- NOT off
// export/public linkage in general. This test proves the narrowness: an `export __global` global
// WITHOUT `__extern_cpp` is an ordinary exported symbol that should be initialized, so reading it
// uninitialized still warns E41017. (See uninitialized-global-extern-cpp.slang for the companion
// no-warn case on the host-provided form.)
//
// `non-exhaustive` is used because kernel C++ output runs the uninitialized-globals check once per
// downstream pass (cpu/cuda/llvm), so the same E41017 is emitted several times; we assert the one
// annotated caret, not the per-pass multiplicity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp

interface IThing
{
    int calc(int x);
}

export __global IThing gExported;

export __extern_cpp int useExported(int x)
{
    return gExported.calc(x);
//CHECK:                 ^ E41017
//CHECK:                 ^ use of uninitialized global variable 'gExported'
}
