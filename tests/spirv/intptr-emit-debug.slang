// Regression test for https://github.com/shader-slang/slang/issues/13200, debug-info variant.
//
// With debug info enabled, a pointer-sized local crashes SPIR-V emit even when
// SCCP folds the variable to a constant and no pointer-sized value survives to
// run time: the debug info still describes the variable's type, which keeps
// the `IntPtr`/`UIntPtr` type inst alive (the debug-type size query decorates
// it with `SizeAndAlignment`) so it reaches `emitGlobalInst`. Without `-g` the
// folded-away type never reaches emit, so this shader compiles even on a
// compiler that lacks the `OpTypeInt` case for pointer-sized types.

//TEST:SIMPLE(filecheck=CHECK_G_S): -target spirv-asm -entry computeMainS -stage compute -emit-spirv-directly -g
//TEST:SIMPLE(filecheck=CHECK_G_U): -target spirv-asm -entry computeMainU -stage compute -emit-spirv-directly -g

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMainS(uniform RWStructuredBuffer<int> outp)
{
    intptr_t x = intptr_t(3);
    outp[0] = int(x);
}

// The pointer-sized local's debug type must resolve to a 64-bit signed integer
// type declaration plus a signed `DebugTypeBasic` named after the source type.
// The result-id anchor matters because `-g` embeds the whole source file in the
// module as an OpString — these CHECK comment lines included — so an unanchored
// `OpTypeInt 64 1` would match the embedded copy of this very comment. The
// `OpString "intptr_t"` pattern needs no anchor: spirv-dis escapes the quotes in
// the embedded text, so only the real instruction matches.
// CHECK_G_S-DAG: %{{[a-zA-Z_0-9]+}} = OpTypeInt 64 1
// CHECK_G_S-DAG: OpString "intptr_t"
// CHECK_G_S-DAG: OpExtInst %void %{{[0-9]+}} DebugTypeBasic %{{[0-9]+}} %uint_64 %uint_4 %uint_{{[0-9]+}}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMainU(uniform RWStructuredBuffer<int> outp)
{
    uintptr_t x = uintptr_t(3);
    outp[0] = int(x);
}

// CHECK_G_U-DAG: %{{[a-zA-Z_0-9]+}} = OpTypeInt 64 0
// CHECK_G_U-DAG: OpString "uintptr_t"
// CHECK_G_U-DAG: OpExtInst %void %{{[0-9]+}} DebugTypeBasic %{{[0-9]+}} %uint_64 %uint_6 %uint_{{[0-9]+}}
