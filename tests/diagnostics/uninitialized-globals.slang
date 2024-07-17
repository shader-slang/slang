//TEST:SIMPLE(filecheck=CHK): -target spirv

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
    //CHK-DAG: warning 41017: use of uninitialized global variable 'gsUndefined'
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
    //CHK-DAG: warning 41017: use of uninitialized global variable 'writtenNever'
    return writtenNever;
}

// OK because of prior store
float use_later_writte()
{
    return writtenLater;
}

//CHK-NOT: warning 41017
