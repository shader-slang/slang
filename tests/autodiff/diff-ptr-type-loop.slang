//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

//TEST_INPUT: set storageBuffer = ubuffer(data=[0.0 0.0 0.0 0.0], stride=4)
uniform RWStructuredBuffer<float>.Handle storageBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// ----- MyPtrType definition -----
struct MyPtrType : IDifferentiablePtrType
{
    typealias Differential = MyPtrType;

    RWStructuredBuffer<float>.Handle buffer;
    uint offset;

    float load(uint idx) { return buffer[offset + idx]; }
    void accumulate(uint idx, float value) { buffer[offset + idx] += value; }
}

[BackwardDerivative(load_bwd)]
float load(MyPtrType b, uint idx)
{
    return b.load(idx);
}

void load_bwd(DifferentialPtrPair<MyPtrType> b, uint idx, float grad)
{
    b.d.accumulate(idx, grad);
}


// ------
[Differentiable]
float reduce(MyPtrType a, uint num)
{   
    float sum = 0;
    [MaxIters(3)]
    for (uint i = 0; i < num; i++)
    {
        sum += load(a, i);
    }

    return sum;
}

[Differentiable]
float test(MyPtrType b, uint num)
{
    return reduce(b, num);
}

[numthreads(1, 1, 1)]
void computeMain(uint id : SV_DispatchThreadID)
{
    storageBuffer[0] = 1; // BUFFER: 1
    storageBuffer[1] = 2; // BUFFER: 2

    // Denote the first two elements in the buffer as the primal buffer and the last two elements for the derivative.
    //
    // Note that storageBuffer is a bindless handle, which is required to robustly
    // pass buffers around as values, particularly with auto-diff which may create
    // more intermediate values of the buffer type.
    //
    var b = DifferentialPtrPair<MyPtrType>( { storageBuffer, 0 }, { storageBuffer, 2 } );

    bwd_diff(test)(b, 2, 1.5f);

    for (uint i = 0; i < 4; i++)
        outputBuffer[i] = storageBuffer[i];

    // Check locations [2] and [3] in the buffer
    // BUFFER: 1.5
    // BUFFER: 1.5
}