//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -o -

// A function carrying both `[ForceInline]` and `[noinline]` that survives inlining — here because
// it is exported, so its definition must still be emitted — must not receive both the `alwaysinline`
// and `noinline` LLVM attributes, which LLVM's verifier rejects as incompatible. `[ForceInline]`
// wins, so the emitted function's attribute set carries `alwaysinline` and not `noinline` — the same
// precedence the CUDA emitter applies to `__forceinline__`/`__noinline__` (shader-slang/slang#12623).
// `CHECK-NOT` is placed after the `attributes` match so filecheck scans from there to end of
// output, keeping the earlier mangled symbol line (which can embed the substring `noinline`) out
// of its range.
// CHECK: attributes #0 = { alwaysinline
// CHECK-NOT: noinline

[DllExport]
[ForceInline]
[noinline]
int combined(int x)
{
    return x + 1;
}
