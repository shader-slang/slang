import user;
import playground;

[format("rgba8")]
[playground::BLACK_SCREEN(1.0, 1.0)]
WTexture2D                          outputTexture;

[shader("compute")]
[numthreads(16, 16, 1)]
[playground::CALL::SIZE_OF("outputTexture")]
void imageMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint width = 0;
    uint height = 0;
    outputTexture.GetDimensions(width, height);

    float4 color = imageMain(dispatchThreadID.xy, int2(width, height));

    if (dispatchThreadID.x >= width || dispatchThreadID.y >= height)
        return;

    outputTexture.Store(dispatchThreadID.xy, color);
}
