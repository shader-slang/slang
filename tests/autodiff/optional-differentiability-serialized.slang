//TEST:COMPILE: tests/autodiff/optional-diff-library.slang -o tests/autodiff/optional-diff-library.slang-module
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -xslang -experimental-feature

// Import prefers the precompiled module. This checks optional pack evidence and the
// pointer receiver's associated function declarations after an AST/IR serialization round trip.
import optional_diff_library;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var value = diffPair(3.0f);
    bwd_diff(serializedSquare<uint>)(7u, value, 1.0f);
    outputBuffer[0] = value.d; // CHECK: 6.0
    outputBuffer[1] = float(serializedCount<uint, float>()); // CHECK: 1.0
    outputBuffer[2] = float(serializedCount<>()); // CHECK: 0.0

    let function = SerializedIndexed();
    let receiver = DifferentialPtrPair<SerializedIndexed>(function, function);
    let applied = __apply(function.operator())(receiver, 7u, 3.0f);
    let backward = function.operator().remat(applied._1, receiver, 7u, 3.0f);
    __none_t receiverGradient;
    __none_t indexGradient;
    float gradient;
    backward(receiverGradient, indexGradient, gradient, 1.0f);
    outputBuffer[3] = applied._0 + gradient; // CHECK: 22.0
}
