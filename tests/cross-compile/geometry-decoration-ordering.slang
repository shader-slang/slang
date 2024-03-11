//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry main -stage geometry

// Make sure OutputTriangleStrip always appears after OutputVertices,
// otherwise, the shader won't work correctly.

// CHECK: OpExecutionMode {{.*}} OutputVertices 4
// CHECK: OpExecutionMode {{.*}} OutputTriangleStrip

[maxvertexcount(4)]
void main( point float4 input[1], inout TriangleStream<float4> OutputStream )
{
    OutputStream.Append(float4(0));
    OutputStream.Append(float4(1));
    OutputStream.Append(float4(2));
    OutputStream.Append(float4(3));
}