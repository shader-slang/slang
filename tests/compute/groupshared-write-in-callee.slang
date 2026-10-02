// The CPU targets, like Metal, materialize `groupshared` memory as a local of the entry
// point that callees reach through the kernel context. A read after a call that writes
// the slot must observe the callee's store.

//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0

// CPP: [[S:shared_[0-9]+]]{{\[}}[[T:t_[0-9]+]]{{\]}} = 1.0f;
// CPP: writeShared_{{[0-9]+}}([[T]], 2.0f,
// CPP: = [[S]]{{\[}}[[T]]{{\]}};

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float shared[4];

[noinline]
void writeShared(uint i, float x)
{
    shared[i] = x;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    shared[t] = 1.0;
    writeShared(t, 2.0);
    // BUF: 2.0
    // BUF-NEXT: 2.0
    // BUF-NEXT: 2.0
    // BUF-NEXT: 2.0
    outputBuffer[t] = shared[t];
}
