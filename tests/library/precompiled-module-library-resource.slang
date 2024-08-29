// precompiled-module-library-resource.slang

// Compile this library source with -embed-dxil option. Tests that modules can be
// precompiled to dxil despite having resource parameters or return types.

//TEST(windows):COMPILE: tests/library/precompiled-module-library-resource.slang -o tests/library/precompiled-module-library-resource.slang-module -embed-dxil -profile lib_6_6 -incomplete-library

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

internal int matrix_in_parameter_internal(int1x1 matrix)
{
    return matrix[0][0];
}

public int matrix_in_parameter_public(int a)
{
    int1x1 matrix = {a};
    return matrix_in_parameter_internal(matrix);
}
