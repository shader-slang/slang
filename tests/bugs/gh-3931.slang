//TEST:SIMPLE(filecheck=CHECK): -O0 -target spirv -emit-spirv-directly -stage compute -entry computeMain -matrix-layout-row-major
//COM:TEST:SIMPLE(filecheck=CHECK): -O2 -target spirv -emit-spirv-directly -stage compute -entry computeMain -matrix-layout-row-major

// Any level of optimization removes all OpMemberDecorations from FooBar
// with spirv-opt 2023 or upstream (2024-04). spirv-opt 2024-1 fixes the issue.

//CHECK: ColMajor

struct FooBar {
    float4x4 c;
    int load(int row, int col)
    {
        return int(c[row][col]);
        //return *(int*)int(c[row][col]); // Does not fail if using a pointer to any member to indirectly send data
    }
};
RWStructuredBuffer<int> outputBuffer;
uniform StructuredBuffer<FooBar, ScalarDataLayout> sb;
[numthreads(4, 1, 1)]
void computeMain(
    int3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = sb[0].load(dispatchThreadID.x/4, dispatchThreadID.x%4);
}