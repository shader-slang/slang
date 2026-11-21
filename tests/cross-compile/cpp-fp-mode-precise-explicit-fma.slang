// Under `-fp-mode precise`, the C++ output disables contraction, which also covers the prelude. An
// explicit `fma` must still round once. a = b = 1 + 2^-12 and c = -(1 + 2^-11): the exact product
// is 1 + 2^-11 + 2^-24, so `fma(a, b, c)` is 2^-24 (0x33800000), while `a * b + c` gives 0.

//TEST:COMPARE_COMPUTE(targets=cpu+llvm, filecheck-buffer=CHECK):-xslang -fp-mode -xslang precise
//TEST:COMPARE_COMPUTE(targets=cpu+llvm, filecheck-buffer=CHECK):

//TEST_INPUT:ubuffer(data=[0x3F800800 0x3F800800 0xBF801000 0], stride=4):out,name io
RWStructuredBuffer<uint> io;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = asfloat(io[0]);
    float b = asfloat(io[1]);
    float c = asfloat(io[2]);
    io[3] = asuint(fma(a, b, c));
}

// CHECK: 3F800800
// CHECK-NEXT: 3F800800
// CHECK-NEXT: BF801000
// CHECK-NEXT: 33800000
