#version 450
#extension GL_EXT_fragment_shader_barycentric : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 2605 "core.meta.slang"
layout(location = 0)
out vec4 entryPointParam_fsmain_0;


#line 10393 "hlsl.meta.slang"
pervertexEXT
layout(location = 0)
in int  vout_vertexID_0[3];


#line 12 "tests/spirv/get-vertex-attribute.slang"
void main()
{

#line 12
    entryPointParam_fsmain_0 = vec4(ivec4(vout_vertexID_0[0U]));

#line 12
    return;
}

