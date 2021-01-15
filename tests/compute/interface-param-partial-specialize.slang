// Tests generating dynamic dispatch code for a function
// with existential-struct-typed param by specializing it
// with __Dynamic. This verifies that the handling of
// "partially" specializing an existential type is correct.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cpu -shaderobj

[anyValueSize(8)]
interface IInterface
{
    uint eval();
}

public struct Impl : IInterface
{
    uint val;
    uint eval()
    {
        return val;
    }
};

struct Params
{
    StructuredBuffer<IInterface> obj;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<uint> gOutputBuffer;

void compute(uint tid, Params p)
{
    gOutputBuffer[tid] = p.obj[0].eval();
}

//TEST_INPUT: entryPointExistentialType __Dynamic

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID,
//TEST_INPUT:ubuffer(data=[rtti(Impl) witness(Impl, IInterface) 1 0], stride=4):name=params.obj
    uniform Params params)
{
	uint tid = dispatchThreadID.x;
	compute(tid, params);
}