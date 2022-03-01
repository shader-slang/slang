//TEST_IGNORE_FILE:
#version 450
#extension GL_NV_conservative_raster_underestimation : require
layout(row_major) uniform;
layout(row_major) buffer;

layout(location = 0)
out vec4 _S1;

void main()
{
    _S1 = vec4(uint(gl_FragFullyCoveredNV));
    return;
}
