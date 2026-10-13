// -Wno-31228 keeps the E31228 warning out of the compute comparison output.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-31228
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)

RWStructuredBuffer<uint> outputBuffer;

constexpr uint doubleIt(uint x) { return x * 2; }

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = doubleIt(5);
}

// CHECK: 10
