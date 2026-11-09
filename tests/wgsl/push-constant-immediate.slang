//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute

// A push-constant buffer maps to WGSL immediate data: a `var<immediate>` with no
// `@group`/`@binding`, enabled by `requires immediate_address_space;`.

// CHECK: requires immediate_address_space;
// CHECK: @binding(0) @group(0) var<storage, read_write> outputBuffer_{{.*}} : array<f32>;
// CHECK-NOT: var<uniform>
// CHECK: var<immediate> pc_{{[0-9]+}} : Params_std430_{{[0-9]+}};
// CHECK-NOT: var<uniform>

struct Params
{
    uint scale;
    float bias;
    float2 offset;
};

[[vk::push_constant]]
ConstantBuffer<Params> pc;

RWStructuredBuffer<float> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] =
        float(dispatchThreadID.x * pc.scale) + pc.bias + pc.offset.x + pc.offset.y;
}
