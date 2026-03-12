//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -allow-glsl -target spirv -entry computeMain

// Using groupshared variables
groupshared float4 gsUninitialized;

float use_undefined_gs()
{
    return gsUninitialized.x;
//CHK:                     ^ use of uninitialized global variable
//CHK:                     ^ use of uninitialized global variable 'gsUninitialized'
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
    return writtenNever;
/*CHK:
    ^^^^^^ use of uninitialized global variable
    ^^^^^^ use of uninitialized global variable 'writtenNever'
*/
}

// OK because of prior store
float use_later_writte()
{
    return writtenLater;
}

// Varying inputs never warn
layout(location = 0) in vec4 data;

vec4 glsl_layout_in_ok()
{
    return data;
}

// Layout outputs should still be written to at some point
layout(location = 1) out vec4 x;

vec4 glsl_layout_out_undefined()
{
    return x;
/*CHK:
    ^^^^^^ use of uninitialized global variable
    ^^^^^^ use of uninitialized global variable 'x'
*/
}

layout(location = 2) out vec4 y;

void glsl_layout_out_store(vec4 data)
{
    // Written to here...
    y = data;
}

vec4 glsl_layout_out_ok()
{
    // ...so read here is treated as OK
    return y;
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}
