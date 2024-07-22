struct RadianceHitInfo { int x; };
struct Attributes      { float2 bary; };
struct ShadingState    { float4 tex; };

RWStructuredBuffer<float4> result;

void mdl_init(inout ShadingState state)
{
    //state.tex.x = 0;
    //state.tex.y = 0;
    //state.tex.z = 0;
}

void mdl_init_int(inout int state)
{
    //state.tex.x = 0;
    //state.tex.y = 0;
    //state.tex.z = 0;
}

[shader("closesthit")]
void fn(inout RadianceHitInfo payload, Attributes attrib)
{
    ShadingState mdl_state;
    mdl_init(mdl_state);
    result[0] = float4(mdl_state.tex.xyz, payload.x);
}
