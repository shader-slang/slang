//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

struct Test
{
    uint a;
	uint b;
};


//CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test test[3] = {1, 2, 3, 4, 5, 6};

    outputBuffer[0] = true
        && test[0].a == 1
        && test[0].b == 2

        && test[1].a == 1
        && test[1].b == 2

        && test[2].a == 1
        && test[2].b == 2
        ;
}