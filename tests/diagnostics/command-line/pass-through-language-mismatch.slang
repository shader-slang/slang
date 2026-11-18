// pass-through-language-mismatch.slang

// `-pass-through` hands the original source file to the selected downstream compiler, and a
// C/C++-only compiler (clang, gcc, visualstudio, llvm, genericcpp) cannot consume Slang or any
// other shader language. Every variant below must produce E52016 naming the language the user
// selected: the plain, `-entry`, `-o`, sharedlib, llvm, visualstudio and genericcpp spellings
// pass Slang source to a compiler that records only C/C++; the `wgsl`-suffixed variants pass a
// translation unit in the named language via `-lang`; the multi-TU variant mixes a C++ unit the
// compiler accepts with a wgsl unit it does not, pinning the fail-fast over all translation
// units; the tint variant proves a compiler with no recorded languages is skipped (E51700
// never appears; the request instead fails the entry-point check, which is only reached when
// the language gate let it through); the id variant pins the E52016 id itself. (Pass-through
// skips the Slang front end entirely, so this file's body is inert for every variant — which is
// also why the language variants can re-add this same file with `-lang <name>`.)

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
// output), and a SIMPLE test always lists its input file first, so these variants re-add the
// test's own file with `-lang wgsl` to get a WGSL translation unit. For dxc the prepended
// Slang unit maps to HLSL, which dxc accepts, so only the wgsl unit is diagnosed here; the
// clang-based variants cover the same path without needing dxc installed, and for them the
// prepended Slang unit is rejected too, since the gate is fail-fast over every translation
// unit.
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
// the request must be rejected with the mismatch of the offending unit (the harness's
// prepended Slang unit is rejected as well).
//DIAGNOSTIC_TEST:SIMPLE(diag=MULTITU,exit-code=2):-target host-callable -pass-through clang -lang cpp $dirname/pass-through-language-mismatch.slang -lang wgsl $dirname/pass-through-language-mismatch.slang
//MULTITU: the pass-through compiler 'clang' cannot compile 'slang' source
//MULTITU: the pass-through compiler 'clang' cannot compile 'wgsl' source

// Tint has no recorded languages, so the gate skips it and the request proceeds to the
// entry-point check — E52017 here proves the language gate let a zero-flag compiler through
// (a wrongly-firing E52016 fails the test first). The check runs before any downstream
// compiler is loaded, so no toolchain is needed.
//DIAGNOSTIC_TEST:SIMPLE(diag=SKIP,exit-code=2):-target host-callable -pass-through tint
//SKIP: pass-through compilation hands over the source of exactly one entry point

// Pin the E52016 id itself.
//DIAGNOSTIC_TEST:SIMPLE(diag=IDS,exit-code=2):-target host-callable -pass-through clang
//IDS: E52016
