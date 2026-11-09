//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

struct Buffer : IDifferentiablePtrType
{
    typealias Differential = Buffer;
    no_diff RWStructuredBuffer<float> data;

    [Differentiable]
    float square(uint index, float value) { return value * value; }
}

void storeValue(Buffer destination, float value)
{
    destination.data[0] = value;
}

struct StoreContext
{
    Buffer gradient;

    void operator()(out __none_t destinationGradient, out float valueGradient)
    {
        destinationGradient = __none_t();
        valueGradient = gradient.data[0];
    }
}

__func_extension __apply(storeValue)(DifferentialPtrPair<Buffer> destination, float value)
    -> StoreContext
{
    storeValue(destination.p, value);
    return StoreContext(destination.d);
}

//TEST_INPUT:ubuffer(data=[0], stride=4):name=primal
//TEST_INPUT:ubuffer(data=[5.0], stride=4):name=gradient
//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> primal;
RWStructuredBuffer<float> gradient;
RWStructuredBuffer<float> outputBuffer;
static __none_t noGradient;

[numthreads(1, 1, 1)]
void computeMain()
{
    let destination = DifferentialPtrPair<Buffer>(Buffer(primal), Buffer(gradient));
    let minimal = __apply(storeValue)(destination, 3.0);
    typealias Function = __func_as_type(storeValue);
    let context = Function.remat(minimal, destination, 3.0);
    float valueGradient;
    context(noGradient, valueGradient);
    outputBuffer[0] = primal[0];          // CHECK: 3.0
    outputBuffer[1] = valueGradient;      // CHECK: 5.0

    var value = diffPair(7.0);
    bwd_diff(storeValue)(destination, value);
    outputBuffer[2] = value.d;           // CHECK: 5.0

    let applied = __apply(Buffer::square)(destination, 7, 3.0);
    typealias Method = __func_as_type(Buffer::square);
    let backward = Method.remat(applied._1, destination, 7, 3.0);
    __none_t indexGradient;
    backward(noGradient, indexGradient, valueGradient, 1.0);
    outputBuffer[3] = applied._0;        // CHECK: 9.0
    outputBuffer[4] = valueGradient;     // CHECK: 6.0
}
