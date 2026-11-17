// All ref/borrow parameter-passing mode spellings remain accepted on
// parameters: `__ref`, `__constref`, `__borrow` and the legacy `const __ref`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

float viaRef(__ref float x) { return x * 2.0; }
float viaConstref(__constref float x) { return x * 3.0; }
float viaBorrow(__borrow float x) { return x * 5.0; }
float viaLegacyConstRef(const __ref float x) { return x * 7.0; }

[numthreads(1, 1, 1)]
void computeMain()
{
    float v = 1.0;
    outputBuffer[0] = viaRef(v) + viaConstref(v) + viaBorrow(v) + viaLegacyConstRef(v);
    // CHECK: 17.0
}
