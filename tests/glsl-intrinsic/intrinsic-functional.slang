//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=DX11):-allow-glsl -compute -dx11 -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=DX12):-allow-glsl -compute -dx12 -use-dxil -output-using-type -profile sm_6_0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=VK):-allow-glsl -compute -vk -emit-spirv-directly -output-using-type -profile sm_6_0 -render-feature hardware-device

#define TRIG_FUNCTIONS 0
#define EXP_FUNCTIONS 1
#define COMMON_FUNCTIONS 2
#define PACK_FUNCTIONS 3
#define GEOMETRIC_FUNCTIONS 4
#define MATRIX_FUNCTIONS 5
#define VEC_FUNCTIONS 6
#define INT_FUNCTIONS 7


float AngleAndTrig()
{
    float val = 0.0f;

	if( radians(90.0f) > 0.0f )
        val += 1.0f;
    if( degrees(1.0f) > 0.0f )
        val += 1.0f;
    if( sin(1.0f) > 0.0f )
        val += 1.0f;
	if( cos(1.0f) > 0.0f )
        val += 1.0f;
	if( tan(1.0f) > 0.0f )
        val += 1.0f;
	if( asin(1.0f) > 0.0f )
        val += 1.0f;
	if( acos(0.5f) > 0.0f )
        val += 1.0f;
	if( atan(1.0f) > 0.0f )
        val += 1.0f;
	if( sinh(1.0f) > 0.0f )
        val += 1.0f;
	if( cosh(1.0f) > 0.0f )
        val += 1.0f;
	if( tanh(1.0f) > 0.0f )
        val += 1.0f;

    vec2 v2 = radians(float2(90.0f, 90.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
    v2 = degrees(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
    v2 = sin(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = cos(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = tan(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = asin(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = acos(float2(0.5f, 0.5f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = atan(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = sinh(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = cosh(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;
	v2 = tanh(float2(1.0f, 1.0f));
	if( v2.x > 0.0f && v2.y > 0.0f )
        val += 1.0f;

	float3 v3 = radians(float3(90.0f, 90.0f, 90.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
    v3 = degrees(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
    v3 = sin(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = cos(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = tan(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = asin(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = acos(float3(0.5f, 0.5f, 0.5f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = atan(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = sinh(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = cosh(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;
	v3 = tanh(float3(1.0f, 1.0f, 1.0f));
	if( v3.x > 0.0f && v3.y > 0.0f && v3.z > 0.0f )
        val += 1.0f;

	float4 v4 = radians(float4(90.0f, 90.0f, 90.0f, 90.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
    v4 = degrees(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
    v4 = sin(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = cos(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = tan(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = asin(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = acos(float4(0.5f, 0.5f, 0.5f, 0.5f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = atan(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = sinh(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = cosh(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;
	v4 = tanh(float4(1.0f, 1.0f, 1.0f, 1.0f));
	if( v4.x > 0.0f && v4.y > 0.0f && v4.z > 0.0f && v4.w > 0.0f )
        val += 1.0f;

    return val;
}

float Exponential()
{
    float val = 0.0f;

    if (pow(2.0f, 8.0f) == 256.0f)
        val += 1.0f;
    if (exp(2.0f) > 7.0f && exp(2.0f) < 8.0f)
        val += 1.0f;
    if (log(10.0f) > 2.2f && log(10.0f) < 2.4f )
        val += 1.0f;
    if (exp2(4.0f) == 16.0f)
        val += 1.0f;
    if (log2(32.0f) == 5.0f)
        val += 1.0f;
    if (sqrt(256.0f) == 16.0f)
        val += 1.0f;
    if (sqrt(double(1024.0)) == double(32.0))
        val += 1.0f;
	if (inversesqrt(256.0f) == 0.0625f)
        val += 1.0f;
    if (inversesqrt(double(1024.0)) == double(0.03125))
        val += 1.0f;

    float2 f2 = pow(float2(2.0f, 2.0f), float2(8.0f, 8.0f));
    if (f2.x == 256.0f && f2.y == 256.0f)
        val += 1.0f;
    f2 = exp(float2(2.0f, 2.0f));
    if (f2.x > 7.0f && f2.x < 8.0f && f2.y > 7.0f && f2.y < 8.0f)
        val += 1.0f;
	f2 = log(float2(10.0f, 10.0f));
    if (f2.x > 2.2f && f2.x < 2.4f && f2.y > 2.2f && f2.y < 2.4f)
        val += 1.0f;
	f2 = exp2(float2(4.0f, 4.0f));
    if (f2.x == 16.0f && f2.y == 16.0f)
        val += 1.0f;
	f2 = log2(float2(32.0f, 32.0f));
    if (f2.x == 5.0f && f2.y == 5.0f)
        val += 1.0f;
    f2 = sqrt(float2(256.0f, 256.0f));
	if (f2.x == 16.0f && f2.y == 16.0f)
        val += 1.0f;
	double2 d2 = sqrt(double2(1024.0, 1024.0));
	if (d2.x == 32.0 && d2.y == 32.0)
        val += 1.0f;
    f2 = inversesqrt(float2(256.0f, 256.0f));
	if (f2.x == 0.0625f && f2.y == 0.0625f)
        val += 1.0f;
	d2 = inversesqrt(double2(1024.0, 1024.0));
	if (d2.x == 0.03125 && d2.y == 0.03125)
        val += 1.0f;

	float3 f3 = pow(float3(2.0f, 2.0f, 2.0f), float3(8.0f, 8.0f, 8.0f));
    if (f3.x == 256.0f && f3.y == 256.0f && f3.z == 256.0f)
        val += 1.0f;
    f3 = exp(float3(2.0f, 2.0f, 2.0f));
    if (f3.x > 7.0f && f3.x < 8.0f && f3.y > 7.0f && f3.y < 8.0f && f3.z > 7.0f && f3.z < 8.0f)
        val += 1.0f;
	f3 = log(float3(10.0f, 10.0f, 10.0f));
    if (f3.x > 2.2f && f3.x < 2.4f && f3.y > 2.2f && f3.y < 2.4f && f3.z > 2.2f && f3.z < 2.4f)
        val += 1.0f;
	f3 = exp2(float3(4.0f, 4.0f, 4.0f));
    if (f3.x == 16.0f && f3.y == 16.0f && f3.z == 16.0f)
        val += 1.0f;
	f3 = log2(float3(32.0f, 32.0f, 32.0f));
    if (f3.x == 5.0f && f3.y == 5.0f && f3.z == 5.0f)
        val += 1.0f;
    f3 = sqrt(float3(256.0f, 256.0f, 256.0f));
	if (f3.x == 16.0f && f3.y == 16.0f && f3.z == 16.0f)
        val += 1.0f;
	double3 d3 = sqrt(double3(1024.0, 1024.0, 1024.0));
	if (d3.x == 32.0 && d3.y == 32.0 && d3.z == 32.0)
        val += 1.0f;
    f3 = inversesqrt(float3(256.0f, 256.0f, 256.0f));
	if (f3.x == 0.0625f && f3.y == 0.0625f && f3.z == 0.0625f)
        val += 1.0f;
	d3 = inversesqrt(double3(1024.0, 1024.0, 1024.0));
	if (d3.x == 0.03125 && d3.y == 0.03125 && d3.z == 0.03125)
        val += 1.0f;

	float4 f4 = pow(float4(2.0f, 2.0f, 2.0f, 2.0f), float4(8.0f, 8.0f, 8.0f, 8.0f));
    if (f4.x == 256.0f && f4.y == 256.0f && f4.z == 256.0f && f4.w == 256.0f)
        val += 1.0f;
    f4 = exp(float4(2.0f, 2.0f, 2.0f, 2.0f));
    if (f4.x > 7.0f && f4.x < 8.0f && f4.y > 7.0f && f4.y < 8.0f && f4.z > 7.0f && f4.z < 8.0f && f4.w > 7.0f && f4.w < 8.0f)
        val += 1.0f;
	f4 = log(float4(10.0f, 10.0f, 10.0f, 10.0f));
    if (f4.x > 2.2f && f4.x < 2.4f && f4.y > 2.2f && f4.y < 2.4f && f4.z > 2.2f && f4.z < 2.4f && f4.w > 2.2f && f4.w < 2.4f)
        val += 1.0f;
	f4 = exp2(float4(4.0f, 4.0f, 4.0f, 4.0f));
    if (f4.x == 16.0f && f4.y == 16.0f && f4.z == 16.0f && f4.w == 16.0f)
        val += 1.0f;
	f4 = log2(float4(32.0f, 32.0f, 32.0f, 32.0f));
    if (f4.x == 5.0f && f4.y == 5.0f && f4.z == 5.0f && f4.w == 5.0f)
        val += 1.0f;
    f4 = sqrt(float4(256.0f, 256.0f, 256.0f, 256.0f));
	if (f4.x == 16.0f && f4.y == 16.0f && f4.z == 16.0f && f4.w == 16.0f)
        val += 1.0f;
	double4 d4 = sqrt(double4(1024.0, 1024.0, 1024.0, 1024.0));
	if (d4.x == 32.0 && d4.y == 32.0 && d4.z == 32.0 && d4.w == 32.0)
        val += 1.0f;
    f4 = inversesqrt(float4(256.0f, 256.0f, 256.0f, 256.0f));
	if (f4.x == 0.0625f && f4.y == 0.0625f && f4.z == 0.0625f && f4.w == 0.0625f)
        val += 1.0f;
	d4 = inversesqrt(double4(1024.0, 1024.0, 1024.0, 1024.0));
	if (d4.x == 0.03125 && d4.y == 0.03125 && d4.z == 0.03125 && d4.w == 0.03125)
        val += 1.0f;

    return val;
}

float Common()
{
    float val = 0.0f;

	// Floats
    if( abs(-5.0f) == 5.0f )
        val += 1.0f;
    vec2 v2 = abs(vec2(-5.0f, 10.0f));
    if (v2.x == 5.0f && v2.y == 10.0f)
        val += 1.0f;
	vec3 v3 = abs(vec3(-5.0f, 10.0f, -1.0f));
    if (v3.x == 5.0f && v3.y == 10.0f && v3.z == 1.0f)
        val += 1.0f;
	vec4 v4 = abs(vec4(-5.0f, 10.0f, -1.0f, -2.5f));
    if (v4.x == 5.0f && v4.y == 10.0f && v4.z == 1.0f && v4.w == 2.5f)
        val += 1.0f;

	if( sign(-5.0f) == -1.0f )
        val += 1.0f;
    v2 = sign(vec2(-5.0f, 10.0f));
    if (v2.x == -1.0f && v2.y == 1.0f)
        val += 1.0f;
	v3 = sign(vec3(-5.0f, 10.0f, -1.0f));
    if (v3.x == -1.0f && v3.y == 1.0f && v3.z == -1.0f)
        val += 1.0f;
	v4 = sign(vec4(-5.0f, 10.0f, -1.0f, -2.5f));
    if (v4.x == -1.0f && v4.y == 1.0f && v4.z == -1.0f && v4.w == -1.0f)
        val += 1.0f;

    if (floor(1.2f) == 1.0f)
        val += 1.0f;
    v2 = floor(vec2(1.2f, 2.9f));
    if (v2.x == 1.0f && v2.y == 2.0f)
        val += 1.0f;
	v3 = floor(vec3(1.2f, 2.9f, 3.5f));
    if (v3.x == 1.0f && v3.y == 2.0f && v3.z == 3.0f)
        val += 1.0f;
    v4 = floor(vec4(1.2f, 2.9f, 3.5f, 4.4f));
    if (v4.x == 1.0f && v4.y == 2.0f && v4.z == 3.0f && v4.w == 4.0f)
        val += 1.0f;

	if (trunc(1.2f) == 1.0f)
        val += 1.0f;
    v2 = trunc(vec2(1.2f, 2.9f));
    if (v2.x == 1.0f && v2.y == 2.0f)
        val += 1.0f;
	v3 = trunc(vec3(1.2f, 2.9f, 3.5f));
    if (v3.x == 1.0f && v3.y == 2.0f && v3.z == 3.0f)
        val += 1.0f;
    v4 = trunc(vec4(1.2f, 2.9f, 3.5f, 4.4f));
    if (v4.x == 1.0f && v4.y == 2.0f && v4.z == 3.0f && v4.w == 4.0f)
        val += 1.0f;

	if (round(1.2f) == 1.0f)
        val += 1.0f;
    v2 = round(vec2(1.2f, 2.9f));
    if (v2.x == 1.0f && v2.y == 3.0f)
        val += 1.0f;
	v3 = round(vec3(1.2f, 2.9f, 3.5f));
    if (v3.x == 1.0f && v3.y == 3.0f && v3.z == 4.0f)
        val += 1.0f;
    v4 = round(vec4(1.2f, 2.9f, 3.5f, 4.4f));
    if (v4.x == 1.0f && v4.y == 3.0f && v4.z == 4.0f && v4.w == 4.0f)
        val += 1.0f;

	if (ceil(1.2f) == 2.0f)
        val += 1.0f;
    v2 = ceil(vec2(1.2f, 2.9f));
    if (v2.x == 2.0f && v2.y == 3.0f)
        val += 1.0f;
	v3 = ceil(vec3(1.2f, 2.9f, 3.5f));
    if (v3.x == 2.0f && v3.y == 3.0f && v3.z == 4.0f)
        val += 1.0f;
    v4 = ceil(vec4(1.2f, 2.9f, 3.5f, 4.4f));
    if (v4.x == 2.0f && v4.y == 3.0f && v4.z == 4.0f && v4.w == 5.0f)
        val += 1.0f;

	// Currently does not work in any profile
	#if 0
	if (fract(1.2f) == 0.2f)
        val += 1.0f;
    v2 = fract(vec2(1.2f, 2.9f));
    if (v2.x == 0.2f && v2.y == 0.9f)
        val += 1.0f;
	v3 = fract(vec3(1.2f, 2.9f, 3.5f));
    if (v3.x == 0.2f && v3.y == 0.9f && v3.z == 0.5f)
        val += 1.0f;
    v4 = fract(vec4(1.2f, 2.9f, 3.5f, 4.4f));
    if (v4.x == 0.2f && v4.y == 0.9f && v4.z == 0.5f && v4.w == 0.4f)
        val += 1.0f;
	#endif

    if (mod(2.0f, 4.0f) == 2.0f)
        val += 1.0f;
	v2 = mod(vec2(2.0f, 2.0f), 4.0f);
    if (v2.x == 2.0f && v2.y == 2.0f)
        val += 1.0f;
	v3 = mod(vec3(2.0f, 2.0f, 2.0f), 4.0f);
    if (v3.x == 2.0f && v3.y == 2.0f && v3.z == 2.0f)
        val += 1.0f;
    v4 = mod(vec4(2.0f, 2.0f, 2.0f, 2.0f), 4.0f);
    if (v4.x == 2.0f && v4.y == 2.0f && v4.z == 2.0f && v4.w == 2.0f)
        val += 1.0f;
	v2 = mod(vec2(2.0f, 2.0f), vec2(4.0f, 4.0f));
    if (v2.x == 2.0f && v2.y == 2.0f)
        val += 1.0f;
    v3 = mod(vec3(2.0f, 2.0f, 2.0f), vec3(4.0f, 4.0f, 4.0f));
    if (v3.x == 2.0f && v3.y == 2.0f && v3.z == 2.0f)
        val += 1.0f;
	v4 = mod(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f));
    if (v4.x == 2.0f && v4.y == 2.0f && v4.z == 2.0f && v4.w == 2.0f)
        val += 1.0f;

    float fout = 0.0f;
    vec2 fout2;
    vec3 fout3;
    vec4 fout4;

    if (modf(1.5f, fout) == 0.5f && fout == 1.0f)
        val += 1.0f;
    v2 = modf(vec2(1.5f, 1.5f), fout2);
    if (v2.x == 0.5f && v2.y == 0.5f && fout2.x == 1.0f && fout2.y == 1.0f)
        val += 1.0f;
	v3 = modf(vec3(1.5f, 1.5f, 1.5f), fout3);
    if (v3.x == 0.5f && v3.y == 0.5f && v3.z == 0.5f && fout3.x == 1.0f && fout3.y == 1.0f && fout3.z == 1.0f )
        val += 1.0f;
	v4 = modf(vec4(1.5f, 1.5f, 1.5f, 1.5f), fout4);
    if (v4.x == 0.5f && v4.y == 0.5f && v4.z == 0.5f && v4.w == 0.5f && fout4.x == 1.0f && fout4.y == 1.0f && fout4.z == 1.0f && fout4.w == 1.0f )
        val += 1.0f;

	if (min(2.0f, 4.0f) == 2.0f)
        val += 1.0f;
	v2 = min(vec2(2.0f, 2.0f), 4.0f);
    if (v2.x == 2.0f && v2.y == 2.0f)
        val += 1.0f;
	v3 = min(vec3(2.0f, 2.0f, 2.0f), 4.0f);
    if (v3.x == 2.0f && v3.y == 2.0f && v3.z == 2.0f)
        val += 1.0f;
    v4 = min(vec4(2.0f, 2.0f, 2.0f, 2.0f), 4.0f);
    if (v4.x == 2.0f && v4.y == 2.0f && v4.z == 2.0f && v4.w == 2.0f)
        val += 1.0f;
	v2 = min(vec2(2.0f, 2.0f), vec2(4.0f, 4.0f));
    if (v2.x == 2.0f && v2.y == 2.0f)
        val += 1.0f;
    v3 = min(vec3(2.0f, 2.0f, 2.0f), vec3(4.0f, 4.0f, 4.0f));
    if (v3.x == 2.0f && v3.y == 2.0f && v3.z == 2.0f)
        val += 1.0f;
	v4 = min(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f));
    if (v4.x == 2.0f && v4.y == 2.0f && v4.z == 2.0f && v4.w == 2.0f)
        val += 1.0f;

	if (max(2.0f, 4.0f) == 4.0f)
        val += 1.0f;
	v2 = max(vec2(2.0f, 2.0f), 4.0f);
    if (v2.x == 4.0f && v2.y == 4.0f)
        val += 1.0f;
	v3 = max(vec3(2.0f, 2.0f, 2.0f), 4.0f);
    if (v3.x == 4.0f && v3.y == 4.0f && v3.z == 4.0f)
        val += 1.0f;
    v4 = max(vec4(2.0f, 2.0f, 2.0f, 2.0f), 4.0f);
    if (v4.x == 4.0f && v4.y == 4.0f && v4.z == 4.0f && v4.w == 4.0f)
        val += 1.0f;
	v2 = max(vec2(2.0f, 2.0f), vec2(4.0f, 4.0f));
    if (v2.x == 4.0f && v2.y == 4.0f)
        val += 1.0f;
    v3 = max(vec3(2.0f, 2.0f, 2.0f), vec3(4.0f, 4.0f, 4.0f));
    if (v3.x == 4.0f && v3.y == 4.0f && v3.z == 4.0f)
        val += 1.0f;
	v4 = max(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f));
    if (v4.x == 4.0f && v4.y == 4.0f && v4.z == 4.0f && v4.w == 4.0f)
        val += 1.0f;

    if (clamp(2.0f, 0.0f, 1.0f) == 1.0f)
        val += 1.0f;
    v2 = clamp(vec2(2.0f, 2.0f), 0.0f, 1.0f);
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
	v3 = clamp(vec3(2.0f, 2.0f, 2.0f), 0.0f, 1.0f);
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
	v4 = clamp(vec4(2.0f, 2.0f, 2.0f, 2.0f), 0.0f, 1.0f);
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;
    v2 = clamp(vec2(2.0f, 2.0f), vec2(0.0f, 0.0f), vec2(1.0f, 1.0f));
	if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
	v3 = clamp(vec3(2.0f, 2.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
	if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
	v4 = clamp(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;


    float m = mix(0.0f, 1.0f, 1.0f);
    if (m == 1.0f)
        val += 1.0f;
    float m_t = mix(0.0f, 1.0f, true);
    if (m_t == 1.0f)
        val += 1.0f;
    float m_f = mix(0.0f, 1.0f, false);
    if (m_f == 0.0f)
        val += 1.0f;

    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), 1.0f);
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), 1.0f);
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;

    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), true);
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), false);
    if (v2.x == 0.0f && v2.y == 0.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), true);
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), false);
    if (v3.x == 0.0f && v3.y == 0.0f && v3.z == 0.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), false);
    if (v4.x == 0.0f && v4.y == 0.0f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;

    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), vec2(1.0f, 1.0f));
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 0.0f));
    if (v2.x == 0.0f && v2.y == 0.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f));
    if (v3.x == 0.0f && v3.y == 0.0f && v3.z == 0.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (v4.x == 0.0f && v4.y == 0.0f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;

    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), bvec2(true, true));
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v2 = mix(vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), bvec2(false, false));
    if (v2.x == 0.0f && v2.y == 0.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), bvec3(true, true, true));
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v3 = mix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), bvec3(false, false, false));
    if (v3.x == 0.0f && v3.y == 0.0f && v3.z == 0.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), bvec4(true, true, true, true));
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;
    v4 = mix(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), bvec4(false, false, false, false));
    if (v4.x == 0.0f && v4.y == 0.0f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;


    float s = step(2.0f, 1.0f);
    if (s == 0.0f)
        val += 1.0f;
    s = step(2.0f, 2.0f);
    if (s == 1.0f)
        val += 1.0f;
    s = step(2.0f, 3.0f);
    if (s == 1.0f)
        val += 1.0f;

    v2 = step(2.0f, vec2(1.0f, 1.0f));
    if (v2.x == 0.0f && v2.y == 0.0f)
        val += 1.0f;
    v2 = step(2.0f, vec2(2.0f, 2.0f));
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v2 = step(2.0f, vec2(3.0f, 3.0f));
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v3 = step(2.0f, vec3(1.0f, 1.0f, 1.0f));
    if (v3.x == 0.0f && v3.y == 0.0f && v3.z == 0.0f)
        val += 1.0f;
    v3 = step(2.0f, vec3(2.0f, 2.0f, 2.0f));
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v3 = step(2.0f, vec3(3.0f, 3.0f, 3.0f));
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v4 = step(2.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (v4.x == 0.0f && v4.y == 0.0f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;
    v4 = step(2.0f, vec4(2.0f, 2.0f, 2.0f, 2.0f));
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;
    v4 = step(2.0f, vec4(3.0f, 3.0f, 3.0f, 3.0f));
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;

    v2 = step(vec2(2.0f, 2.0f), vec2(1.0f, 1.0f));
    if (v2.x == 0.0f && v2.y == 0.0f)
        val += 1.0f;
    v2 = step(vec2(2.0f, 2.0f), vec2(2.0f, 2.0f));
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v2 = step(vec2(2.0f, 2.0f), vec2(3.0f, 3.0f));
    if (v2.x == 1.0f && v2.y == 1.0f)
        val += 1.0f;
    v3 = step(vec3(2.0f, 2.0f, 2.0f), vec3(1.0f, 1.0f, 1.0f));
    if (v3.x == 0.0f && v3.y == 0.0f && v3.z == 0.0f)
        val += 1.0f;
    v3 = step(vec3(2.0f, 2.0f, 2.0f), vec3(2.0f, 2.0f, 2.0f));
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v3 = step(vec3(2.0f, 2.0f, 2.0f), vec3(3.0f, 3.0f, 3.0f));
    if (v3.x == 1.0f && v3.y == 1.0f && v3.z == 1.0f)
        val += 1.0f;
    v4 = step(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (v4.x == 0.0f && v4.y == 0.0f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;
    v4 = step(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(2.0f, 2.0f, 2.0f, 2.0f));
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;
    v4 = step(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(3.0f, 3.0f, 3.0f, 3.0f));
    if (v4.x == 1.0f && v4.y == 1.0f && v4.z == 1.0f && v4.w == 1.0f)
        val += 1.0f;


    float ss = smoothstep(2.0f, 4.0f, 1.0f);
    if (ss == 0.0f)
        val += 1.0f;
    ss = smoothstep(2.0f, 4.0f, 5.0f);
    if (ss == 1.0f)
        val += 1.0f;
    ss = smoothstep(2.0f, 4.0f, 3.0f);
    if (ss == 0.5f)
        val += 1.0f;

    v2 = smoothstep(2.0f, 4.0f, vec2(5.0f, 3.0f));
    if (v2.x == 1.0f && v2.y == 0.5f)
        val += 1.0f;
    v3 = smoothstep(2.0f, 4.0f, vec3(5.0f, 3.0f, 1.0f));
    if (v3.x == 1.0f && v3.y == 0.5f && v3.z == 0.0f)
        val += 1.0f;
    v4 = smoothstep(2.0f, 4.0f, vec4(5.0f, 3.0f, 1.0f, 1.0f));
    if (v4.x == 1.0f && v4.y == 0.5f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;

    v2 = smoothstep(vec2(2.0f, 2.0f), vec2(4.0f, 4.0f), vec2(5.0f, 3.0f));
    if (v2.x == 1.0f && v2.y == 0.5f)
        val += 1.0f;
    v3 = smoothstep(vec3(2.0f, 2.0f, 2.0f), vec3(4.0f, 4.0f, 4.0f), vec3(5.0f, 3.0f, 1.0f));
    if (v3.x == 1.0f && v3.y == 0.5f && v3.z == 0.0f)
        val += 1.0f;
    v4 = smoothstep(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f), vec4(5.0f, 3.0f, 1.0f, 1.0f));
    if (v4.x == 1.0f && v4.y == 0.5f && v4.z == 0.0f && v4.w == 0.0f)
        val += 1.0f;



    // This does not work in DX11
    bool nan = isnan(0.0f / 0.0f);
    if (nan)
        val += 1.0f;
    // ----

    // Does not work in DX12, but Vulkan is fine
#if 0
    bvec2 nan2 = isnan(vec2(0.0f / 0.0f, 0.0f / 0.0f));
    if (nan2.x == true && nan2.y == true)
       val += 1.0f;
	bvec3 nan3 = isnan(vec3(0.0f / 0.0f, 0.0f / 0.0f, 0.0f / 0.0f));
    if (nan3.x == true && nan3.y == true && nan3.z == true)
    	val += 1.0f;
	bvec4 nan4 = isnan(vec4(0.0f / 0.0f, 0.0f / 0.0f, 0.0f / 0.0f, 0.0f / 0.0f));
    if (nan4.x == true && nan4.y == true && nan4.z == true && nan4.w == true)
    	val += 1.0f;
#endif

    bool inf = isinf(1.0f / 0.0f);
    if (inf)
        val += 1.0f;
    bvec2 inf2 = isinf(vec2(1.0f / 0.0f, 1.0f / 0.0f));
    if (inf2.x == true && inf2.y == true)
        val += 1.0f;
    bvec3 inf3 = isinf(vec3(1.0f / 0.0f, 1.0f / 0.0f, 1.0f / 0.0f));
    if (inf3.x == true && inf3.y == true && inf3.z == true)
        val += 1.0f;
    bvec4 inf4 = isinf(vec4(1.0f / 0.0f, 1.0f / 0.0f, 1.0f / 0.0f, 1.0f / 0.0f));
    if (inf4.x == true && inf4.y == true && inf4.z == true && inf4.w == true)
        val += 1.0f;


    float f = fma(2.0f, 4.0f, 2.0f);
    if (f == 10.0f)
        val += 1.0f;

    // This currently does not work
#if 0
    v2 = fma(vec2(2.0f, 2.0f), vec2(4.0f, 4.0f), vec2(2.0f, 2.0f));
    if (v2.x == 10.0f && v2.y == 10.0f)
        val += 1.0f;
    v3 = fma(vec3(2.0f, 2.0f, 2.0f), vec3(4.0f, 4.0f, 4.0f), vec3(2.0f, 2.0f, 2.0f));
    if (v3.x == 10.0f && v3.y == 10.0f && v3.z == 10.0f)
        val += 1.0f;
    v4 = fma(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f), vec4(2.0f, 2.0f, 2.0f, 2.0f));
    if (v4.x == 10.0f && v4.y == 10.0f && v4.z == 10.0f && v4.w == 10.0f)
        val += 1.0f;
#endif


    int iout = 0;
    float fr = frexp(1.0f, iout);
    float ld = ldexp(fr, iout);
    if (ld == 1.0f && iout == 1)
        val += 1.0f;

    ivec2 iout2;
    vec2 fr2 = frexp(vec2(1.0f, 1.0f), iout2);
    vec2 ld2 = ldexp(fr2, iout2);
    if (ld2.x == 1.0f && ld2.y == 1.0f)
        val += 1.0f;

    ivec3 iout3;
    vec3 fr3 = frexp(vec3(1.0f, 1.0f, 1.0f), iout3);
    vec3 ld3 = ldexp(fr3, iout3);
    if (ld3.x == 1.0f && ld3.y == 1.0f && ld3.z == 1.0f)
        val += 1.0f;

    ivec4 iout4;
    vec4 fr4 = frexp(vec4(1.0f, 1.0f, 1.0f, 1.0f), iout4);
    vec4 ld4 = ldexp(fr4, iout4);
    if (ld4.x == 1.0f && ld4.y == 1.0f && ld4.z == 1.0f && ld4.w == 1.0f)
        val += 1.0f;


	
	// Doubles
	if( abs(double(-5.0)) == 5.0 )
        val += 1.0f;
    dvec2 dv2 = abs(dvec2(-5.0, 10.0));
    if (dv2.x == 5.0 && dv2.y == 10.0)
        val += 1.0f;
	dvec3 dv3 = abs(dvec3(-5.0, 10.0, -1.0));
    if (dv3.x == 5.0 && dv3.y == 10.0 && dv3.z == 1.0)
        val += 1.0f;
	dvec4 dv4 = abs(dvec4(-5.0, 10.0, -1.0, -2.5));
    if (dv4.x == 5.0 && dv4.y == 10.0 && dv4.z == 1.0 && dv4.w == 2.5)
        val += 1.0f;

	if( sign(double(-5.0)) == -1.0 )
        val += 1.0f;
    dv2 = sign(dvec2(-5.0, 10.0));
    if (dv2.x == -1.0 && dv2.y == 1.0)
        val += 1.0f;
	dv3 = sign(dvec3(-5.0, 10.0, -1.0));
    if (dv3.x == -1.0 && dv3.y == 1.0 && dv3.z == -1.0)
        val += 1.0f;
	dv4 = sign(dvec4(-5.0, 10.0, -1.0, -2.5));
    if (dv4.x == -1.0 && dv4.y == 1.0 && dv4.z == -1.0 && dv4.w == -1.0)
        val += 1.0f;

    if (floor(double(1.2)) == 1.0)
        val += 1.0f;
    dv2 = floor(dvec2(1.2, 2.9));
    if (dv2.x == 1.0 && dv2.y == 2.0)
        val += 1.0f;
	dv3 = floor(dvec3(1.2, 2.9, 3.5));
    if (dv3.x == 1.0 && dv3.y == 2.0 && dv3.z == 3.0)
        val += 1.0f;
    dv4 = floor(dvec4(1.2, 2.9, 3.5, 4.4));
    if (dv4.x == 1.0 && dv4.y == 2.0 && dv4.z == 3.0 && dv4.w == 4.0)
        val += 1.0f;

	if (trunc(double(1.2)) == 1.0)
        val += 1.0f;
    dv2 = trunc(dvec2(1.2, 2.9));
    if (dv2.x == 1.0 && v2.y == 2.0)
        val += 1.0f;
	dv3 = trunc(dvec3(1.2, 2.9, 3.5));
    if (dv3.x == 1.0 && v3.y == 2.0 && v3.z == 3.0)
        val += 1.0f;
    dv4 = trunc(dvec4(1.2, 2.9, 3.5, 4.4));
    if (dv4.x == 1.0 && dv4.y == 2.0 && dv4.z == 3.0 && dv4.w == 4.0)
        val += 1.0f;

	if (round(double(1.2)) == 1.0)
        val += 1.0f;
    dv2 = round(dvec2(1.2, 2.9));
    if (dv2.x == 1.0 && dv2.y == 3.0)
        val += 1.0f;
	dv3 = round(dvec3(1.2, 2.9, 3.5));
    if (dv3.x == 1.0 && dv3.y == 3.0 && dv3.z == 4.0)
        val += 1.0f;
    dv4 = round(dvec4(1.2, 2.9, 3.5, 4.4));
    if (dv4.x == 1.0 && dv4.y == 3.0 && dv4.z == 4.0 && dv4.w == 4.0)
        val += 1.0f;

	if (ceil(double(1.2)) == 2.0)
        val += 1.0f;
    dv2 = ceil(dvec2(1.2, 2.9));
    if (dv2.x == 2.0 && dv2.y == 3.0)
        val += 1.0f;
	dv3 = ceil(dvec3(1.2, 2.9, 3.5));
    if (dv3.x == 2.0 && dv3.y == 3.0 && dv3.z == 4.0)
        val += 1.0f;
    dv4 = ceil(dvec4(1.2, 2.9, 3.5, 4.4));
    if (dv4.x == 2.0 && dv4.y == 3.0 && dv4.z == 4.0 && dv4.w == 5.0)
        val += 1.0f;

	if (mod(double(2.0), double(4.0)) == 2.0)
        val += 1.0f;
	dv2 = mod(dvec2(2.0, 2.0), 4.0);
    if (dv2.x == 2.0 && dv2.y == 2.0)
        val += 1.0f;
	dv3 = mod(dvec3(2.0, 2.0, 2.0), 4.0);
    if (dv3.x == 2.0 && dv3.y == 2.0 && dv3.z == 2.0)
        val += 1.0f;
    dv4 = mod(dvec4(2.0, 2.0, 2.0, 2.0), 4.0);
    if (dv4.x == 2.0 && dv4.y == 2.0 && dv4.z == 2.0 && dv4.w == 2.0)
        val += 1.0f;
	dv2 = mod(dvec2(2.0, 2.0), dvec2(4.0, 4.0));
    if (dv2.x == 2.0 && dv2.y == 2.0)
        val += 1.0f;
    dv3 = mod(dvec3(2.0, 2.0, 2.0), dvec3(4.0, 4.0, 4.0));
    if (dv3.x == 2.0 && dv3.y == 2.0 && dv3.z == 2.0)
        val += 1.0f;
	dv4 = mod(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(4.0, 4.0, 4.0, 4.0));
    if (dv4.x == 2.0 && dv4.y == 2.0 && dv4.z == 2.0 && dv4.w == 2.0)
        val += 1.0f;

    double dout = 0.0;
    dvec2 dout2;
    dvec3 dout3;
    dvec4 dout4;

    if (modf(double(1.5), dout) == 0.5 && dout == 1.0)
        val += 1.0f;
    dv2 = modf(dvec2(1.5, 1.5), dout2);
    if (dv2.x == 0.5 && dv2.y == 0.5 && dout2.x == 1.0 && dout2.y == 1.0)
        val += 1.0f;
	dv3 = modf(dvec3(1.5, 1.5, 1.5), dout3);
    if (dv3.x == 0.5 && dv3.y == 0.5 && dv3.z == 0.5 && dout3.x == 1.0 && dout3.y == 1.0 && dout3.z == 1.0 )
        val += 1.0f;
	dv4 = modf(dvec4(1.5, 1.5, 1.5, 1.5), dout4);
    if (dv4.x == 0.5 && dv4.y == 0.5 && dv4.z == 0.5 && dv4.w == 0.5 && dout4.x == 1.0 && dout4.y == 1.0 && dout4.z == 1.0 && dout4.w == 1.0 )
        val += 1.0f;

	if (min(double(2.0), double(4.0)) == 2.0)
        val += 1.0f;
	dv2 = min(dvec2(2.0, 2.0), 4.0);
    if (dv2.x == 2.0 && dv2.y == 2.0)
        val += 1.0f;
	dv3 = min(dvec3(2.0, 2.0, 2.0), 4.0);
    if (dv3.x == 2.0 && dv3.y == 2.0 && dv3.z == 2.0)
        val += 1.0f;
    dv4 = min(dvec4(2.0, 2.0, 2.0, 2.0), 4.0);
    if (dv4.x == 2.0 && dv4.y == 2.0 && dv4.z == 2.0 && dv4.w == 2.0)
        val += 1.0f;
	dv2 = min(dvec2(2.0, 2.0), dvec2(4.0, 4.0));
    if (dv2.x == 2.0 && dv2.y == 2.0)
        val += 1.0f;
    dv3 = min(dvec3(2.0, 2.0, 2.0), dvec3(4.0, 4.0, 4.0));
    if (dv3.x == 2.0 && dv3.y == 2.0 && dv3.z == 2.0)
        val += 1.0f;
	dv4 = min(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(4.0, 4.0, 4.0, 4.0));
    if (dv4.x == 2.0 && dv4.y == 2.0 && dv4.z == 2.0 && dv4.w == 2.0)
        val += 1.0f;

	if (max(double(2.0), double(4.0)) == 4.0)
        val += 1.0f;
	dv2 = max(dvec2(2.0, 2.0), 4.0);
    if (dv2.x == 4.0 && dv2.y == 4.0)
        val += 1.0f;
	dv3 = max(dvec3(2.0, 2.0, 2.0), 4.0);
    if (dv3.x == 4.0 && dv3.y == 4.0 && dv3.z == 4.0)
        val += 1.0f;
    dv4 = max(dvec4(2.0, 2.0, 2.0, 2.0), 4.0);
    if (dv4.x == 4.0 && dv4.y == 4.0 && dv4.z == 4.0 && dv4.w == 4.0)
        val += 1.0f;
	dv2 = max(dvec2(2.0, 2.0), dvec2(4.0, 4.0));
    if (dv2.x == 4.0 && dv2.y == 4.0)
        val += 1.0f;
    dv3 = max(dvec3(2.0, 2.0, 2.0), dvec3(4.0, 4.0, 4.0));
    if (dv3.x == 4.0 && dv3.y == 4.0 && dv3.z == 4.0)
        val += 1.0f;
	dv4 = max(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(4.0, 4.0, 4.0, 4.0));
    if (dv4.x == 4.0 && dv4.y == 4.0 && dv4.z == 4.0 && dv4.w == 4.0)
        val += 1.0f;

	if (clamp(double(2.0), double(0.0), double(1.0)) == double(1.0))
        val += 1.0f;
    dv2 = clamp(dvec2(2.0, 2.0), 0.0, 1.0);
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
	dv3 = clamp(dvec3(2.0, 2.0, 2.0), 0.0, 1.0);
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
	dv4 = clamp(dvec4(2.0, 2.0, 2.0, 2.0), 0.0, 1.0);
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;
    dv2 = clamp(dvec2(2.0, 2.0), dvec2(0.0, 0.0), dvec2(1.0, 1.0));
	if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
	dv3 = clamp(dvec3(2.0, 2.0, 2.0), dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0));
	if (dv3.x == 1.0f && dv3.y == 1.0f && dv3.z == 1.0f)
        val += 1.0f;
	dv4 = clamp(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0));
	if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;

    double md = mix(double(0.0), double(1.0), double(1.0));
    if (md == 1.0)
        val += 1.0f;
    double md_t = mix(double(0.0), double(1.0), true);
    if (md_t == 1.0)
        val += 1.0f;
    double md_f = mix(double(0.0), double(1.0), false);
    if (md_f == 0.0)
        val += 1.0f;

    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), 1.0);
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0), 1.0);
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), 1.0);
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;

    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), true);
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), false);
    if (dv2.x == 0.0 && dv2.y == 0.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0), true);
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0), false);
    if (dv3.x == 0.0 && dv3.y == 0.0 && dv3.z == 0.0)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), true);
    if (dv4.x == 1.0f && dv4.y == 1.0f && dv4.z == 1.0f && dv4.w == 1.0f)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), false);
    if (dv4.x == 0.0 && dv4.y == 0.0 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;

    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), dvec2(1.0, 1.0));
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), dvec2(0.0, 0.0));
    if (dv2.x == 0.0 && dv2.y == 0.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0), dvec3(1.0, 1.0, 1.0));
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0f, 0.0f, 0.0f), dvec3(1.0f, 1.0f, 1.0f), dvec3(0.0f, 0.0f, 0.0f));
    if (dv3.x == 0.0f && dv3.y == 0.0f && dv3.z == 0.0f)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), dvec4(1.0, 1.0, 1.0, 1.0));
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), dvec4(0.0, 0.0, 0.0, 0.0));
    if (dv4.x == 0.0 && dv4.y == 0.0 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;

    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), bvec2(true, true));
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv2 = mix(dvec2(0.0, 0.0), dvec2(1.0, 1.0), bvec2(false, false));
    if (dv2.x == 0.0 && dv2.y == 0.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0), bvec3(true, true, true));
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv3 = mix(dvec3(0.0, 0.0, 0.0), dvec3(1.0, 1.0, 1.0), bvec3(false, false, false));
    if (dv3.x == 0.0 && dv3.y == 0.0 && dv3.z == 0.0)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), bvec4(true, true, true, true));
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;
    dv4 = mix(dvec4(0.0, 0.0, 0.0, 0.0), dvec4(1.0, 1.0, 1.0, 1.0), bvec4(false, false, false, false));
    if (dv4.x == 0.0 && dv4.y == 0.0 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;

    double sd = step(double(2.0), double(1.0));
    if (sd == 0.0)
        val += 1.0f;
    sd = step(double(2.0), double(2.0));
    if (sd == 1.0)
        val += 1.0f;
    sd = step(double(2.0), double(3.0));
    if (sd == 1.0)
        val += 1.0f;

    dv2 = step(2.0, dvec2(1.0, 1.0));
    if (dv2.x == 0.0 && dv2.y == 0.0)
        val += 1.0f;
    dv2 = step(2.0, dvec2(2.0, 2.0));
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv2 = step(2.0, dvec2(3.0, 3.0));
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv3 = step(2.0, dvec3(1.0, 1.0, 1.0));
    if (dv3.x == 0.0 && dv3.y == 0.0 && dv3.z == 0.0)
        val += 1.0f;
    dv3 = step(2.0, dvec3(2.0, 2.0, 2.0));
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv3 = step(2.0, dvec3(3.0, 3.0, 3.0));
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv4 = step(2.0, dvec4(1.0, 1.0, 1.0, 1.0));
    if (dv4.x == 0.0 && dv4.y == 0.0 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;
    dv4 = step(2.0, dvec4(2.0, 2.0, 2.0, 2.0));
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;
    dv4 = step(2.0, dvec4(3.0, 3.0, 3.0, 3.0));
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;

    dv2 = step(dvec2(2.0, 2.0), dvec2(1.0, 1.0));
    if (dv2.x == 0.0 && dv2.y == 0.0)
        val += 1.0f;
    dv2 = step(dvec2(2.0, 2.0), dvec2(2.0, 2.0));
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv2 = step(dvec2(2.0, 2.0), dvec2(3.0, 3.0));
    if (dv2.x == 1.0 && dv2.y == 1.0)
        val += 1.0f;
    dv3 = step(dvec3(2.0, 2.0, 2.0), dvec3(1.0, 1.0, 1.0));
    if (dv3.x == 0.0 && dv3.y == 0.0 && dv3.z == 0.0)
        val += 1.0f;
    dv3 = step(dvec3(2.0, 2.0, 2.0), dvec3(2.0, 2.0, 2.0));
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv3 = step(dvec3(2.0, 2.0, 2.0), dvec3(3.0, 3.0, 3.0));
    if (dv3.x == 1.0 && dv3.y == 1.0 && dv3.z == 1.0)
        val += 1.0f;
    dv4 = step(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(1.0, 1.0, 1.0, 1.0));
    if (dv4.x == 0.0 && dv4.y == 0.0 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;
    dv4 = step(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(2.0, 2.0, 2.0, 2.0));
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;
    dv4 = step(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(3.0, 3.0, 3.0, 3.0));
    if (dv4.x == 1.0 && dv4.y == 1.0 && dv4.z == 1.0 && dv4.w == 1.0)
        val += 1.0f;

    double dss = smoothstep(double(2.0), double(4.0), double(1.0));
    if (dss == 0.0)
        val += 1.0f;
    dss = smoothstep(double(2.0), double(4.0), double(5.0));
    if (dss == 1.0)
        val += 1.0f;
    dss = smoothstep(double(2.0), double(4.0), double(3.0));
    if (dss == 0.5)
        val += 1.0f;

    dv2 = smoothstep(2.0, 4.0, dvec2(5.0, 3.0));
    if (dv2.x == 1.0 && dv2.y == 0.5)
        val += 1.0f;
    dv3 = smoothstep(2.0, 4.0, dvec3(5.0, 3.0, 1.0));
    if (dv3.x == 1.0 && dv3.y == 0.5 && dv3.z == 0.0)
        val += 1.0f;
    dv4 = smoothstep(2.0, 4.0, dvec4(5.0, 3.0, 1.0, 1.0));
    if (dv4.x == 1.0 && dv4.y == 0.5 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;

    dv2 = smoothstep(dvec2(2.0, 2.0), dvec2(4.0, 4.0), dvec2(5.0, 3.0));
    if (dv2.x == 1.0 && dv2.y == 0.5)
        val += 1.0f;
    dv3 = smoothstep(dvec3(2.0, 2.0, 2.0), dvec3(4.0, 4.0, 4.0), dvec3(5.0, 3.0, 1.0));
    if (dv3.x == 1.0 && dv3.y == 0.5 && dv3.z == 0.0)
        val += 1.0f;
    dv4 = smoothstep(dvec4(2.0, 2.0, 2.0, 2.0), dvec4(4.0, 4.0, 4.0, 4.0), dvec4(5.0, 3.0, 1.0, 1.0));
    if (dv4.x == 1.0 && dv4.y == 0.5 && dv4.z == 0.0 && dv4.w == 0.0)
        val += 1.0f;


    // This does not work in DX11
    bool dnan = isnan(double(0.0) / double(0.0));
    if (dnan)
        val += 1.0f;
    // ----
    // TODO: Add vector versions once fixed


    bool dinf = isinf(double(1.0) / double(0.0));
    if (dinf)
        val += 1.0f;
    bvec2 dinf2 = isinf(dvec2(double(1.0) / double(0.0), double(1.0) / double(0.0)));
    if (dinf2.x == true && dinf2.y == true)
        val += 1.0f;
    bvec3 dinf3 = isinf(dvec3(double(1.0) / double(0.0), double(1.0) / double(0.0), double(1.0) / double(0.0)));
    if (dinf3.x == true && dinf3.y == true && dinf3.z == true)
        val += 1.0f;
    bvec4 dinf4 = isinf(dvec4(double(1.0) / double(0.0), double(1.0) / double(0.0), double(1.0) / double(0.0), double(1.0) / double(0.0)));
    if (dinf4.x == true && dinf4.y == true && dinf4.z == true && dinf4.w == true)
        val += 1.0f;


    double fd = fma(2.0f, 4.0f, 2.0f);
    if (fd == 10.0f)
        val += 1.0f;

    // This currently does not work (TODO: change to double type when fixed)
#if 0
    v2 = fma(vec2(2.0f, 2.0f), vec2(4.0f, 4.0f), vec2(2.0f, 2.0f));
    if (v2.x == 10.0f && v2.y == 10.0f)
        val += 1.0f;
    v3 = fma(vec3(2.0f, 2.0f, 2.0f), vec3(4.0f, 4.0f, 4.0f), vec3(2.0f, 2.0f, 2.0f));
    if (v3.x == 10.0f && v3.y == 10.0f && v3.z == 10.0f)
        val += 1.0f;
    v4 = fma(vec4(2.0f, 2.0f, 2.0f, 2.0f), vec4(4.0f, 4.0f, 4.0f, 4.0f), vec4(2.0f, 2.0f, 2.0f, 2.0f));
    if (v4.x == 10.0f && v4.y == 10.0f && v4.z == 10.0f && v4.w == 10.0f)
        val += 1.0f;
#endif


    iout = 0;
    double frd = frexp(double(1.0), iout);
    double ldd = ldexp(frd, iout);
    if (ldd == 1.0 && iout == 1)
        val += 1.0f;

    // Does not work in Vulkan
        /*
            Validation: error 0: Validation Error: [ UNASSIGNED-Debug-Printf ] | MessageID = 0x9472fbd3 | vkCreateShaderModule():  Error during shader instrumentation in spirv-opt: 
                line 8978: GLSL.std.450 Exp2: expected Result Type to be a 16 or 32-bit scalar or vector float type
                
                %15642 = OpExtInst %v2double %314 Exp2 %14322
    */
#if 0
    iout2;
    dvec2 frd2 = frexp(dvec2(1.0, 1.0), iout2);
    dvec2 ldd2 = ldexp(frd2, iout2);
    if (ldd2.x == 1.0 && ldd2.y == 1.0)
        val += 1.0f;

    iout3;
    dvec3 frd3 = frexp(dvec3(1.0f, 1.0f, 1.0f), iout3);
    dvec3 ldd3 = ldexp(frd3, iout3);
    if (ldd3.x == 1.0 && ldd3.y == 1.0 && ldd3.z == 1.0)
        val += 1.0f;

    iout4;
    dvec4 frd4 = frexp(dvec4(1.0f, 1.0f, 1.0f, 1.0f), iout4);
    dvec4 ldd4 = ldexp(frd4, iout4);
    if (ldd4.x == 1.0 && ldd4.y == 1.0 && ldd4.z == 1.0 && ldd4.w == 1.0)
        val += 1.0f;
#endif



	// Integers
	if( abs(-5) == 5 )
        val += 1.0f;
    ivec2 iv2 = abs(ivec2(-5, 10));
    if (iv2.x == 5 && iv2.y == 10)
        val += 1.0f;
	ivec3 iv3 = abs(ivec3(-5, 10, -1));
    if (iv3.x == 5 && iv3.y == 10 && iv3.z == 1)
        val += 1.0f;
	ivec4 iv4 = abs(ivec4(-5, 10, -1, -2));
    if (iv4.x == 5 && iv4.y == 10 && iv4.z == 1 && iv4.w == 2)
        val += 1.0f;
	
	if( sign(-5) == -1 )
        val += 1.0f;
    iv2 = sign(ivec2(-5, 10));
    if (iv2.x == -1 && iv2.y == 1)
        val += 1.0f;
	iv3 = sign(ivec3(-5, 10, -1));
    if (iv3.x == -1 && iv3.y == 1 && iv3.z == -1)
        val += 1.0f;
	iv4 = sign(ivec4(-5, 10, -1, -2));
    if (iv4.x == -1 && iv4.y == 1 && iv4.z == -1 && iv4.w == -1)
        val += 1.0f;
	
	if (min(2, 4) == 2)
        val += 1.0f;
	iv2 = min(ivec2(2, 2), 4);
    if (iv2.x == 2 && iv2.y == 2)
        val += 1.0f;
	iv3 = min(ivec3(2, 2, 2), 4);
    if (iv3.x == 2 && iv3.y == 2 && iv3.z == 2)
        val += 1.0f;
    iv4 = min(ivec4(2, 2, 2, 2), 4);
    if (iv4.x == 2 && iv4.y == 2 && iv4.z == 2 && iv4.w == 2)
        val += 1.0f;
	iv2 = min(ivec2(2, 2), ivec2(4, 4));
    if (iv2.x == 2 && iv2.y == 2)
        val += 1.0f;
    iv3 = min(ivec3(2, 2, 2), ivec3(4, 4, 4));
    if (iv3.x == 2 && iv3.y == 2 && iv3.z == 2)
        val += 1.0f;
	iv4 = min(ivec4(2, 2, 2, 2), ivec4(4, 4, 4, 4));
    if (iv4.x == 2 && iv4.y == 2 && iv4.z == 2 && iv4.w == 2)
        val += 1.0f;

	if (max(2, 4) == 4)
        val += 1.0f;
	iv2 = max(ivec2(2, 2), 4);
    if (iv2.x == 4 && iv2.y == 4)
        val += 1.0f;
	iv3 = max(ivec3(2, 2, 2), 4);
    if (iv3.x == 4 && iv3.y == 4 && iv3.z == 4)
        val += 1.0f;
    iv4 = max(ivec4(2, 2, 2, 2), 4);
    if (iv4.x == 4 && iv4.y == 4 && iv4.z == 4 && iv4.w == 4)
        val += 1.0f;
	iv2 = max(ivec2(2, 2), ivec2(4, 4));
    if (iv2.x == 4 && iv2.y == 4)
        val += 1.0f;
    iv3 = max(ivec3(2, 2, 2), ivec3(4, 4, 4));
    if (iv3.x == 4 && iv3.y == 4 && iv3.z == 4)
        val += 1.0f;
	iv4 = max(ivec4(2, 2, 2, 2), ivec4(4, 4, 4, 4));
    if (iv4.x == 4 && iv4.y == 4 && iv4.z == 4 && iv4.w == 4)
        val += 1.0f;

	if (clamp(2, 0, 1) == 1)
        val += 1.0f;
    iv2 = clamp(ivec2(2, 2), 0, 1);
    if (iv2.x == 1 && iv2.y == 1)
        val += 1.0f;
	iv3 = clamp(ivec3(2, 2, 2), 0, 1);
    if (iv3.x == 1 && iv3.y == 1 && iv3.z == 1)
        val += 1.0f;
	iv4 = clamp(ivec4(2, 2, 2, 2), 0, 1);
    if (iv4.x == 1 && iv4.y == 1 && iv4.z == 1 && iv4.w == 1)
        val += 1.0f;
    iv2 = clamp(ivec2(2, 2), ivec2(0, 0), ivec2(1, 1));
	if (iv2.x == 1 && iv2.y == 1)
        val += 1.0f;
	iv3 = clamp(ivec3(2, 2, 2), ivec3(0, 0, 0), ivec3(1, 1, 1));
	if (iv3.x == 1 && iv3.y == 1 && iv3.z == 1)
        val += 1.0f;
	iv4 = clamp(ivec4(2, 2, 2, 2), ivec4(0, 0, 0, 0), ivec4(1, 1, 1, 1));
	if (iv4.x == 1 && iv4.y == 1 && iv4.z == 1 && iv4.w == 1)
        val += 1.0f;

    // floatBitsToInt / intBitsToFloat
    int ibits = floatBitsToInt(1.0f);
    float fbits = intBitsToFloat(ibits);
    if (fbits == 1.0f)
        val += 1.0f;
    ivec2 ibits2 = floatBitsToInt(vec2(1.0f, 1.0f));
    vec2 fbits2 = intBitsToFloat(ibits2);
    if (fbits2.x == 1.0f && fbits2.y == 1.0f)
        val += 1.0f;
    ivec3 ibits3 = floatBitsToInt(vec3(1.0f, 1.0f, 1.0f));
    vec3 fbits3 = intBitsToFloat(ibits3);
    if (fbits3.x == 1.0f && fbits3.y == 1.0f && fbits3.z == 1.0f)
        val += 1.0f;
    ivec4 ibits4 = floatBitsToInt(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    vec4 fbits4 = intBitsToFloat(ibits4);
    if (fbits4.x == 1.0f && fbits4.y == 1.0f && fbits4.z == 1.0f && fbits4.w == 1.0f)
        val += 1.0f;


	// Unsigned
	if (min(2u, 4u) == 2u)
        val += 1.0f;
	uvec2 uv2 = min(uvec2(2u, 2u), 4u);
    if (uv2.x == 2u && uv2.y == 2u)
        val += 1.0f;
	uvec3 uv3 = min(uvec3(2u, 2u, 2u), 4u);
    if (uv3.x == 2u && uv3.y == 2u && uv3.z == 2u)
        val += 1.0f;
    uvec4 uv4 = min(uvec4(2u, 2u, 2u, 2u), 4u);
    if (uv4.x == 2u && uv4.y == 2u && uv4.z == 2u && uv4.w == 2u)
        val += 1.0f;
	uv2 = min(uvec2(2u, 2u), uvec2(4u, 4u));
    if (uv2.x == 2u && uv2.y == 2u)
        val += 1.0f;
    uv3 = min(uvec3(2u, 2u, 2u), uvec3(4u, 4u, 4u));
    if (uv3.x == 2u && uv3.y == 2u && uv3.z == 2u)
        val += 1.0f;
	uv4 = min(uvec4(2u, 2u, 2u, 2u), uvec4(4u, 4u, 4u, 4u));
    if (uv4.x == 2u && uv4.y == 2u && uv4.z == 2u && uv4.w == 2u)
        val += 1.0f;

	if (max(2u, 4u) == 4u)
        val += 1.0f;
	uv2 = max(uvec2(2u, 2u), 4u);
    if (uv2.x == 4u && uv2.y == 4u)
        val += 1.0f;
	uv3 = max(uvec3(2u, 2u, 2u), 4u);
    if (uv3.x == 4u && uv3.y == 4u && uv3.z == 4u)
        val += 1.0f;
    uv4 = max(uvec4(2u, 2u, 2u, 2u), 4u);
    if (uv4.x == 4u && uv4.y == 4u && uv4.z == 4u && uv4.w == 4u)
        val += 1.0f;
	uv2 = max(uvec2(2u, 2u), uvec2(4u, 4u));
    if (uv2.x == 4u && uv2.y == 4u)
        val += 1.0f;
    uv3 = max(uvec3(2u, 2u, 2u), uvec3(4u, 4u, 4u));
    if (uv3.x == 4u && uv3.y == 4u && uv3.z == 4u)
        val += 1.0f;
	uv4 = max(uvec4(2u, 2u, 2u, 2u), uvec4(4u, 4u, 4u, 4u));
    if (uv4.x == 4u && uv4.y == 4u && uv4.z == 4u && uv4.w == 4u)
        val += 1.0f;

	if (clamp(2u, 0u, 1u) == 1u)
        val += 1.0f;
    uv2 = clamp(uvec2(2u, 2u), 0u, 1u);
    if (uv2.x == 1u && uv2.y == 1u)
        val += 1.0f;
	uv3 = clamp(uvec3(2u, 2u, 2u), 0u, 1u);
    if (uv3.x == 1u && uv3.y == 1u && uv3.z == 1u)
        val += 1.0f;
	uv4 = clamp(uvec4(2u, 2u, 2u, 2u), 0u, 1u);
    if (uv4.x == 1u && uv4.y == 1u && uv4.z == 1u && uv4.w == 1u)
        val += 1.0f;
    uv2 = clamp(uvec2(2u, 2u), uvec2(0u, 0u), uvec2(1u, 1u));
	if (uv2.x == 1u && uv2.y == 1u)
        val += 1.0f;
	uv3 = clamp(uvec3(2u, 2u, 2u), uvec3(0u, 0u, 0u), uvec3(1u, 1u, 1u));
	if (uv3.x == 1u && uv3.y == 1u && uv3.z == 1u)
        val += 1.0f;
	uv4 = clamp(uvec4(2u, 2u, 2u, 2u), uvec4(0u, 0u, 0u, 0u), uvec4(1u, 1u, 1u, 1u));
	if (uv4.x == 1u && uv4.y == 1u && uv4.z == 1u && uv4.w == 1u)
        val += 1.0f;

    // floatBitsToUint / uintBitsToFloat
    uint uibits = floatBitsToUint(1.0f);
    fbits = uintBitsToFloat(uibits);
    if (fbits == 1.0f)
        val += 1.0f;
    uvec2 uibits2 = floatBitsToUint(vec2(1.0f, 1.0f));
    fbits2 = uintBitsToFloat(uibits2);
    if (fbits2.x == 1.0f && fbits2.y == 1.0f)
        val += 1.0f;
    uvec3 uibits3 = floatBitsToUint(vec3(1.0f, 1.0f, 1.0f));
    fbits3 = uintBitsToFloat(uibits3);
    if (fbits3.x == 1.0f && fbits3.y == 1.0f && fbits3.z == 1.0f)
        val += 1.0f;
    uvec4 uibits4 = floatBitsToUint(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    fbits4 = uintBitsToFloat(uibits4);
    if (fbits4.x == 1.0f && fbits4.y == 1.0f && fbits4.z == 1.0f && fbits4.w == 1.0f)
        val += 1.0f;


    return val;
}

float Pack()
{
    float val = 0.0f;

	const float EPSILON = 1.0f / 65535.0f;
	const float EPSILON2 = 1.0f / 32767.0f * 2.0f;
	const float EPSILON3 = 1.0f / 255.0f;
	const float EPSILON4 = 1.0f / 127.0f;

	// 2x16
    vec2 v2 = vec2(0.2f, 0.9f);

    uint P = packUnorm2x16(v2);
    vec2 U = unpackUnorm2x16(P);

    vec2 v = abs(v2 - U);
	if( v.x < EPSILON && v.y < EPSILON )
        val += 1.0f;

	

    P = packSnorm2x16(v2);
    U = unpackSnorm2x16(P);

    v = abs(v2 - U);
	if( v.x < EPSILON2 && v.y < EPSILON2 )
        val += 1.0f;


	// 4x8 are not working as expected
#if 0
	// 4x8
    vec4 v4 = vec4(1.0f, 2.0f, 3.0f, 4.0f);

	P = packUnorm4x8(v4);
    vec4 U4 = unpackUnorm4x8(P);

    vec4 v4diff = abs(v4 - U4);
	if( v4diff.x < EPSILON3 && v4diff.y < EPSILON3 && v4diff.z < EPSILON3 && v4diff.w < EPSILON3 )
        val += 1.0f;

    P = packSnorm4x8(v4);
    U4 = unpackSnorm4x8(P);

	v4diff = abs(v4 - U4);
	if( v4diff.x < EPSILON4 && v4diff.y < EPSILON4 && v4diff.z < EPSILON4 && v4diff.w < EPSILON4 )
        val += 1.0f;
#endif

	// Half 2x16
	P = packHalf2x16(v2);
    U = unpackHalf2x16(P);

    v = abs(v2 - U);
	if( v.x < EPSILON4 && v.y < EPSILON4 )
        val += 1.0f;


	// Not working as expected
#if 0
    // Double 2x32
    uvec2 uv2 = uvec2(1, 2);
    double PD = packDouble2x32(uv2);
    uvec2 U2 = unpackDouble2x32(PD);

    if (uv2.x == U2.x && uv2.y == U2.y)
        val += 1.0f;
#endif

    return val;
}

float Geometric()
{
    float val = 0.0f;

	// Length
	float len = length(1.0f);
	if (len == 1.0f)
        val += 1.0f;
	len = length(vec2(1.0f, 0.0f));
	if (len == 1.0f)
        val += 1.0f;
	len = length(vec3(1.0f, 0.0f, 0.0f));
	if (len == 1.0f)
        val += 1.0f;
	len = length(vec4(1.0f, 0.0f, 0.0f, 0.0f));
	if (len == 1.0f)
        val += 1.0f;

	double dlen = length(double(1.0));
	if (dlen == 1.0)
        val += 1.0f;
	dlen = length(dvec2(1.0, 0.0));
	if (dlen == 1.0)
        val += 1.0f;
	dlen = length(dvec3(1.0, 0.0, 0.0));
	if (dlen == 1.0)
        val += 1.0f;
	dlen = length(dvec4(1.0, 0.0, 0.0, 0.0));
	if (dlen == 1.0)
        val += 1.0f;
    
	// Distance
	float dist  = distance(1.0f, 1.0f);
    if (dist == 0.0f)
        val += 1.0f;
	float dist2 = distance(vec2(1.0f, 0.0f), vec2(1.0f, 0.0f));
	if (dist == 0.0f)
        val += 1.0f;
	float dist3 = distance(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
	if (dist == 0.0f)
        val += 1.0f;
	float dist4 = distance(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f));
	if (dist == 0.0f)
        val += 1.0f;

	double ddist  = distance(double(1.0), double(1.0));
    if (ddist == 0.0)
        val += 1.0f;
	double ddist2 = distance(dvec2(1.0, 0.0), dvec2(1.0, 0.0));
	if (ddist == 0.0)
        val += 1.0f;
	double ddist3 = distance(dvec3(1.0, 0.0, 0.0), dvec3(1.0, 0.0, 0.0));
	if (ddist == 0.0)
        val += 1.0f;
	double ddist4 = distance(dvec4(1.0, 0.0, 0.0, 0.0), dvec4(1.0, 0.0, 0.0, 0.0));
	if (ddist == 0.0)
        val += 1.0f;

	// Dot
    float dot1 = dot(1.0f, 1.0f);
    if (dot1 == 1.0f)
        val += 1.0f;
	float dot2 = dot(vec2(1.0f, 1.0f), vec2(1.0f, 1.0f));
    if (dot2 == 2.0f)
        val += 1.0f;
	float dot3 = dot(vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
    if (dot3 == 3.0f)
        val += 1.0f;
	float dot4 = dot(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (dot4 == 4.0f)
        val += 1.0f;

	double ddot1 = dot(double(1.0), double(1.0));
    if (ddot1 == 1.0)
        val += 1.0f;
	double ddot2 = dot(dvec2(1.0, 1.0), dvec2(1.0, 1.0));
    if (ddot2 == 2.0)
        val += 1.0f;
	double ddot3 = dot(dvec3(1.0, 1.0, 1.0), dvec3(1.0, 1.0, 1.0));
    if (ddot3 == 3.0)
        val += 1.0f;
	double ddot4 = dot(dvec4(1.0, 1.0, 1.0, 1.0), dvec4(1.0, 1.0, 1.0, 1.0));
    if (ddot4 == 4.0)
        val += 1.0f;

	// Cross
    vec3 xprod = cross(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    if (xprod.x == 0.0f && xprod.y == 0.0f && xprod.z == 1.0f)
        val += 1.0f;

	dvec3 dxprod = cross(dvec3(1.0, 0.0, 0.0), dvec3(0.0, 1.0, 0.0));
    if (dxprod.x == 0.0 && dxprod.y == 0.0 && dxprod.z == 1.0)
        val += 1.0f;

    // Normalize
    float norm = normalize(2.0f);
    if (norm == 1.0f)
        val += 1.0f;
    vec2 norm2 = normalize(vec2(2.0f, 0.0f));
    if (norm2.x == 1.0f && norm2.y == 0.0f)
        val += 1.0f;
	vec3 norm3 = normalize(vec3(0.0f, 2.0f, 0.0f));
    if (norm3.x == 0.0f && norm3.y == 1.0f && norm3.z == 0.0f)
        val += 1.0f;
	vec4 norm4 = normalize(vec4(0.0f, 0.0f, 0.0f, 5.0f));
    if (norm4.x == 0.0f && norm4.y == 0.0f && norm4.z == 0.0f && norm4.w == 1.0f)
        val += 1.0f;

	double dnorm = normalize(double(2.0));
    if (dnorm == 1.0)
        val += 1.0f;
    dvec2 dnorm2 = normalize(dvec2(2.0, 0.0));
    if (dnorm2.x == 1.0 && dnorm2.y == 0.0)
        val += 1.0f;
	dvec3 dnorm3 = normalize(dvec3(0.0, 2.0, 0.0));
    if (dnorm3.x == 0.0 && dnorm3.y == 1.0 && dnorm3.z == 0.0)
        val += 1.0f;
	dvec4 dnorm4 = normalize(dvec4(0.0, 0.0, 0.0, 5.0));
    if (dnorm4.x == 0.0 && dnorm4.y == 0.0 && dnorm4.z == 0.0 && dnorm4.w == 1.0)
        val += 1.0f;

    // Faceforward
    float forward = faceforward(1.0f, 1.0f, 1.0f);
    if (forward == -1.0f)
        val += 1.0f;
    vec2 forward2 = faceforward(vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 1.0f));
    if (forward2.x == -1.0f && forward2.y == -1.0f)
        val += 1.0f;
	vec3 forward3 = faceforward(vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
    if (forward3.x == -1.0f && forward3.y == -1.0f && forward3.z == -1.0f)
        val += 1.0f;
	vec4 forward4 = faceforward(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (forward4.x == -1.0f && forward4.y == -1.0f && forward4.z == -1.0f && forward4.w == -1.0f)
        val += 1.0f;

	double dforward = faceforward(double(1.0), double(1.0), double(1.0));
    if (dforward == -1.0)
        val += 1.0f;
    dvec2 dforward2 = faceforward(dvec2(1.0, 1.0), dvec2(1.0, 1.0), dvec2(1.0, 1.0));
    if (dforward2.x == -1.0 && dforward2.y == -1.0)
        val += 1.0f;
	dvec3 dforward3 = faceforward(dvec3(1.0, 1.0, 1.0), dvec3(1.0, 1.0, 1.0), dvec3(1.0, 1.0, 1.0));
    if (dforward3.x == -1.0 && dforward3.y == -1.0 && dforward3.z == -1.0)
        val += 1.0f;
	dvec4 dforward4 = faceforward(dvec4(1.0, 1.0, 1.0, 1.0), dvec4(1.0, 1.0, 1.0, 1.0), dvec4(1.0, 1.0, 1.0, 1.0));
    if (dforward4.x == -1.0f && dforward4.y == -1.0f && dforward4.z == -1.0f && dforward4.w == -1.0f)
        val += 1.0f;

    // Reflect

	// Doesn't work on DX12/DXIL
    float refl = reflect(1.0f, 1.0f);
    if (refl == -1.0f)
        val += 1.0f;
	// ----
    vec2 refl2 = reflect(vec2(1.0f, -1.0f), vec2(0.0f, 1.0f));
    if (refl2.x == 1.0f && refl2.y == 1.0f)
        val += 1.0f;
	vec3 refl3 = reflect(vec3(1.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    if (refl3.x == 1.0f && refl3.y == 1.0f && refl3.z == 0.0f)
        val += 1.0f;
	vec4 refl4 = reflect(vec4(1.0f, -1.0f, 0.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f));
    if (refl4.x == 1.0f && refl4.y == 1.0f && refl4.z == 0.0f && refl4.w == 0.0f)
        val += 1.0f;

	// Doesn't work on DX12/DXIL
	double drefl = reflect(double(1.0), double(1.0));
    if (drefl == -1.0)
        val += 1.0f;
	// ----
    dvec2 drefl2 = reflect(dvec2(1.0, -1.0), dvec2(0.0, 1.0));
    if (drefl2.x == 1.0 && drefl2.y == 1.0)
        val += 1.0f;
	dvec3 drefl3 = reflect(dvec3(1.0, -1.0, 0.0), dvec3(0.0, 1.0, 0.0));
    if (drefl3.x == 1.0 && drefl3.y == 1.0 && drefl3.z == 0.0)
        val += 1.0f;
	dvec4 drefl4 = reflect(dvec4(1.0, -1.0, 0.0, 0.0), dvec4(0.0, 1.0, 0.0, 0.0));
    if (drefl4.x == 1.0 && drefl4.y == 1.0 && drefl4.z == 0.0 && drefl4.w == 0.0)
        val += 1.0f;

	// Refract
	float refr = refract(1.0f, 1.0f, 0.5f);
    if (refr == -1.0f)
        val += 1.0f;
    vec2 refr2 = refract(vec2(1.0f, -1.0f), vec2(0.0f, 1.0f), 0.5f);
    if (refr2.x == 0.5f && refr2.y == -1.0f)
        val += 1.0f;
	vec3 refr3 = refract(vec3(1.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.5f);
    if (refr3.x == 0.5f && refr3.y == -1.0f && refr3.z == 0.0f)
        val += 1.0f;
	vec4 refr4 = refract(vec4(1.0f, -1.0f, 0.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f), 0.5f);
    if (refr4.x == 0.5f && refr4.y == -1.0f && refr4.z == 0.0f && refr4.w == 0.0f)
        val += 1.0f;

	double drefr = refract(double(1.0), double(1.0), double(0.5));
    if (drefr == -1.0)
        val += 1.0f;
    dvec2 drefr2 = refract(dvec2(1.0, -1.0), dvec2(0.0, 1.0), double(0.5));
    if (drefr2.x == 0.5 && drefr2.y == -1.0)
        val += 1.0f;
	dvec3 drefr3 = refract(dvec3(1.0, -1.0, 0.0), dvec3(0.0, 1.0, 0.0), double(0.5));
    if (drefr3.x == 0.5 && drefr3.y == -1.0 && drefr3.z == 0.0)
        val += 1.0f;
	dvec4 drefr4 = refract(dvec4(1.0, -1.0, 0.0, 0.0), dvec4(0.0, 1.0, 0.0, 0.0), double(0.5));
    if (drefr4.x == 0.5 && drefr4.y == -1.0 && drefr4.z == 0.0 && drefr4.w == 0.0)
        val += 1.0f;

    return val;
}

float Matrix()
{
    float val = 0.0f;

	// matrixCompMult
	mat2 m2 = mat2(0.0f, 1.0f, 2.0f, 3.0f);
	mat2 m2comp = matrixCompMult(m2, m2);
	mat2 m2expected = mat2(0.0f, 1.0f, 4.0f, 9.0f);
	if( all(equal(m2comp[0], m2expected[0])) && all(equal(m2comp[1], m2expected[1])))
        val += 1.0f;

    mat2x3 m23 = mat2x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
    mat2x3 m23comp = matrixCompMult(m23, m23);
    mat2x3 m23expected = mat2x3(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f);    
	if( all(equal(m23comp[0], m23expected[0])) && all(equal(m23comp[1], m23expected[1])) && all(equal(m23comp[2], m23expected[2])) )
        val += 1.0f;

    mat2x4 m24 = mat2x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);
    mat2x4 m24comp = matrixCompMult(m24, m24);
    mat2x4 m24expected = mat2x4(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f);
	if( all(equal(m24comp[0], m24expected[0])) && all(equal(m24comp[1], m24expected[1])) && all(equal(m24comp[2], m24expected[2])) && all(equal(m24comp[3], m24expected[3])) )
        val += 1.0f;

	mat3 m3 = mat3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
	mat3 m3comp = matrixCompMult(m3, m3);
	mat3 m3expected = mat3(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f);
	if( all(equal(m3comp[0], m3expected[0])) && all(equal(m3comp[1], m3expected[1])) && all(equal(m3comp[2], m3expected[2])))
        val += 1.0f;

	mat3x2 m32 = mat3x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
    mat3x2 m32comp = matrixCompMult(m32, m32);
    mat3x2 m32expected = mat3x2(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f);    
	if( all(equal(m32comp[0], m32expected[0])) && all(equal(m32comp[1], m32expected[1])) )
        val += 1.0f;

	mat3x4 m34 = mat3x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
    mat3x4 m34comp = matrixCompMult(m34, m34);
    mat3x4 m34expected = mat3x4(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f, 81.0f, 100.0f, 121.0f);    
	if( all(equal(m34comp[0], m34expected[0])) && all(equal(m34comp[1], m34expected[1])) && all(equal(m34comp[2], m34expected[2]))&& all(equal(m34comp[3], m34expected[3])) )
        val += 1.0f;

	mat4 m4 = mat4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f);
	mat4 m4comp = matrixCompMult(m4, m4);
	mat4 m4expected = mat4(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f, 81.0f, 100.0f, 121.0f, 144.0f, 169.0f, 196.0f, 225.0f);
	if( all(equal(m4comp[0], m4expected[0])) && all(equal(m4comp[1], m4expected[1])) && all(equal(m4comp[2], m4expected[2])) && all(equal(m4comp[3], m4expected[3])) )
        val += 1.0f;

	mat4x2 m42 = mat4x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);
    mat4x2 m42comp = matrixCompMult(m42, m42);
    mat4x2 m42expected = mat4x2(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f);    
	if( all(equal(m42comp[0], m42expected[0])) && all(equal(m42comp[1], m42expected[1])) )
        val += 1.0f;

	mat4x3 m43 = mat4x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
    mat4x3 m43comp = matrixCompMult(m43, m43);
    mat4x3 m43expected = mat4x3(0.0f, 1.0f, 4.0f, 9.0f, 16.0f, 25.0f, 36.0f, 49.0f, 64.0f, 81.0f, 100.0f, 121.0f);    
	if( all(equal(m43comp[0], m43expected[0])) && all(equal(m43comp[1], m43expected[1])) && all(equal(m43comp[2], m43expected[2])) )
        val += 1.0f;

    // outerProduct
    mat2 m2_1    = mat2(1.0f, 1.0f);
    mat2x3 m23_1 = mat2x3(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    mat2x4 m24_1 = mat2x4(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    mat3 m3_1    = mat3(1.0f, 1.0f, 1.0f);
	mat3x2 m32_1 = mat3x2(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	mat3x4 m34_1 = mat3x4(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    mat4 m4_1    = mat4(1.0f, 1.0f, 1.0f, 1.0f);
    mat4x2 m42_1 = mat4x2(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    mat4x3 m43_1 = mat4x3(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    m2 = outerProduct(vec2(1.0f, 1.0f), vec2(1.0f, 1.0f));
    if (all(equal(m2[0], m2_1[0])) && all(equal(m2[1], m2_1[1])))
        val += 1.0f;

    m3 = outerProduct(vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
	if (all(equal(m3[0], m3_1[0])) && all(equal(m3[1], m3_1[1])) && all(equal(m3[2], m3_1[2])))
        val += 1.0f;

    m4 = outerProduct(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (all(equal(m4[0], m4_1[0])) && all(equal(m4[1], m4_1[1])) && all(equal(m4[2], m4_1[2])) && all(equal(m4[3], m4_1[3])))
        val += 1.0f;

    m23 = outerProduct(vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 1.0f));
	if (all(equal(m23[0], m23_1[0])) && all(equal(m23[1], m23_1[1])) && all(equal(m23[2], m23_1[2])))
        val += 1.0f;

    m32 = outerProduct(vec2(1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
	if (all(equal(m32[0], m32_1[0])) && all(equal(m32[1], m32_1[1])))
        val += 1.0f;

    m24 = outerProduct(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec2(1.0f, 1.0f));
	if (all(equal(m24[0], m24_1[0])) && all(equal(m24[1], m24_1[1])) && all(equal(m24[2], m24_1[2])) && all(equal(m24[3], m24_1[3])))
        val += 1.0f;

    m42 = outerProduct(vec2(1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (all(equal(m42[0], m42_1[0])) && all(equal(m42[1], m42_1[1])))
        val += 1.0f;

    m34 = outerProduct(vec4(1.0f, 1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
	if (all(equal(m34[0], m34_1[0])) && all(equal(m34[1], m34_1[1])) && all(equal(m34[2], m34_1[2])) && all(equal(m34[3], m34_1[3])))
        val += 1.0f;

    m43 = outerProduct(vec3(1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (all(equal(m43[0], m43_1[0])) && all(equal(m43[1], m43_1[1])) && all(equal(m43[2], m43_1[2])))
        val += 1.0f;

	// Transpose
    m2 = transpose(mat2(0.0f, 1.0f, 2.0f, 3.0f));
    m2expected = mat2(0.0f, 2.0f, 1.0f, 3.0f);
    if (all(equal(m2[0], m2expected[0])) && all(equal(m2[1], m2expected[1])))
        val += 1.0f;

    m3 = transpose(mat3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f));
    m3expected = mat3(0.0f, 3.0f, 6.0f, 1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f);
    if (all(equal(m3[0], m3expected[0])) && all(equal(m3[1], m3expected[1])) && all(equal(m3[2], m3expected[2])))
        val += 1.0f;

    m4 = transpose(mat4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f));
	m4expected = mat4(0.0f, 4.0f, 8.0f, 12.0f, 1.0f, 5.0f, 9.0f, 13.0f, 2.0f, 6.0f, 10.0f, 14.0f, 3.0f, 7.0f, 11.0f, 15.0f);
    if (all(equal(m4[0], m4expected[0])) && all(equal(m4[1], m4expected[1])) && all(equal(m4[2], m4expected[2])) && all(equal(m4[3], m4expected[3])))
        val += 1.0f;

	// Does not work as expected
#if 0
    m23 = transpose(mat3x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f));
    m23expected = mat2x3(0.0f, 2.0f, 4.0f, 1.0f, 3.0f, 5.0f);
	if (all(equal(m23[0], m23expected[0])) && all(equal(m23[1], m23expected[1])) && all(equal(m23[2], m23expected[2])))
        val += 1.0f;
#endif

	// Does not work as expected
#if 0
	m32 = transpose(mat2x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f));
    m32expected = mat3x2(0.0f, 3.0f, 1.0f, 4.0f, 2.0f, 5.0f);
	if (all(equal(m32[0], m32expected[0])) && all(equal(m32[1], m32expected[1])))
        val += 1.0f;
#endif

// Does not work as expected
#if 0
    m24 = transpose(mat4x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f));
    m24expected = mat2x4(0.0f, 2.0f, 4.0f, 6.0f, 1.0f, 3.0f, 5.0f, 7.0f);
	if (all(equal(m24[0], m24expected[0])) && all(equal(m24[1], m24expected[1])) && all(equal(m24[2], m24expected[2])) && all(equal(m24[3], m24expected[3])))
        val += 1.0f;
#endif

	// Does not work as expected
#if 0
    m42 = transpose(mat2x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f));
    m42expected = mat4x2(0.0f, 4.0f, 1.0f, 5.0f, 2.0f, 6.0f, 3.0f, 7.0f);
	if (all(equal(m42[0], m42expected[0])) && all(equal(m42[1], m42expected[1])))
        val += 1.0f;
#endif

	// Does not work as expected
#if 0
    m34 = transpose(mat4x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f));
    m34expected = mat3x4(0.0f, 3.0f, 6.0f, 9.0f, 1.0f, 4.0f, 7.0f, 10.0f, 2.0f, 5.0f, 8.0f, 11.0f);
	if (all(equal(m34[0], m34expected[0])) && all(equal(m34[1], m34expected[1])) && all(equal(m34[2], m34expected[2])) && all(equal(m34[3], m34expected[3])))
        val += 1.0f;
#endif

	// Does not work as expected
#if 0
    m43 = transpose(mat3x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f));
    m43expected = mat4x3(0.0f, 4.0f, 8.0f, 1.0f, 5.0f, 9.0f, 2.0f, 6.0f, 10.0f, 3.0f, 7.0f, 11.0f);
	if (all(equal(m43[0], m43expected[0])) && all(equal(m43[1], m43expected[1])) && all(equal(m43[2], m43expected[2])))
        val += 1.0f;
#endif

	// Determinant
    float fdet = determinant(mat2(3.0f, 8.0f, 4.0f, 6.0f));
    if (fdet == -14)
        val += 1.0f;

    fdet = determinant(mat3(6.0f, 1.0f, 1.0f, 4.0f, -2.0f, 5.0f, 2.0f, 8.0f, 7.0f));
	if (fdet == -306)
        val += 1.0f;

    fdet = determinant(mat4(1.0f, 2.0f, 3.0f, 4.0f, 9.0f, 2.0f, 7.0f, 6.0f, 5.0f, 6.0f, 9.0f, 8.0f, 1.0f, 3.0f, 2.0f, 1.0f));
	if (fdet == 84)
        val += 1.0f;

	// Revisit when inverse works
#if 0
    mat2 i2 = mat2(1.0f);
    mat2 m2x2 = mat2(vec2(1.0f, 1.0f), vec2(0.0f, 1.0f));
    m2 = inverse(m2x2);
    mat2 m2_i = m2x2 * m2;
    if (all(equal(m2_i[0], i2[0])) && all(equal(m2_i[1], i2[1])))
        val += 1.0f;

    m3 = inverse(mat3 m);

    m4 = inverse(mat4 m);
#endif

    return val;
}

float VectorRelational()
{
    float val = 0.0f;

    vec2 v2_1 = vec2(1.0f, 2.0f);
    vec2 v2_2 = vec2(3.0f, 4.0f);
	vec3 v3_1 = vec3(1.0f, 2.0f, 3.0f);
    vec3 v3_2 = vec3(3.0f, 4.0f, 5.0f);
	vec4 v4_1 = vec4(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 v4_2 = vec4(3.0f, 4.0f, 5.0f, 6.0f);

	dvec2 d2_1 = dvec2(1.0, 2.0);
    dvec2 d2_2 = dvec2(3.0, 4.0);
	dvec3 d3_1 = dvec3(1.0, 2.0, 3.0);
    dvec3 d3_2 = dvec3(3.0, 4.0, 5.0);
	dvec4 d4_1 = dvec4(1.0, 2.0, 3.0, 4.0);
    dvec4 d4_2 = dvec4(3.0, 4.0, 5.0, 6.0);

	bvec2 bv2_1 = bvec2(true, false);
    bvec2 bv2_2 = bvec2(false, true);
	bvec3 bv3_1 = bvec3(true, false, true);
    bvec3 bv3_2 = bvec3(false, true, false);
	bvec4 bv4_1 = bvec4(true, false, true, false);
    bvec4 bv4_2 = bvec4(false, true, false, true);

	ivec2 iv2_1 = ivec2(1, 2);
    ivec2 iv2_2 = ivec2(3, 4);
	ivec3 iv3_1 = ivec3(1, 2, 3);
    ivec3 iv3_2 = ivec3(3, 4, 5);
	ivec4 iv4_1 = ivec4(1, 2, 3, 4);
    ivec4 iv4_2 = ivec4(3, 4, 5, 6);

	uvec2 uv2_1 = uvec2(1, 2);
    uvec2 uv2_2 = uvec2(3, 4);
	uvec3 uv3_1 = uvec3(1, 2, 3);
    uvec3 uv3_2 = uvec3(3, 4, 5);
	uvec4 uv4_1 = uvec4(1, 2, 3, 4);
    uvec4 uv4_2 = uvec4(3, 4, 5, 6);

    bvec2 bv2res;
    bvec3 bv3res;
    bvec4 bv4res;

	// Floats
    bv2res = lessThan(v2_1, v2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThan(v3_1, v3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThan(v4_1, v4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = lessThanEqual(v2_1, v2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThanEqual(v3_1, v3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThanEqual(v4_1, v4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = greaterThan(v2_1, v2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThan(v3_1, v3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThan(v4_1, v4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = greaterThanEqual(v2_1, v2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThanEqual(v3_1, v3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThanEqual(v4_1, v4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = equal(v2_1, v2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = equal(v3_1, v3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = equal(v4_1, v4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = notEqual(v2_1, v2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = notEqual(v3_1, v3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = notEqual(v4_1, v4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	// Doubles
	bv2res = lessThan(d2_1, d2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThan(d3_1, d3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThan(d4_1, d4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = lessThanEqual(d2_1, d2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThanEqual(d3_1, d3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThanEqual(d4_1, d4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = greaterThan(d2_1, d2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThan(d3_1, d3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThan(d4_1, d4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = greaterThanEqual(d2_1, d2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThanEqual(d3_1, d3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThanEqual(d4_1, d4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = equal(d2_1, d2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = equal(d3_1, d3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = equal(d4_1, d4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = notEqual(d2_1, d2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = notEqual(d3_1, d3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = notEqual(d4_1, d4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	// Bools
	bv2res = lessThan(bv2_1, bv2_2);
    if (bv2res == bvec2(false,true))
        val += 1.0f;
    bv3res = lessThan(bv3_1, bv3_2);
	if (bv3res == bvec3(false,true,false))
        val += 1.0f;
    bv4res = lessThan(bv4_1, bv4_2);
	if (bv4res == bvec4(false, true, false, true))
        val += 1.0f;

	bv2res = lessThanEqual(bv2_1, bv2_2);
    if (bv2res == bvec2(false,true))
        val += 1.0f;
    bv3res = lessThanEqual(bv3_1, bv3_2);
	if (bv3res == bvec3(false,true,false))
        val += 1.0f;
    bv4res = lessThanEqual(bv4_1, bv4_2);
	if (bv4res == bvec4(false, true,false,true))
        val += 1.0f;

	bv2res = greaterThan(bv2_1, bv2_2);
    if (bv2res == bvec2(true,false))
        val += 1.0f;
    bv3res = greaterThan(bv3_1, bv3_2);
	if (bv3res == bvec3(true,false,true))
        val += 1.0f;
    bv4res = greaterThan(bv4_1, bv4_2);
	if (bv4res == bvec4(true, false, true, false))
        val += 1.0f;

	bv2res = greaterThanEqual(bv2_1, bv2_2);
    if (bv2res == bvec2(true,false))
        val += 1.0f;
    bv3res = greaterThanEqual(bv3_1, bv3_2);
	if (bv3res == bvec3(true,false,true))
        val += 1.0f;
    bv4res = greaterThanEqual(bv4_1, bv4_2);
	if (bv4res == bvec4(true, false, true, false))
        val += 1.0f;

	bv2res = equal(bv2_1, bv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = equal(bv3_1, bv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = equal(bv4_1, bv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = notEqual(bv2_1, bv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = notEqual(bv3_1, bv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = notEqual(bv4_1, bv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

    if (any(bv2_1))
        val += 1.0f;
    if (any(bv3_1))
        val += 1.0f;
	if (any(bv4_1))
        val += 1.0f;

	if (!all(bv2_1))
        val += 1.0f;
    if (!all(bv3_1))
        val += 1.0f;
	if (!all(bv4_1))
        val += 1.0f;

    bv2res = not(bv2_1);
    if (bv2res == bvec2(false, true))
        val += 1.0f;
	bv3res = not(bv3_1);
    if (bv3res == bvec3(false, true, false))
        val += 1.0f;
	bv4res = not(bv4_1);
    if (bv4res == bvec4(false, true, false, true))
        val += 1.0f;

	// Int
	bv2res = lessThan(iv2_1, iv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThan(iv3_1, iv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThan(iv4_1, iv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = lessThanEqual(iv2_1, iv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThanEqual(iv3_1, iv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThanEqual(iv4_1, iv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = greaterThan(iv2_1, iv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThan(iv3_1, iv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThan(iv4_1, iv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = greaterThanEqual(iv2_1, iv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThanEqual(iv3_1, iv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThanEqual(iv4_1, iv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = equal(iv2_1, iv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = equal(iv3_1, iv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = equal(iv4_1, iv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = notEqual(iv2_1, iv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = notEqual(iv3_1, iv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = notEqual(iv4_1, iv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	// Uint
	bv2res = lessThan(uv2_1, uv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThan(uv3_1, uv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThan(uv4_1, uv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = lessThanEqual(uv2_1, uv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = lessThanEqual(uv3_1, uv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = lessThanEqual(uv4_1, uv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

	bv2res = greaterThan(uv2_1, uv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThan(uv3_1, uv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThan(uv4_1, uv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = greaterThanEqual(uv2_1, uv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = greaterThanEqual(uv3_1, uv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = greaterThanEqual(uv4_1, uv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = equal(uv2_1, uv2_2);
    if (bv2res == bvec2(false,false))
        val += 1.0f;
    bv3res = equal(uv3_1, uv3_2);
	if (bv3res == bvec3(false,false,false))
        val += 1.0f;
    bv4res = equal(uv4_1, uv4_2);
	if (bv4res == bvec4(false, false,false,false))
        val += 1.0f;

	bv2res = notEqual(uv2_1, uv2_2);
    if (bv2res == bvec2(true,true))
        val += 1.0f;
    bv3res = notEqual(uv3_1, uv3_2);
	if (bv3res == bvec3(true,true,true))
        val += 1.0f;
    bv4res = notEqual(uv4_1, uv4_2);
	if (bv4res == bvec4(true, true,true,true))
        val += 1.0f;

    return val;
}

float Integer()
{
    float val = 0.0f;

    int ibits = 0;
    uint uibits = 0;

    uint carry = 0;
    uaddCarry(uint(0xF), uint(0xFFFFFFFF), carry);
    if (carry == 1)
        val += 1.0f;

    uint borrow = 0;
	usubBorrow(16, 17, borrow);
    if (borrow == 1)
        val += 1.0f;

    uint umsb = 0;
    uint ulsb = 0;
    umulExtended(uint(2), uint(3), umsb, ulsb);
    if (umsb == 0 && ulsb == 6)
        val += 1.0f;

	int msb = 0;
    int lsb = 0;
    umulExtended(int(2), int(3), msb, lsb);
    if (msb == 0 && lsb == 6)
        val += 1.0f;

    ibits = bitfieldExtract(int(0xFFFFFFFF), int(0), int(32));
    if (ibits == int(0xFFFFFFFF))
        val += 1.0f;

	uibits = bitfieldExtract(uint(0xFFFFFFFF), int(0), int(32));
    if (uibits == uint(0xFFFFFFFF))
        val += 1.0f;

	ibits = bitfieldInsert(int(0x00000000), int(0xFFFFFFFF), int(0), int(32));
    if (ibits == int(0xFFFFFFFF))
        val += 1.0f;

    uibits = bitfieldInsert(uint(0x00000000), uint(0xFFFFFFFF), int(0), int(32));
    if (uibits == uint(0xFFFFFFFF))
        val += 1.0f;

	/*
		0xABC      = 101010111100
		0x3D500000 = 001111010101
	*/
    ibits = bitfieldReverse(int(0xABC));
    if (ibits == int(0x3D500000))
        val += 1.0f;

	uibits = bitfieldReverse(uint(0xABC));
    if (uibits == uint(0x3D500000))
        val += 1.0f;

	ibits = bitCount(int(0xABC));
    if (ibits == 7)
        val += 1.0f;

	uibits = bitCount(uint(0xABC));
    if (uibits == 7)
        val += 1.0f;

    ibits = findLSB(int(0xABC));
    if (ibits == 2)
        val += 1.0f;

    ibits = findLSB(uint(0xABC));
	if (ibits == 2)
        val += 1.0f;

	ibits = findMSB(int(0xABC));
    if (ibits == 11)
        val += 1.0f;

    ibits = findMSB(uint(0xABC));
	if (ibits == 11)
        val += 1.0f;

    return val;
}

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    // 8.1. Angle and Trigonometry Functions
    outputBuffer[TRIG_FUNCTIONS] = AngleAndTrig();
    
    // 8.2. Exponential Functions
    outputBuffer[EXP_FUNCTIONS] = Exponential();

    // 8.3. Common Functions
    outputBuffer[COMMON_FUNCTIONS] = Common();

    // 8.4. Floating-Point Pack and Unpack Functions
     outputBuffer[PACK_FUNCTIONS] = Pack();

    // 8.5. Geometric Functions
    outputBuffer[GEOMETRIC_FUNCTIONS] = Geometric();

    // 8.6. Matrix Functions
    outputBuffer[MATRIX_FUNCTIONS] = Matrix();

    // 8.7. Vector Relational Functions
     outputBuffer[VEC_FUNCTIONS] = VectorRelational();

    // 8.8. Integer Functions
    outputBuffer[INT_FUNCTIONS] = Integer();
}

// DX11: 44.0
// DX11: 36.0
// DX11: 291.0
// DX11: 3.0
// DX11: 58.0
// DX11: 24.0
// DX11: 99.0
// DX11: 16.0

// DX12: 44.0
// DX12: 36.0
// DX12: 293.0
// DX12: 3.0
// DX12: 56.0
// DX12: 24.0
// DX12: 99.0
// DX12: 16.0

// VK: 44.0
// VK: 36.0
// VK: 293.0
// VK: 3.0
// VK: 58.0
// VK: 24.0
// VK: 99.0
// VK: 16.0
