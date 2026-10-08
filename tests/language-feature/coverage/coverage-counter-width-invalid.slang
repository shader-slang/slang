// Verifies that `-trace-coverage-counter-width` rejects values
// other than 32 or 64 with a clear diagnostic. The flag accepts
// exactly the two byte widths that the IR coverage pass and the
// LCOV converter support; any other value would either silently
// fall back to one of them or produce a buffer layout that no
// downstream tool can read. Each line below pins one representative
// out-of-range value so the rejection predicate cannot regress to
// something looser like `>= 32` or "any power of two":
//   - `0` exercises the degenerate-zero edge.
//   - `16` exercises a smaller power of two.
//   - `33` exercises a near-valid off-by-one.
//   - `128` exercises a larger power of two beyond the supported range.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 0
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 16
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 33
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 128

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

//CHECK: E45113
