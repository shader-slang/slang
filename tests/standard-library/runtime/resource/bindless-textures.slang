// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;
import slang.runtime.resource.simple_texture;

typealias InputTexture = Texture2D<float, 0, TextureFormat.r32Float>;

struct TextureArguments
{
    Array<InputTexture.Handle, 2> inputs;
    RWTexture2D<uint>.Handle output;
}

// Only the returned handle keeps this texture alive. The highest mip has a distinct value.
[host]
[noinline]
InputTexture.Handle makeTextureHandle(float value)
{
    let texture = allocateReadOnlyTexture2D<float, TextureFormat.r32Float, 3>(4, 4);
    texture.clear(value);
    var range = SubresourceRange();
    range.mip = 2;
    range.mipCount = 1;
    texture.clear(value + 2.0, range);
    return texture.getDescriptorHandle();
}

[host]
void checkTextureHandles()
{
    let output = allocateTexture2D<uint>(2, 1);
    TextureArguments arguments;
    arguments.inputs[0] = makeTextureHandle(10.0);
    arguments.inputs[1] = makeTextureHandle(30.0);
    arguments.output = output.getDescriptorHandle();
    Grid(uint(2)).dispatch(
        (uint x) =>
        {
            let input = nonuniform(arguments.inputs[x]);
            uint width, height, levels;
            input.GetDimensions(0, width, height, levels);
            arguments.output[uint2(x, 0)] =
                uint(input.Load(int3(0, 0, 2))) + width + height + levels;
        });
    let recovered = *arguments.output;
    let checks = RWStructuredBuffer<uint>::alloc(2);
    Grid(uint(2)).dispatch(
        (uint x) =>
        { checks[x] = recovered[uint2(x, 0)]; });
    assert(checks.readBack(0) == 23);
    assert(checks.readBack(1) == 43);

    // Transition the same textures through clear, bindless UAV, and ordinary compute bindings.
    output.clear(7u);
    Grid(uint(2)).dispatch(
        (uint x) =>
        { arguments.output[uint2(x, 0)] += 3; });
    Grid(uint(2)).dispatch(
        (uint x) =>
        { checks[x] = output[uint2(x, 0)]; });
    assert(checks.readBack(0) == 10);
    assert(checks.readBack(1) == 10);

    // A formatted UAV uses the texture's storage conversion, not its shader element size.
    // Multiple allocated mips still produce a single-mip storage view.
    let unorm = allocateTexture2D<float4, TextureFormat.rgba8Unorm, 3>(4, 4);
    let unormHandle = unorm.getDescriptorHandle();
    Grid(uint(1)).dispatch(
        (uint x) =>
        { unormHandle[uint2(0)] = float4(1, 0, 0.5, 1); });
    Grid(uint(1)).dispatch(
        (uint x) =>
        {
            // Storage conversion can round the half-step to either adjacent UNORM8 value.
            checks[0] = all(abs(unorm[uint2(0)] - float4(1, 0, 0.5, 1)) < 1.0 / 255.0) ? 1 : 0;
        });
    assert(checks.readBack(0) == 1);
}

[host]
void checkDepthTextureHandle<let Format : int>()
{
    let depth = allocateDepthTarget2D<Format>(2, 2);
    depth.clear<TextureAspect.DepthOnly>(0.25);
    let handle = depth.getDescriptorHandle();
    let result = RWStructuredBuffer<float>::alloc(1);
    Grid(uint(1)).dispatch(
        (uint x) =>
        { result[0] = handle.Load(int3(0)); });
    float value = result.readBack(0);
    assert(value > 0.2499 && value < 0.2501);
}

[host]
void main()
{
    checkTextureHandles();
    checkDepthTextureHandle<TextureFormat.d32Float>();
    checkDepthTextureHandle<TextureFormat.d16Unorm>();
    checkDepthTextureHandle<TextureFormat.d32FloatS8Uint>();
}
