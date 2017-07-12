//TEST(smoke):SIMPLE:-profile ps_4_0 -no-checking -target reflection-json

// Check that we report sample-rate entry point input correctly

uniform texture2D t;
uniform sampler s;

sample in vec2 uv;

out vec4 c;

void main()
{
    c = texture(sampler2D(t,s), uv);
}
