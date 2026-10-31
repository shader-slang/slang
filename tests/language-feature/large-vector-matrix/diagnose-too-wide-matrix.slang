// A matrix with too many columns is reported with its column count, also when it is only passed
// around whole, so that no row vector type of it is used, and when its row count is valid but
// larger than 4.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// CHECK-NOT: invalid element count '6'
// CHECK: vector has invalid element count '20', valid values are between '1' and '16' inclusive
// CHECK-NOT: invalid element count '6'

RWStructuredBuffer<matrix<float, 6, 20>> wideBuffer;

[noinline]
matrix<float, 6, 20> pass(matrix<float, 6, 20> m)
{
    return m;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    wideBuffer[0] = pass(wideBuffer[1]);
}
