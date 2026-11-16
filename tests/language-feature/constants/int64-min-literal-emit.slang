//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -profile cs_6_0
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type

// The most negative int64_t is written as `(-9223372036854775807LL - 1LL)`, not as
// `-9223372036854775808LL`: the latter negates a literal that does not fit in a `long long`,
// which Apple's Metal compiler and clang warn about (-Wconstant-conversion /
// "integer literal is too large to be represented in a signed integer type").

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=8):out,name=outputBuffer
RWStructuredBuffer<int64_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK-NOT: 9223372036854775808
    // CHECK: (-9223372036854775807LL - 1LL)
    outputBuffer[0] = -9223372036854775807LL - 1;
    outputBuffer[1] = outputBuffer[0] + 1;
}

// BUF: type: int64_t
// BUF-NEXT: -9223372036854775808
// BUF-NEXT: -9223372036854775807
