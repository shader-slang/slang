// pass-through-language-mismatch.slang

// `-pass-through` hands the original source file to the selected downstream compiler, and a
// C/C++-only compiler (clang, gcc, visualstudio, llvm, genericcpp) cannot consume Slang or
// any other shader language. The no-`-entry` variants guarded, pre-fix, the empty
// entry-point list reaching `getSingleEntryPointIndex` (the #14603 crash); the `-entry`
// variant reached the downstream compile with source the compiler cannot consume. The
// `wgsl`-suffixed variants guard the language-name lookup that release-asserted on languages
// missing from the name table. (Pass-through skips the Slang front end entirely, so this
// file's body is inert for every variant — which is also why the wgsl variants can re-add
// this same file with `-lang wgsl`.)

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
// test's own file with `-lang wgsl` to get a WGSL translation unit. The clang-based variants
// cover the same path without needing dxc installed, since the gate fires before the
// downstream compiler is loaded.
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL,exit-code=2):-target dxil -pass-through dxc -lang wgsl $dirname/pass-through-language-mismatch.slang
//WGSL: the pass-through compiler 'dxc' cannot compile 'wgsl' source

//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL2,exit-code=2):-target host-callable -pass-through clang -lang wgsl $dirname/pass-through-language-mismatch.slang
//WGSL2: the pass-through compiler 'clang' cannot compile 'slang' source
//WGSL2: the pass-through compiler 'clang' cannot compile 'wgsl' source

//DIAGNOSTIC_TEST:SIMPLE(diag=METAL,exit-code=2):-target host-callable -pass-through clang -lang metal $dirname/pass-through-language-mismatch.slang
//METAL: the pass-through compiler 'clang' cannot compile 'slang' source
//METAL: the pass-through compiler 'clang' cannot compile 'metal' source

//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV,exit-code=2):-target host-callable -pass-through clang -lang spirv $dirname/pass-through-language-mismatch.slang
//SPIRV: the pass-through compiler 'clang' cannot compile 'slang' source
//SPIRV: the pass-through compiler 'clang' cannot compile 'spirv' source

//DIAGNOSTIC_TEST:SIMPLE(diag=LLVMIR,exit-code=2):-target host-callable -pass-through clang -lang llvm $dirname/pass-through-language-mismatch.slang
//LLVMIR: the pass-through compiler 'clang' cannot compile 'slang' source
//LLVMIR: the pass-through compiler 'clang' cannot compile 'llvm' source

// The generic C/C++ pass-through spelling must reject shader source too: the language-table
// entry that lets it accept C and C++ is load-bearing for that rejection.
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC,exit-code=2):-target host-callable -pass-through genericcpp
//GENERIC: the pass-through compiler 'genericcpp' cannot compile 'slang' source

// The gate is deliberately fail-fast over every translation unit, not only the one the entry
// point would consume: a C++ unit is fine for clang, the wgsl unit alongside it is not, and
// the request must be rejected with the mismatch of the offending unit.
//DIAGNOSTIC_TEST:SIMPLE(diag=MULTITU,exit-code=2):-target host-callable -pass-through clang -lang cpp $dirname/pass-through-language-mismatch.slang -lang wgsl $dirname/pass-through-language-mismatch.slang
//MULTITU: the pass-through compiler 'clang' cannot compile 'slang' source
//MULTITU: the pass-through compiler 'clang' cannot compile 'wgsl' source
