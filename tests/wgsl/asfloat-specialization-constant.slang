//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCONST): -target wgsl -stage compute -entry computeMain
// There is no wgsl-spirv-asm (Tint) row: the slang-tint build that CI fetches (slang-tint-0) rejects
// `bitcast<f32>` of an override-expression, which current Tint accepts.

// A specialization constant is emitted as a WGSL `override`, and an expression that reads one is an
// override-expression, which a WGSL `const` initializer cannot hold. A function-scope value computed
// from a specialization constant is therefore bound with `var`. WGSL's `asfloat` is an IR bit cast,
// so `asfloat(kBits)` is such a value too, and so is `uint2(kBits, 1u)`. Each value is read twice so
// that it is bound to a name instead of being folded into a single use; the name's declaration is
// what NOCONST and the `var` checks inspect.

// NOCONST: result code = 0
// NOCONST-NOT: const {{.*}}kBits

[vk::constant_id(0)]
const uint kBits = 0x3F800000u;

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<uint2> vectorBuffer;

// WGSL: @id(0) override kBits{{.*}} : u32
// WGSL: fn computeMain
// WGSL: var {{.*}} : f32 = {{.*}}bitcast<f32>({{.*}}kBits
// WGSL: var {{.*}} : vec2<u32> = vec2<u32>(kBits
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float b = asfloat(kBits);
    outputBuffer[tid.x] = b;
    outputBuffer[tid.x + 1] = b * 2.0;

    uint2 v = uint2(kBits, 1u);
    vectorBuffer[tid.x] = v;
    vectorBuffer[tid.x + 1] = v + tid.xy;
}
