//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IBufferPointer<T> : IDifferentiablePtrType
{
    [Differentiable]
    T load(uint idx);
}

struct MyBufferPointer<T> : IBufferPointer<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias Differential = MyBufferPointer<T.Differential>;

    RWStructuredBuffer<T> buf;
    uint offset;

    [BackwardDerivative(load_bwd)]
    T load(uint idx) { return buf[offset + idx]; }

    static void load_bwd(DifferentialPtrPair<This> pair, uint idx, T.Differential grad)
    {
        pair.d.buf[pair.d.offset + idx] += grad;
    }
}

[Differentiable]
float test(MyBufferPointer<float> p, uint idx)
{
    return p.load(idx) + p.load(idx + 1);
}

[numthreads(1, 1, 1)]
void computeMain(uint id : SV_DispatchThreadID)
{
    outputBuffer[0] = 1; // CHECK: 1
    outputBuffer[1] = 2; // CHECK: 2

    // Denote the first two elements in the buffer as the primal buffer and the last two elements for the derivative.
    MyBufferPointer<float> p = { outputBuffer, 0 };
    MyBufferPointer<float>.Differential dp = { outputBuffer, 2 };
    var pair = DifferentialPtrPair<MyBufferPointer<float>>(p, dp);

    bwd_diff(test)(pair, 0, 1.5f);

    // Check locations [2] and [3] in the buffer
    // CHECK: 1.5
    // CHECK: 1.5
}
