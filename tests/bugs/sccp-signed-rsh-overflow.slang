//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test SCCP constant folding of signed right shift when shift count >= bit width.
// Arithmetic right shift of negative values should return -1 (sign-extended).
// Uses generic value parameters to ensure folding happens in SCCP (not frontend).

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct RshTest<let V : int, let S : int>
{
    static int compute() { return V >> S; }
};

[numthreads(1, 1, 1)]
void computeMain()
{
    // Negative value >> bitWidth: arithmetic shift fills with sign bit => -1
    outputBuffer[0] = RshTest<-1, 32>.compute();

    // Negative value >> (bitWidth + 1): same result
    outputBuffer[1] = RshTest<-1, 33>.compute();

    // Positive value >> bitWidth: all bits shifted out => 0
    outputBuffer[2] = RshTest<1, 32>.compute();

    // Normal case (shift < bitWidth): -4 >> 1 => -2
    outputBuffer[3] = RshTest<-4, 1>.compute();
}

// CHECK: -1
// CHECK: -1
// CHECK: 0
// CHECK: -2
