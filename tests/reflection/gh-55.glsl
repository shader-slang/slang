//TEST(smoke):REFLECTION:-profile ps_4_0 -target glsl

// Confirm fix for GitHub issue #55

layout(set = 0, binding = 0)
uniform PerFrameCB
{
	vec2 offset;
	vec2 scale;
};

void main()
{}
