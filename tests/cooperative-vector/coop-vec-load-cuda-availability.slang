// Compile-smoke of the three buffer overloads whose declared [require]s gained
// the optix_coopvec alternative: plain CUDA still compiles them through the
// cooperative_vector alternative, and each overload is exercised so a future
// narrowing of the union into a conjunction shows up here.
//TEST:SIMPLE: -target cuda -stage compute -entry computeMain -o coop-vec-load-cuda-availability.cuda.cpp

RWByteAddressBuffer rwbab;
ByteAddressBuffer bab;
groupshared float gsData[16];

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    CoopVec<float, 16> v0 = coopVecLoad<16, float>(rwbab, 0);
    CoopVec<float, 16> v1 = coopVecLoad<16, float>(bab, 0);
    CoopVec<float, 16> v2 = coopVecLoadGroupshared<16, float, 16>(gsData, 0);
}
