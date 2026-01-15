struct GeometryOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color : COLOR;
};


[shader("geometry")]
[maxvertexcount(3)]
void main(
    inout PointStream<GeometryOutput> outputStream
    )
{
}
