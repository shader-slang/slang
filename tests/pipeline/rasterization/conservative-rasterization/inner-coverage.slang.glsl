#version 450

#extension GL_NV_conservative_raster_underestimation : require

layout(location = 0)
out vec4 _S1;

void main()
{
    vec4 _S2;
    _S2 = vec4(uint(gl_FragFullyCoveredNV));
    _S1 = _S2;
    return;
}
