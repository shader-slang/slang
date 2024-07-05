//TEST:SIMPLE(filecheck=CHK): -target spirv

// Should not warn here (unconditionalBranch)
float3 unconditional(int mode)
{
    float f(float) { return 1; }

    float k0;
    float k1;

    if (mode == 1) {
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
    //CHK-DAG: error 41015: use of uninitialized value 'k'
    return (k > 0);
}

// Using unitialized values
int use_undefined_value(int k)
{
    int x;
    x += k;
    //CHK-DAG: error 41015: use of uninitialized value 'x'
    return x;
}

// Returning uninitialized values
__generic<T>
T generic_undefined_return()
{
    T x;
    //CHK-DAG: warning 41018: returning without initializing 'x'
    return x;
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

    __init() {
        // TODO: specialized warning for initialized
        // fields in the constructors
        field = 1;
    }
};

struct NoDefault {
    int f(int i) {
        return i;
    }
};

float4 main()
{
    EmptyStruct empty;
    NoDefault no_default;
    Data inputData = Data(1.0);

    float undefVar;
    Data undefData;
    NestedData nestedData;

    float result = inputData.value;

    //CHK-DAG: error 41015: use of uninitialized value 'undefVar'
    result += undefVar;

    //CHK-DAG: error 41015: use of uninitialized value 'undefData'
    result += undefData.value;

    //CHK-DAG: error 41015: use of uninitialized value 'nestedData'
    result += nestedData.data.value;

    return float4(result);
}
