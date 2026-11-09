// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

struct Scale : IDifferentiablePtrType
{
    typealias Differential = Scale;
    no_diff float value;

    T multiply<T : __BuiltinFloatingPointType>(T x) { return T(value) * x; }
}

struct ScaleContext
{
    float scale;

    void operator()(out __none_t dSelf, out float dx, float seed)
    {
        dSelf = none;
        dx = scale * seed;
    }
}

// The explicit paired receiver must be the implementation's only receiver parameter.
__func_extension __apply(Scale::multiply)(DifferentialPtrPair<Scale> self, float x)
    -> Tuple<float, ScaleContext>
{
    return makeTuple(self.p.multiply(x), ScaleContext(self.p.value));
}

[BackwardDifferentiable]
float evaluate(Scale scale, float x) { return scale.multiply(x); }

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let scale = Scale(3.0f);
    let pairedScale = DifferentialPtrPair<Scale>(scale, Scale(0.0f));
    let applied = __apply(scale.multiply)(pairedScale, 4.0f);
    __none_t dSelf;
    float dx;
    applied._1(dSelf, dx, 2.0f);
    outputBuffer[0] = applied._0; // CHECK: 12.0
    outputBuffer[1] = dx;         // CHECK: 6.0

    var x = diffPair(4.0f);
    bwd_diff(evaluate)(pairedScale, x, 2.0f);
    outputBuffer[2] = x.d;        // CHECK: 6.0
}
