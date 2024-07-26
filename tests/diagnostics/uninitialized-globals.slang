//TEST:SIMPLE(filecheck=CHK): -allow-glsl -target spirv

// Using groupshared variables
groupshared float4 gsConstexpr = float4(1.0f);
groupshared float4 gsUndefined;

// OK
float use_constexpr_initialized_gs()
{
    return gsConstexpr.x;
}

float use_undefined_gs()
{
    //CHK-DAG: ([[# @LINE + 1]]): warning 41017: use of uninitialized global variable 'gsUndefined'
    return gsUndefined.x;
}

// Using static variables
static const float cexprInitialized = 1.0f;
static float writtenNever;
static float writtenLater;

// OK
float use_initialized_static()
{
    return cexprInitialized;
}

// Should detect this and treat it as a store
void write_to_later()
{
    writtenLater = 1.0f;
}

float use_never_written()
{
    //CHK-DAG: ([[# @LINE + 1]]): warning 41017: use of uninitialized global variable 'writtenNever'
    return writtenNever;
}

// OK because of prior store
float use_later_writte()
{
    return writtenLater;
}

// Should not warn here (glsl layout decoration check)
layout(location = 0) in vec4 data;

vec4 glsl_layout_in_ok()
{
    return data;
}

// Invalid use of layout variables
layout(location = 1) out vec4 x;

vec4 glsl_layout_out_undefined()
{
    //CHK-DAG: ([[# @LINE + 1]]): warning 41017: use of uninitialized global variable 'x'
    return x;
}

// OK use of layout variables
layout(location = 2) out vec4 y;

void glsl_layout_out_store(vec4 data)
{
    y = data;
}

vec4 glsl_layout_out_ok()
{
    return y;
}

//CHK-NOT: warning 41017
