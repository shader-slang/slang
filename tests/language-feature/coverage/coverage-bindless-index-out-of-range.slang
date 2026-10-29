// Verifies that a bindless index is rejected before it can truncate through
// the compiler's internal `int` option storage, mirroring
// coverage-reserved-space-out-of-range.slang.
//
// Without the range check, 2147483648 truncates to a negative `int`, which is
// the value that selects the single-buffer form -- so a host asking for the
// shared descriptor array would silently get one binding per shader instead.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-bindless-index 2147483648

//CHECK: option '-trace-coverage-bindless-index' expects a value in range 0..2147483647, but got '2147483648'

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}
