//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Disabled: partial writes currently count as whole-variable stores, so these
// reads of unwritten components do not produce the expected diagnostics.
// Enable when initialization analysis tracks which parts a store covers.

struct Pair { float x; float y; };
struct Outer { Pair inner; Pair items[4]; float2 uv; float3x3 matrix; };

// Check that reading an unwritten vector component is diagnosed.
float4 missingAlpha(bool condition)
{
    float4 value;
    if (condition) value.rgb = float3(1, 2, 3);
    if (condition)
        return value;
//CHECK:^ E41035
//CHECK:^ variable 'value' may be uninitialized on some paths
    return float4(0);
}

// Check that reading a different array element is diagnosed.
float differentElements(bool condition)
{
    float values[2];
    if (condition) values[0] = 1;
    if (condition)
        return values[1];
//CHECK:^ E41035
//CHECK:^ variable 'values' may be uninitialized on some paths
    return 0;
}

// Check that reading a different nested field is diagnosed.
float differentFields(bool condition)
{
    Outer value;
    if (condition) value.inner.x = 1;
    if (condition)
        return value.inner.y;
//CHECK:^ E41035
//CHECK:^ variable 'value' may be uninitialized on some paths
    return 0;
}

// Check that reading an unwritten dynamically indexed field is diagnosed.
float dynamicElements(bool condition, int written, int read)
{
    Outer value;
    if (condition) value.items[written].x = 1;
    if (condition)
        return value.items[read].y;
//CHECK:^ E41035
//CHECK:^ variable 'value' may be uninitialized on some paths
    return 0;
}

// Check that a nested vector read after one component write is diagnosed.
float2 nestedVector(bool condition)
{
    Outer value;
    if (condition) value.uv.x = 1;
    if (condition)
        return value.uv;
//CHECK:^ E41035
//CHECK:^ variable 'value' may be uninitialized on some paths
    return float2(0);
}

// Check that a matrix row read after one element write is diagnosed.
float3 matrixRow(bool condition)
{
    Outer value;
    if (condition) value.matrix[1][2] = 1;
    if (condition)
        return value.matrix[1];
//CHECK:^ E41035
//CHECK:^ variable 'value' may be uninitialized on some paths
    return float3(0);
}

[shader("compute")]
void main() {}
