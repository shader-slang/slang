// Corner case: multiple sequential barriers with data dependencies.
// Round 1: each thread writes its index, barrier, read neighbor.
// Round 2: each thread writes the value it read, barrier, read neighbor again.
// This chains two communication rounds through groupshared memory.
//
// Round 1: write idx → barrier → read (idx+1)%4
//   Thread 0 reads sharedData[1] = 1
//   Thread 1 reads sharedData[2] = 2
//   Thread 2 reads sharedData[3] = 3
//   Thread 3 reads sharedData[0] = 0
//
// Round 2: write what was read → barrier → read (idx+1)%4 again
//   sharedData = {1, 2, 3, 0} after round 2 writes
//   Thread 0 reads sharedData[1] = 2
//   Thread 1 reads sharedData[2] = 3
//   Thread 2 reads sharedData[3] = 0
//   Thread 3 reads sharedData[0] = 1

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

groupshared int sharedData[4];

[numthreads(4, 1, 1)]
void computeMain(uint idx : SV_GroupIndex)
{
    int neighbor = (int(idx) + 1) % 4;

    // Round 1: write own index
    sharedData[idx] = int(idx);
    GroupMemoryBarrierWithGroupSync();

    // Read neighbor's value from round 1
    int val = sharedData[neighbor];

    // Round 2: write what we just read
    GroupMemoryBarrierWithGroupSync();
    sharedData[idx] = val;
    GroupMemoryBarrierWithGroupSync();

    // Read neighbor's value from round 2 — effectively two hops
    outputBuffer[idx] = sharedData[neighbor];
}

// Each thread ends up reading 2 positions ahead (two neighbor hops):
// Thread 0: (0+2)%4 = 2
// Thread 1: (1+2)%4 = 3
// Thread 2: (2+2)%4 = 0
// Thread 3: (3+2)%4 = 1
// CHECK: 2
// CHECK: 3
// CHECK: 0
// CHECK: 1
