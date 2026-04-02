//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUFFER): -vk -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type

interface IAddressLike : IDifferentiablePtrType
{
    [ForwardDifferentiable]
    This getOffset(int elements);

    uint asIndex();
}

struct MyAddress : IAddressLike
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

    uint asIndex()
    {
        return uint(baseIndex);
    }

    [ForwardDerivative(fwd_getOffset)]
    This getOffset(int elements)
    {
        This addr = This(handle);
        addr.baseIndex = baseIndex + uint(elements);
        return addr;
    }

    [Differentiable]
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

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=result
RWStructuredBuffer<float> result;

[BackwardDerivative(load_bwd)]
float load<Addr : IAddressLike>(Addr addr)
    where Addr.Differential : IAddressLike
{
    return buf[addr.asIndex()];
}

void load_bwd<Addr: IAddressLike>(DifferentialPtrPair<Addr> dpAddr, float inp)
    where Addr.Differential : IAddressLike
{
    gradBuf[dpAddr.d.asIndex()] += inp;
}

[Differentiable]
float compute<Addr : IAddressLike>(Addr addr, float x)
    where Addr.Differential : IAddressLike
{
    let addr2 = addr.getOffset(2);
    return x * load(addr2);
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
    
    // Copy from gradbuf to output
    result[0] = gradBuf[0];
    result[1] = gradBuf[1];
    result[2] = gradBuf[2];
    // BUFFER: 0
    // BUFFER: 0
    // BUFFER: 1
}
