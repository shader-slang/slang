// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

// Returning a differentiable pointer does not make a call entirely primal when it has
// a backward hook. The forward hook computes a new buffer; the backward context must
// propagate that buffer's adjoint to the input instead of just forwarding its handle.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):name=storage
RWStructuredBuffer<float> storage;

struct Buffer : IDifferentiablePtrType
{
    typealias Differential = Buffer;
    no_diff RWStructuredBuffer<float> data;
    no_diff uint index;

    Buffer twice()
    {
        storage[2] = 2.0f * data[index];
        return Buffer(storage, 2);
    }
}

__func_extension fwd_diff(Buffer::twice)(DifferentialPtrPair<Buffer> self)
    -> DifferentialPtrPair<Buffer>
{
    storage[4] += 1.0f;
    let output = self.p.twice();
    storage[3] = 2.0f * self.d.data[self.d.index];
    return DifferentialPtrPair<Buffer>(output, Buffer(storage, 3));
}

struct TwiceContext
{
    Buffer inputGradient;
    Buffer outputGradient;

    void operator()(out __none_t dSelf)
    {
        dSelf = none;
        storage[6] += 1.0f;
        inputGradient.data[inputGradient.index] +=
            2.0f * outputGradient.data[outputGradient.index];
    }
}

__func_extension __apply(Buffer::twice)(DifferentialPtrPair<Buffer> self)
    -> Tuple<DifferentialPtrPair<Buffer>, TwiceContext>
{
    storage[5] += 1.0f;
    let output = self.p.twice();
    let outputGradient = Buffer(storage, 3);
    storage[3] = 0.0f;
    return makeTuple(DifferentialPtrPair<Buffer>(output, outputGradient),
        TwiceContext(self.d, outputGradient));
}

float load(Buffer value) { return value.data[value.index]; }

__func_extension bwd_diff(load)(DifferentialPtrPair<Buffer> value, float seed)
{
    value.d.data[value.d.index] += seed;
}

[ForwardDifferentiable]
Buffer forwardComposition(Buffer input) { return input.twice(); }

// Exercise a synthesized backward association as well as the explicit apply hook.
[Differentiable]
Buffer identity(Buffer input) { return input; }

[BackwardDifferentiable]
float loss(Buffer input) { return load(identity(input).twice()); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    storage[0] = 3.0f;
    storage[1] = 1.0f;
    let input = DifferentialPtrPair<Buffer>(Buffer(storage, 0), Buffer(storage, 1));
    let result = fwd_diff(forwardComposition)(input);
    outputBuffer[0] = load(result.p); // CHECK: 6.0
    outputBuffer[1] = load(result.d); // CHECK: 2.0

    storage[1] = 0.0f;
    bwd_diff(loss)(input, 1.0f);
    outputBuffer[2] = storage[1]; // CHECK: 2.0
    bwd_diff(loss)(input, 1.0f);
    outputBuffer[3] = storage[1]; // CHECK: 4.0

    outputBuffer[4] = storage[4]; // CHECK: 1.0
    outputBuffer[5] = storage[5]; // CHECK: 2.0
    outputBuffer[6] = storage[6]; // CHECK: 2.0
}
