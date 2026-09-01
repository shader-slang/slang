// Repro for finding cli-embed-downstream-ir-silent-failure.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
//
// `-embed-downstream-ir` succeeds with `-target spirv` and fails with exit
// code 255 and no output whatsoever for every other target tried
// (spirv-asm, hlsl, glsl, metal, wgsl, cpp).
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    RWStructuredBuffer<int> b;
    b[0] = int(tid.x);
}
