// Regression for shader-slang/slang#13446: an integer matrix in the stage interface keeps the
// row-vector arrays GLSL has always used for it. Buffer element lowering wraps integer matrices
// in memory in a layout-aware storage struct, but a varying has no memory layout.

//TEST:SIMPLE(filecheck=VERT):-target glsl -entry vertMain -stage vertex -matrix-layout-column-major
//TEST:SIMPLE(filecheck=FRAG):-target glsl -entry fragMain -stage fragment -matrix-layout-column-major

struct VertexOutput
{
    float4 position : SV_Position;
    nointerpolation int2x3 m : MATRIX;
}

[shader("vertex")]
VertexOutput vertMain(uint id : SV_VertexID)
{
    VertexOutput output;
    output.position = float4(0, 0, 0, 1);
    output.m = int2x3(int(id), 1, 2, 3, 4, 5);
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput input) : SV_Target
{
    return float4(input.m[1][2], 0, 0, 1);
}

// VERT: out ivec3 {{.*}}m{{.*}}[2];
// VERT-NOT: _MatrixStorage

// FRAG: in ivec3 {{.*}}m{{.*}}[2];
// FRAG-NOT: _MatrixStorage
