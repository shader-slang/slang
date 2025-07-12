//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

// CHECK: error{{.*}} undefined identifier 'expand'
// CHECK: error{{.*}} undefined identifier 'each'
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    expand int x = 0;
    each int y = 1;

    outputBuffer[0] = x;
    outputBuffer[1] = y;
}
