//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

// CHECK: error{{.*}} expand
// CHECK: error{{.*}} each
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    expand int x = 0;
    each int y = 1;

    outputBuffer[0] = x;
    outputBuffer[1] = y;
    // CHECK-COUNT-2: 2
}
