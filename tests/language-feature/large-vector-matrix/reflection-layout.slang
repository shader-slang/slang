// Reflection reports large vectors and matrices with their real sizes, laid out as arrays:
// element by element for a vector, row after row for a matrix (row-major whatever the layout
// mode), with each row laid out as an array when it has more than 4 elements.

//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target spirv -no-codegen
//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target spirv -no-codegen -matrix-layout-column-major

// The structured buffer uses std430: no padding between elements or rows.
// CHECK: "name": "inputBuffer"
// CHECK: "name": "v"
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 20, "elementStride": 4}
// CHECK: "name": "tag"
// CHECK: "binding": {"kind": "uniform", "offset": 20, "size": 4
// CHECK: "name": "m"
// CHECK: "rowCount": 3
// CHECK: "columnCount": 6
// CHECK: "binding": {"kind": "uniform", "offset": 24, "size": 72
// CHECK: "name": "t"
// CHECK: "binding": {"kind": "uniform", "offset": 96, "size": 128
// CHECK: {"kind": "uniform", "value": 224, "alignment": 16}

// The constant buffer uses std140: every array element, so every vector element and every row
// element, starts on a 16-byte boundary.
// CHECK: "name": "constants"
// CHECK: "name": "v"
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 80, "elementStride": 16}
// CHECK: "name": "tag"
// CHECK: "binding": {"kind": "uniform", "offset": 80, "size": 4
// CHECK: "name": "m"
// CHECK: "binding": {"kind": "uniform", "offset": 96, "size": 288
// CHECK: "name": "t"
// CHECK: "binding": {"kind": "uniform", "offset": 384, "size": 128

struct Element
{
    vector<float, 5> v;
    float tag;
    matrix<float, 3, 6> m;
    matrix<float, 8, 3> t;
}

StructuredBuffer<Element> inputBuffer;
ConstantBuffer<Element> constants;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = inputBuffer[0].v[4] + constants.m[2][5] + constants.t[7].z;
}
