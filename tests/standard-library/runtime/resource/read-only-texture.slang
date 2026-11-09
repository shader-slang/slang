// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import slang.runtime.io.byte_stream;
import slang.runtime.core.simple_assert;
import slang.runtime.resource.simple_texture;
import slang.runtime.resource.texture_format;
import slang.compute.buffer_utils;
import slang.compute.texture_utils;

static const int kRGBA8 = TextureFormat::rgba8Unorm;

[shader("compute")]
[numthreads(1, 1, 1)]
void verify(Texture2D<float4, 0, kRGBA8> texture, RWStructuredBuffer<uint> result)
{
    uint4 red = uint4(texture[uint2(0, 0)] * 255.0f + 0.5f);
    uint4 green = uint4(texture[uint2(1, 0)] * 255.0f + 0.5f);
    uint4 blue = uint4(texture[uint2(0, 1)] * 255.0f + 0.5f);
    uint4 white = uint4(texture[uint2(1, 1)] * 255.0f + 0.5f);
    result[0] = all(red == uint4(255, 0, 0, 255)) && all(green == uint4(0, 255, 0, 255)) &&
                all(blue == uint4(0, 0, 255, 255)) && all(white == uint4(255, 255, 255, 255));
}

[host]
void main()
{
    Array<uint, 4> pixels = {
        0xff0000ff,
        0xff00ff00,
        0xffff0000,
        0xffffffff,
    };
    ByteStream output = openByteStream("read-only-texture.actual", ByteStreamMode::write);
    assert(writeByteStream(output, pixels) == 16);
    closeByteStream(output);

    Texture2D<float4, 0, kRGBA8> texture = allocateReadOnlyTexture2D<float4, kRGBA8>(2, 2);
    ByteStream input = openByteStream("read-only-texture.actual", ByteStreamMode::read);
    assert(readByteStreamTexture2D(input, texture, 16) == 16);
    closeByteStream(input);

    RWStructuredBuffer<uint> result = RWStructuredBuffer<uint>::alloc(1);
    result.fill(0);
    dispatch(verify, uint3(1, 1, 1))(texture, result);
    assert(readBack(result, 0) == 1);
}
