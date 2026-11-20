//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum Count : uint
{
    [require(hlsl)] large = 2147483648u,
}

void f()
{
    // The retained case requirement must not hide the unsigned size from numeric validation.
    float values[uint(Count.large)];
//CHECK: error E30034
//CHECK: array size must not exceed 2147483647.
}
