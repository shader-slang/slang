//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// While `_coerce` builds an optional conversion, it creates an internal declaration for the value
// stored in the optional. Semantic checking must capture the programmer-declared `capturedValue`
// without treating that internal declaration as another value from the enclosing function.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Optional<int> capturedValue = 7;
    let convertCapturedValue = () =>
    {
        Optional<float> convertedValue = capturedValue;
        return convertedValue;
    };

    Optional<float> result = convertCapturedValue();
    outputBuffer[0] = result.hasValue ? 1 : 0;
    outputBuffer[1] = result.hasValue ? int(result.value) : 0;
}

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 7
