#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 5 0
layout(rgba32f)
layout(binding = 0)
uniform image2D t_0;


#line 7
void writeColor_0(vec3 v_0)
{

#line 7
    ivec2 _S1 = ivec2(uvec2(0U, 0U));

#line 7
    vec4 _S2 = imageLoad(t_0,_S1);

#line 7
    vec4 _S3 = _S2;
    _S3.xyz = v_0;

#line 7
    imageStore(t_0,_S1,_S3);


    return;
}


#line 10
layout(location = 0)
out vec4 _S4;


#line 12
void main()
{
    writeColor_0(vec3(1.0));

#line 14
    _S4 = vec4(0.0);

#line 14
    return;
}

