import glsl;

#version 400


in VertexData
{
    vec2 texcoord_0;
} vd;

/*
in vec2 texcoord_0;
in vec2 texcoord_1;
in vec2 texcoord_2;
*/

out vec4 out1;



void main()

{

    out1 = vec4(vd.texcoord_0, vd.texcoord_0);
    //out1 = vec4(texcoord_0.x, texcoord_1.y, texcoord_2);

}

