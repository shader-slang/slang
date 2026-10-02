//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute -matrix-layout-column-major

// A module-scope `static const` array whose length is a specialization constant, initialized from
// an array of another matrix layout, is not supported: such an array cannot be emitted even
// without a conversion. We pin the current message so that a change in behaviour is noticed.

[vk::constant_id(0)]
const int N = 3;

static const row_major float4x4 src[N] = (row_major float4x4[N])0;
static const float4x4 dst[N] = src;

RWStructuredBuffer<float> output;

float f(float4x4 x[N], uint i) { return x[i][0][0]; }

[numthreads(1, 1, 1)]
void main(uint3 t : SV_DispatchThreadID)
{
    output[0] = f(dst, t.x);
}

// CHECK: unimplemented: array layout conversion of a module-scope array whose length is not a literal
