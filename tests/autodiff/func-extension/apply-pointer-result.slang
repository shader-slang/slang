// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

struct Buffer : IDifferentiablePtrType
{
    typealias Differential = Buffer;
    no_diff RWStructuredBuffer<float> data;
}

Buffer identity(Buffer value) { return value; }

struct IdentityContext
{
    void operator()(out __none_t gradient) { gradient = none; }
}

__func_extension __apply(identity)(DifferentialPtrPair<Buffer> value)
    -> Tuple<DifferentialPtrPair<Buffer>, IdentityContext>
{
    return makeTuple(value, IdentityContext());
}

float load(Buffer value) { return value.data[0]; }

__func_extension bwd_diff(load)(DifferentialPtrPair<Buffer> value, float seed) -> void
{
    value.d.data[0] += seed;
}

[BackwardDifferentiable]
float composed(Buffer value) { return load(identity(value)); }

//TEST_INPUT:ubuffer(data=[3.0], stride=4):name=primal
//TEST_INPUT:ubuffer(data=[0.0], stride=4):name=gradient
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> primal;
RWStructuredBuffer<float> gradient;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let value = DifferentialPtrPair<Buffer>(Buffer(primal), Buffer(gradient));
    let applied = __apply(identity)(value);
    let result = applied._0;
    outputBuffer[0] = result.p.data[0];    // CHECK: 3.0
    result.d.data[0] = 7.0f;
    outputBuffer[1] = gradient[0];         // CHECK: 7.0
    bwd_diff(composed)(value, 2.0f);
    outputBuffer[2] = gradient[0];         // CHECK: 9.0
}
