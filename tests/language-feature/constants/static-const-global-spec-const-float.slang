//TEST:SIMPLE(filecheck=CHECK):-target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

// Only integer scalar initializers built from builtin operators constant-fold, so a float
// scalar or a vector `static const` global that names a specialization constant reaches
// the E31226 check, as an array does. A specialization constant is a constant expression,
// not a mutable global, so these initializers are accepted and read the constant's default
// value when it is not overridden. A specialization constant declared without `const` is
// still read-only, but its integer initializers do not fold, so `k` reaches the check too.

// CHECK-NOT: E31226
// CHECK: %F = OpSpecConstant %float 1.5
// CHECK: %S = OpSpecConstant %int 4
// render-test cannot override a specialization constant, and folding the default 1.5 into
// the initializers would give the same buffer values. These checks confirm that `scaled`
// and `vec.x` are computed from %F.
// CHECK: %scaled = OpFMul %float %F %float_2
// CHECK: OpStore %{{.*}} %scaled
// CHECK: OpStore %{{.*}} %F

[vk::constant_id(1)]
const float F = 1.5;

[vk::constant_id(2)]
int S = 4;

static const float scaled = F * 2.0;
static const float2 vec = float2(F, 1.0);
static const int k = S;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = scaled;
    outputBuffer[1] = vec.x;
    outputBuffer[2] = vec.y;
    outputBuffer[3] = float(k);
    // BUF: 3.0
    // BUF-NEXT: 1.5
    // BUF-NEXT: 1.0
    // BUF-NEXT: 4.0
}
