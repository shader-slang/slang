//TEST:SIMPLE(filecheck=ERROR): -target hlsl -entry computeMain -stage compute

public struct Test
{
    // note: we need an initExpr here for an error because else Slang will disallow
    // synthisis of a 'public' constructor, meaning nothing has to fail.
    internal uint a = 5;
	public uint b;
};


//ERROR: error 30500

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