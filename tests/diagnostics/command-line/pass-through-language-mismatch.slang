// pass-through-language-mismatch.slang

// A C/C++-only pass-through compiler (clang, gcc, visualstudio, llvm) cannot consume Slang
// source, because `-pass-through` hands the original source file to the downstream compiler.
// Every spelling below must diagnose that mismatch instead of crashing on the empty
// entry-point list that whole-program targets (host-callable, sharedlib) compile with.

export __extern_cpp int add(int a, int b) { return a + b; }

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=2):-target host-callable -pass-through clang
//CHECK: the pass-through compiler 'clang' cannot compile 'slang' source

//DIAGNOSTIC_TEST:SIMPLE(diag=ENTRY,exit-code=2):-target host-callable -pass-through clang -entry add
//ENTRY: the pass-through compiler 'clang' cannot compile 'slang' source

//DIAGNOSTIC_TEST:SIMPLE(diag=OUT,exit-code=2):-target host-callable -pass-through clang -o out.dll
//OUT: the pass-through compiler 'clang' cannot compile 'slang' source

//DIAGNOSTIC_TEST:SIMPLE(diag=SHARED,exit-code=2):-target sharedlib -pass-through gcc
//SHARED: the pass-through compiler 'gcc' cannot compile 'slang' source

//DIAGNOSTIC_TEST:SIMPLE(diag=LLVM,exit-code=2):-target host-callable -pass-through llvm
//LLVM: the pass-through compiler 'llvm' cannot compile 'slang' source

//DIAGNOSTIC_TEST:SIMPLE(diag=VS,exit-code=2):-target host-callable -pass-through visualstudio
//VS: the pass-through compiler 'visualstudio' cannot compile 'slang' source

// The gate must name every source language the enum has, not only the shader ones: a WGSL
// translation unit rejected by dxc used to crash asserting in the language-name lookup. A
// `.wgsl` file cannot be tracked here (the repo ignores `.wgsl` under tests/ as generated
// output), and a SIMPLE test always lists its input file first, so this variant re-adds the
// test's own file with `-lang wgsl` to get a WGSL translation unit.
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL,exit-code=2):-target dxil -pass-through dxc -lang wgsl $dirname/pass-through-language-mismatch.slang
//WGSL: the pass-through compiler 'dxc' cannot compile 'wgsl' source
