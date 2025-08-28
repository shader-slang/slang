import playground;

const static int MAX_BRUSH_SIZE = 16;

[playground::BLACK_SCREEN(1.0, 1.0)]
RWTexture2D<float> tex_red;
[playground::BLACK_SCREEN(1.0, 1.0)]
RWTexture2D<float> tex_green;
[playground::BLACK_SCREEN(1.0, 1.0)]
RWTexture2D<float> tex_blue;

[playground::SLIDER(10.0, 4.0, 16.0)]
uniform float brush_size;
[playground::COLOR_PICK(1.0, 0.0, 1.0)]
uniform float3 color;

[playground::MOUSE_POSITION]
uniform float4 mousePosition;

[shader("compute")]
[numthreads(8, 8, 1)]
[playground::CALL(MAX_BRUSH_SIZE, MAX_BRUSH_SIZE, 1)]
void draw(uint2 dispatchThreadId: SV_DispatchThreadID)
{
    if (mousePosition.z >= 0)
        return;

    let offset = float2(dispatchThreadId.xy) - float(MAX_BRUSH_SIZE) / 2;
    if (length(offset) > brush_size / 2)
        return;

    var mouse_pos = uint2(mousePosition.xy + offset);
    tex_red[mouse_pos] = color.r;
    tex_green[mouse_pos] = color.g;
    tex_blue[mouse_pos] = color.b;
}

float4 imageMain(uint2 dispatchThreadID, int2 screenSize)
{
    uint imageW;
    uint imageH;
    tex_red.GetDimensions(imageW, imageH);

    uint2 scaled = (uint2)floor(float2(dispatchThreadID.xy));
    uint2 flipped = uint2(scaled.x, imageH - scaled.y);

    float4 imageColor = float4(tex_red[flipped], tex_green[flipped], tex_blue[flipped], 1.0);
    return imageColor;
}