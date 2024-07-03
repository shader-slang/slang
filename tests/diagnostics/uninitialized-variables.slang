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
    //CHK: error 41015: use of uninitialized value 'k'
    return (k > 0);
}

// Using unitialized values
int use_undefined_value(int k)
{
    int x;
    x += k;
    //CHK: error 41015: use of uninitialized value 'x'
    return x;
}

// Returning uninitialized values
__generic<T>
T generic_undefined_return()
{
    T x;
    //CHK: warning 41018: returning without initializing 'x'
    return x;
}

// No warnings here, even thought autodiff generates
// IR which frequently returns undefined values
struct DiffStruct : IDifferentiable
{
    Texture2D tex;
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

// Empty structures should not generate diagnostics
// for empty default constructors
struct EmptyStruct
{
    __init() {}
};

float4 main()
{
    Data inputData = Data(1.0);

    float undefVar;
    Data undefData;
    NestedData nestedData;

    float result = inputData.value;

    //CHK: error 41015: use of uninitialized value 'undefVar'
    result += undefVar;

    //CHK: error 41015: use of uninitialized value 'undefData'
    result += undefData.value;

    //CHK: error 41015: use of uninitialized value 'nestedData'
    result += nestedData.data.value;

    return float4(result);
}
