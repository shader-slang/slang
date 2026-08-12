// downstream-compiler-version.slang
//
// Exercise the `-<compiler>-version` query option (issue #11552 / PR #11556). Each invocation
// prints the version of the downstream compiler that Slang would actually load for that
// pass-through (via IGlobalSession::getDownstreamCompilerVersion), then lets compilation of this
// comment-only file proceed and exit cleanly. The printed line has a stable "<compiler> version:"
// prefix whether or not the toolchain is installed (it prints "not found" when the compiler
// cannot be located), so these checks are deterministic across machines and need no GPU.

//TEST:SIMPLE(filecheck=DXC): -dxc-version
//TEST:SIMPLE(filecheck=FXC): -fxc-version
//TEST:SIMPLE(filecheck=GLSLANG): -glslang-version
//TEST:SIMPLE(filecheck=NVRTC): -nvrtc-version
// spirv-dis exercises the hyphenated-name path: `-spirv-dis-version` must recover "spirv-dis".
//TEST:SIMPLE(filecheck=SPIRVDIS): -spirv-dis-version
// Two query flags in one invocation: proves the option consumes no value (it must not swallow the
// following `-glslang-version`), and that versions print in command-line order.
//TEST:SIMPLE(filecheck=MULTI): -dxc-version -glslang-version

// DXC: dxc version:
// FXC: fxc version:
// GLSLANG: glslang version:
// NVRTC: nvrtc version:
// SPIRVDIS: spirv-dis version:
// MULTI: dxc version:
// MULTI: glslang version:
