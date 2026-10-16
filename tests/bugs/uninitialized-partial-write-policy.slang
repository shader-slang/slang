//TEST:SIMPLE: -target spirv -no-codegen -warnings-as-errors all

struct Pair { float x; float y; };
struct Outer { Pair inner; Pair items[4]; float2 uv; float3x3 matrix; };

// Check that reading the written field under the same guard stays silent.
float sameField(bool condition)
{
    Pair value;
    if (condition) value.x = 1;
    if (condition) return value.x;
    return 0;
}

// Check that whole-subobject writes cover a later leaf read.
float wholeThenLeaf(bool condition, Pair initialized)
{
    Outer value;
    if (condition) value.inner = initialized;
    if (condition) return value.inner.y;
    return 0;
}

// Check that reading only a written vector component stays silent.
float sameComponent(bool condition)
{
    float4 value;
    if (condition) value.x = 1;
    if (condition) return value.x;
    return 0;
}

// Check that separate component writes do not introduce a whole-vector warning.
float4 allComponents(bool condition)
{
    float4 value;
    if (condition)
    {
        value.rgb = float3(1, 2, 3);
        value.a = 1;
    }
    if (condition) return value;
    return float4(0);
}

[shader("compute")]
void main() {}
