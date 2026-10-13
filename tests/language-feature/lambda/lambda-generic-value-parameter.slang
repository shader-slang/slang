//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type
//TEST:SIMPLE(filecheck=IR): -target spirv -entry computeMain -stage compute -no-codegen -dump-ir

// A generic value parameter is a compile-time declaration owned by the generic function. A lambda
// may use the specialized value directly; the compiler must not add it to the runtime closure.

int invokeLambdaWithGenericValue<int value>()
{
    let getValue = () => value;
    return getValue();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = invokeLambdaWithGenericValue<42>();
}

// CHECK: type: int32_t
// CHECK-NEXT: 42

// The specialized generic value appears in the lambda body, but the closure itself remains empty.
// IR-LABEL: [nameHint("invokeLambdaWithGenericValue._slang_Lambda_invokeLambdaWithGenericValue_{{[0-9]+}}")]
// IR-NEXT: struct {{%[A-Za-z0-9_]+}} : Type;
