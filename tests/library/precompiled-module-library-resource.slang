// precompiled-module-library-resource.slang

// Compile this library source with -embed-dxil and -embed-spirv options.
// Tests that modules can be precompiled to dxil despite having resource
// parameters or return types.

//TEST(windows):COMPILE: tests/library/precompiled-module-library-resource.slang -o tests/library/precompiled-module-library-resource-dxil.slang-module -embed-dxil -profile lib_6_6 -incomplete-library
//TEST:COMPILE: tests/library/precompiled-module-library-resource.slang -o tests/library/precompiled-module-library-resource-spv.slang-module -embed-spirv -incomplete-library

module "precompiled-module-library-resource";

public struct ResourceStruct {
    public StructuredBuffer<int> buffer;
};

public int resource_in_parameter(StructuredBuffer<int> buffer)
{
    return buffer[0];
}

public int resource_in_struct_parameter(ResourceStruct rs)
{
    return rs.buffer[0];
}

internal float matrix_in_parameter_internal(float2x2 matrix)
{
    return matrix[0][0];
}

public float matrix_in_parameter_public(float a)
{
    float2x2 matrix = {a, .2, .3, .4};
    return matrix_in_parameter_internal(matrix);
}
