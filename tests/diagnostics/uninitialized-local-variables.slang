//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

float f(float) { return 1; }

// Should not warn here (unconditionalBranch)
float3 unconditional(int mode)
{
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
    return (k > 0);
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'k'
}

// Using unitialized values
int use_undefined_value(int k)
{
    int x;
    x += k;
//CHK:^^ use of uninitialized variable
//CHK:^^ use of uninitialized variable 'x'
    return x;
}

// We don't know the exact type of T yet.
// T may not have any members, and it may not need any initialization.
__generic<T>
T generic_undefined_return()
{
    T y;
    return y;
}

// Array variables
float undefined_array()
{
    float array[2];
    return array[0];
/*CHK:
    ^^^^^^ use of uninitialized variable
    ^^^^^^ use of uninitialized variable 'array'
*/
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

    result += undefVar;
//CHK:     ^^ use of uninitialized variable
//CHK:     ^^ use of uninitialized variable 'undefVar'

    result += undefData.value;
//CHK:     ^^ use of uninitialized variable
//CHK:     ^^ use of uninitialized variable 'undefData'

    result += nestedData.data.value;
//CHK:     ^^ use of uninitialized variable
//CHK:     ^^ use of uninitialized variable 'nestedData'

    return result;
}

// Warnings even in nested scopes
float nested_scopes(int x, inout float p)
{
    if (x == 0)
    {
        float y;
        return y;
//CHK:  ^^^^^^ use of uninitialized variable
//CHK:  ^^^^^^ use of uninitialized variable 'y'
    }
    else if (x == 1)
    {
        float y;
        p = y + 1;
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'y'

        if (x == 2)
        {
            float z;
            p += z;
//CHK:        ^^ use of uninitialized variable
//CHK:        ^^ use of uninitialized variable 'z'
        }
    }

    return 1.0;
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}
