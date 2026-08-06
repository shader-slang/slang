// `[DllImport]` lowers to a native function pointer: `generateDllImportFuncs` creates a
// function-typed global, which host C++ supports via `Slang_FuncType` in its prelude. The
// #12367 diagnostic must not fire on it.
//
// The check that catches an unsupported function-typed value cannot distinguish this
// synthesized pointer by type, so it keys on the value having a source location -- a
// synthesized one has none. This test pins that: compilation must succeed AND the function
// pointer must actually appear in the output, so a change that stopped emitting it (or started
// rejecting it) fails here rather than passing vacuously.

//TEST:SIMPLE(filecheck=CHECK): -target host-cpp -entry main -stage compute

//CHECK: Slang_FuncType
//CHECK-NOT: error

[DllImport("", "someExternalFunc")]
int someExternalFunc(int x);

export __extern_cpp int main()
{
    return someExternalFunc(41);
}
