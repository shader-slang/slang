// Companion to func-type-unsupported-on-kernel-target.slang for the targets that compile
// through kernel C++ rather than emitting it directly.
//
// `shader-sharedlib` and `host-callable` map to `CPPSource` in `_getDefaultSourceForTarget`, so
// the unsupported function type reaches the same emitter -- but the check receives the final
// target, not the intermediate one. Without them in the target set the undefined name was left
// for the downstream toolchain to complain about: gcc reported `'Slang_FuncType' does not name
// a type`, and the LLVM path `no template named 'Slang_FuncType'`, neither naming the shader
// line responsible. `llvm-shader-ir` shares that lowering and crashed outright rather than
// reporting anything, so the diagnostic replaces a segmentation fault there.
//
// Left in exhaustive mode deliberately: `//CHECK-NOT:` is inert in a `diag=` test, so
// exhaustive matching is the only thing asserting that one mistake yields one error. Adding
// `non-exhaustive` to quiet a future cascading diagnostic would silently drop that.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target shader-sharedlib -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target host-callable -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target llvm-shader-ir -entry computeMain -stage compute

int addOne(int x)
{
    return x + 1;
}

static functype(int) -> int gFn = addOne;
//CHECK:                    ^^^ function-typed value is not supported on this target
//CHECK:                    ^^^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime

int applyIt(functype(int) -> int f, int x)
{
    return f(x);
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = applyIt(gFn, 41);
}
