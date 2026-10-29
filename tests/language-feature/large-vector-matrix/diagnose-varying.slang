// A vector or matrix larger than 4 cannot be a shader stage input or output.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry vertexMain -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry vertexMain -stage vertex

struct VertexOutput
{
    float4 position : SV_Position;
    vector<float, 8> weights : WEIGHTS;
}

VertexOutput vertexMain(float3 position : POSITION, vector<float, 6> extra : EXTRA)
//CHECK:     ^^^^^^^^^^ vector or matrix larger than 4 used as a shader stage input or output
//CHECK:     ^^^^^^^^^^ the result of 'vertexMain' contains a vector with more than 4 elements or a matrix with more than 4 rows or columns
//CHECK:                                                             ^^^^^ vector or matrix larger than 4 used as a shader stage input or output
//CHECK:                                                             ^^^^^ parameter 'extra' contains a vector with more than 4 elements or a matrix with more than 4 rows or columns
{
    VertexOutput output;
    output.position = float4(position, 1.0);
    output.weights = vector<float, 8>(extra[5]);
    return output;
}
