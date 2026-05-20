//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute
RWStructuredBuffer<int> outBuf;
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = {10, 20, 30, 40};
    outBuf[0] = arr[100];
//CHECK:  ^ array index out of bounds
}
