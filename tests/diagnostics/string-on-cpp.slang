// Regression test for #11297: using `String` on a target that emits
// standalone C++/CUDA (i.e. one that does not include `slang.h` and so
// has no `Slang::String` in scope) used to silently emit invalid output
// like `int64_t _S3 = "123".getLength();`. The lexer/parser doesn't know
// the target, but `checkUnsupportedInst` runs once per emit target with
// the IR module — diagnose `kIROp_StringType` use there for cpp / cuda
// targets, with a hint pointing at `NativeString` (which works on cpp
// via `__intrinsic_asm "int(strlen($0))"`).
//
// The host-cpp / executable / host-callable targets keep working
// because they pull in `slang.h` and `Slang::String` is in scope —
// `tests/cpu-program/string-passing.slang` exercises that path.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -stage compute -entry main

RWStructuredBuffer<int64_t> output;

// Helper with a `String` parameter — exercises the parameter-type
// branch in `_findStringUseInFunc` (caught even before any String
// member call appears in the body).
int64_t getLen(String s)
//CHECK:              ^ the 'String' type is not supported on the 'cpp' target; use 'NativeString' instead, or compile with -target host-cpp
//CHECK:              ^ use of 'String' here
{
    return s.getLength();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[0] = getLen("123");
}
