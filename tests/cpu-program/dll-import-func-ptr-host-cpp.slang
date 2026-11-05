// `[DllImport]` lowers to a native function pointer: `generateDllImportFuncs` creates a
// function-typed global, which host C++ supports via `Slang_FuncType` in its prelude.
//
// Host C++ is outside the target set that rejects function-typed values (#12367), so this pins
// that support from the other side: compilation must succeed and the function pointer must
// actually reach the output. Widening that target set to cover host output, or dropping the
// pointer from the emitted code, both fail here rather than passing unnoticed.
//
// The check matches a *declaration* of the pointer rather than the bare type name: host output
// includes the host prelude verbatim, and that prelude defines `Slang_FuncType` itself, so the
// name alone appears even for a shader that uses no function pointer at all.
// The DLL wrapper caches the resolved pointer in namespace storage shared across host calls.
// Its null initializer must stay on the declaration; resetting the pointer in `main` would discard
// that cache on every invocation.

//TEST:SIMPLE(filecheck=CHECK): -target host-cpp -entry main -stage compute

//CHECK: Slang_FuncType<{{.*}}> {{ *}}[[IMPORT_PTR:[A-Za-z_][A-Za-z0-9_]*]] = nullptr;
//CHECK: int32_t {{_main_[0-9]+}}()
//CHECK: {
//CHECK-NOT: [[IMPORT_PTR]] = nullptr;
//CHECK: return

[DllImport("", "someExternalFunc")]
int someExternalFunc(int x);

export __extern_cpp int main()
{
    return someExternalFunc(41);
}
