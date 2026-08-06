// Regression test for the #12367 diagnostic: `[DllImport]` lowers to a native function pointer
// (`generateDllImportFuncs` creates a function-typed global), which is legitimate and must not
// be reported as an unsupported function-typed value.
//
// The `String` errors below are pre-existing and unrelated -- the DllImport lowering builds
// `String`-typed insts, which kernel C++ does not support either (E55213). What this test pins
// is the absence of E55215: the function pointer itself must not be diagnosed.

//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute

//CHECK-NOT: E55215

[DllImport("", "someExternalFunc")]
int someExternalFunc(int x);

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = someExternalFunc(41);
}
