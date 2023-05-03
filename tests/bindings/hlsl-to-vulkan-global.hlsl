//TEST:REFLECTION:-target glsl -profile ps_4_0 -entry main -fvk-bind-globals 3 2

uniform int a;
uniform float b;

Texture2D t;

float4 main() : SV_TARGET
{
	return float4(1, 1, 1, 0);
}