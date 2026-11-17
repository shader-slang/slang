// The coopVecLoad wrappers declare the union of the alternatives their
// CoopVec<T, N>.load callees declare: plain CUDA stays available through the
// cooperative_vector alternative, and the optix_coopvec alternative only adds
// the OptiX path. This pins the plain-CUDA compile so the union is not later
// narrowed into a conjunction.
//TEST:SIMPLE: -target cuda -stage compute -entry computeMain -o coop-vec-load-cuda-availability.cuda.cpp

RWByteAddressBuffer bab;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    CoopVec<float, 16> v = coopVecLoad<16, float>(bab, 0);
}
