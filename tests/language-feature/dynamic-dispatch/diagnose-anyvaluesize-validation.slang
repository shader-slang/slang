// Boundary value test for [anyValueSize] limit (0x7FFF = 32767).
// The check uses `>` so 32767 is the last valid value and 32768 is the
// first value that triggers the diagnostic.

// Test 1: [anyValueSize(32768)] — first value over the limit — should fail.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_FAIL): -target spirv -DTEST_OVER_LIMIT

// Test 2: [anyValueSize(32767)] — exactly at the limit — should compile.
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -DTEST_AT_LIMIT -stage compute -entry computeMain

#ifdef TEST_OVER_LIMIT
[anyValueSize(32768)]
/*CHECK_FAIL:
 ^^^^^^^^^^^^ 'anyValueSize' exceeds limit
 ^^^^^^^^^^^^ 'anyValueSize' cannot exceed 32767
*/
interface IFoo
{
    int get();
}
#endif

#ifdef TEST_AT_LIMIT
[anyValueSize(32767)]
interface IFoo
{
    int get();
}

struct Impl : IFoo
{
    int get() { return 0; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}

// CHECK_PASS-NOT: exceeds limit
// CHECK_PASS: computeMain
#endif
