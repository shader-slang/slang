//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

[SpecializationConstant] const int MAX_ITERS = 0;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    for (int i = 0; i < MAX_ITERS; i++) {
        outputBuffer[i] = i;
    }
}

// CHECK-NOT: 30505
