#version 450
layout(row_major) uniform;
layout(row_major) buffer;

layout(location = 0)
out vec4 main_0;

void main()
{
    main_0 = vec4((ivec4(sign((vec4(1.5, 1.0, -1.5, -1.0))))));
    return;
}
