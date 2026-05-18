// Verifies that reserved descriptor-set values are rejected
// before they can truncate through the compiler's internal `int`
// option storage.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-reserved-space 2147483648

//CHECK: option '-trace-coverage-reserved-space' expects a value in range 0..2147483647, but got '2147483648'

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}
