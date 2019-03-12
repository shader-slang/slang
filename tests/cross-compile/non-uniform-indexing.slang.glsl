//TEST_IGNORE_FILE
#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(binding = 0)
uniform texture2D t_0[10];

layout(binding = 1)
uniform sampler s_0;

layout(location = 0)
out vec4 _S1;

layout(location = 0)
in vec3 _S2;

void main()
{
    vec4 _S3 = texture(
    	sampler2D(
    		t_0[nonuniformEXT(int(_S2.z))],
    		s_0),
		_S2.xy);

    _S1 = _S3;
    return;
}
