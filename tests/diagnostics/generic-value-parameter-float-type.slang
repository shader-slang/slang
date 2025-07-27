//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that generic value parameters with float types are properly rejected

// CHECK: error 30624: generic value parameter type 'float' is not supported
struct ColorDefiner<let TMult : float>
{
    float4 color = {0.3, 0.7, 0.55, 1.0};

    float4 get_color() { return color * TMult; }
}

// CHECK: error 30624: generic value parameter type 'double' is not supported
struct DoubleDefiner<let TMult : double>
{
    float4 color = {0.3, 0.7, 0.55, 1.0};

    float4 get_color() { return color * float(TMult); }
}

// Integer types should work fine (no error expected)
struct IntDefiner<let TMult : int>
{
    float4 color = {0.3, 0.7, 0.55, 1.0};

    float4 get_color() { return color * TMult; }
}

// Enum types should work fine (no error expected)
enum Color { Red = 1, Green = 2, Blue = 3 }

struct EnumDefiner<let TMult : Color>
{
    float4 color = {0.3, 0.7, 0.55, 1.0};

    float4 get_color() { return color * int(TMult); }
}

void computeMain()
{
}