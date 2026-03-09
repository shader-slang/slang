//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUFFER): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type

struct MyAddress : IDifferentiablePtrType
{
    typealias Differential = MyAddress;

    RWStructuredBuffer<float>.Handle handle;
    uint baseIndex;

    __init(RWStructuredBuffer<float>.Handle h)
    {
        handle = h;
        baseIndex = 0;
    }

    __subscript(uint index) -> float
    {
        [nonmutating]
        get { return handle[baseIndex + index]; }
    }

    [Differentiable]
    [ForwardDerivative(fwd_getOffset)]
    This getOffset(int elements)
    {
        This addr = This(handle);
        addr.baseIndex = baseIndex + uint(elements);
        return addr;
    }

    static DifferentialPtrPair<This> fwd_getOffset(DifferentialPtrPair<This> self, int elements)
    {
        return DifferentialPtrPair<This>(
            self.p.getOffset(elements),
            self.d.getOffset(elements));
    }
}

//TEST_INPUT: set buf = ubuffer(data=[1.0 2.0 3.0 4.0], stride=4)
uniform RWStructuredBuffer<float>.Handle buf;

//TEST_INPUT: set gradBuf = ubuffer(data=[0.0 0.0 0.0 0.0], stride=4)
uniform RWStructuredBuffer<float>.Handle gradBuf;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=result
RWStructuredBuffer<uint> result;

[Differentiable]
float compute(MyAddress addr, float x)
{
    let addr2 = addr.getOffset(2);
    return x * no_diff addr2[0];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let addr = MyAddress(buf);
    let gradAddr = MyAddress(gradBuf);
    var addrPair = DifferentialPtrPair<MyAddress>(addr, gradAddr);
    var xPair = diffPair(1.0f);
    bwd_diff(compute)(addrPair, xPair, 1.0f);
    result[0] = xPair.d == 3.0 ? 1 : 0;
    // BUFFER: 1
}
