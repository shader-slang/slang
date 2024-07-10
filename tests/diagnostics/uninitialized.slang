//TEST:SIMPLE(filecheck=CHK): -target spirv

// TODO:
// * warn potentially uninitialized variables (control flow)
// * warn partially uninitialized variables (structs, arrays, etc.)
// * warn uninitialized fields in constructors

///////////////////////////////////
// Uninitialized local variables //
///////////////////////////////////

// Should not warn here (unconditionalBranch)
float3 unconditional(int mode)
{
    float f(float) { return 1; }

    float k0;
    float k1;

    if (mode == 1)
    {
        k1 = 1;
        k0 = 1;

        const float w = k1 * f(1);
        k0 = 4.0f * k0 * w;
        k1 = 2.0f * k1 * w;
    }

    return k0 + k1;
}

// Warn here for branches using the variables
int conditional()
{
    int k;
    //CHK-DAG: warning 41016: use of uninitialized variable 'k'
    return (k > 0);
}

// Using unitialized values
int use_undefined_value(int k)
{
    int x;
    x += k;
    //CHK-DAG: warning 41016: use of uninitialized variable 'x'
    return x;
}

// Returning uninitialized values
__generic<T>
T generic_undefined_return()
{
    T x;
    //CHK-DAG: warning 41016: use of uninitialized variable 'x'
    return x;
}

// Array variables
float undefined_array()
{
    float array[2];
    //CHK-DAG: warning 41016: use of uninitialized variable 'array'
    return array[0];
}

float filled_array(int mode)
{
    float array[2];
    array[0] = 1.0f;
    return array[0];
}

// Structs and nested structs
struct Data
{
    float value;
};

struct NestedData
{
    Data data;
};

// No warnings here, even thought autodiff generates
// IR which frequently returns undefined values
struct DiffStruct : IDifferentiable
{
    Data data;
    float x;
}

// Same story here
[ForwardDifferentiable]
DiffStruct differentiable(float x)
{
    DiffStruct ds;
    ds.x = x;
    return ds;
}

// Empty structures should not generate diagnostics
// for empty default constructors
struct EmptyStruct
{
    __init() {}
};

// No warnings for empty structs even without __init()
struct NonEmptyStruct
{
    int field;

    __init()
    {
        field = 1;
    }
};

// No warnings even when __init() is not specified
struct NoDefault
{
    int f(int i)
    {
        return i;
    }
};

// Constructing the above structs
int constructors()
{
    EmptyStruct empty;
    NoDefault no_default;
    return no_default.f(1);
}

// Using struct fields and nested structs
float structs()
{
    Data inputData = Data(1.0);

    float undefVar;
    Data undefData;
    NestedData nestedData;

    float result = inputData.value;

    //CHK-DAG: warning 41016: use of uninitialized variable 'undefVar'
    result += undefVar;

    //CHK-DAG: warning 41016: use of uninitialized variable 'undefData'
    result += undefData.value;

    //CHK-DAG: warning 41016: use of uninitialized variable 'nestedData'
    result += nestedData.data.value;

    return result;
}

////////////////////////////////////
// Uninitialized global variables //
////////////////////////////////////

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

//////////////////////////////////
// Uninitialized out parameters //
//////////////////////////////////

// Using before assigning
float regular_undefined_use(out float3 v)
{
    //CHK-DAG: warning 41015: use of uninitialized out parameter 'v'
    float r = v.x + 1.0;
    
    //CHK-DAG: warning 41018: returning without initializing out parameter 'v'
    return r;
}

// Returning before assigning
float returning_undefined_use(out float x)
{
    //CHK-DAG: warning 41018: returning without initializing out parameter 'x'
    return 0;
}

// Implicit, still returning before assigning
void implicit_undefined_use(out float x) 
{
    //CHK-DAG: warning 41018: returning without initializing out parameter 'x'
}

// Warn on potential return paths
void control_flow_undefined(bool b, out float y)
{
    if(b)
    {
        //CHK-DAG: warning 41018: returning without initializing out parameter 'y'
        return;
    }
    y = 0;
    return;
}

// No warnings if all paths are fine
void control_flow_defined(bool b, out float y)
{
    if(b)
    {
        unused(y);
        return;
    }
    y = 0;
    return;
}

//CHK-NOT: warning 41015
//CHK-NOT: warning 41016
//CHK-NOT: warning 41017
//CHK-NOT: warning 41018
