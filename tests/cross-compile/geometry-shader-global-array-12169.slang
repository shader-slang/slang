// A GLSL-style global-scope geometry-shader input declared as an array of
// structs (`in triangle CoarseVertex coarseVertices[3];`) must legalize and
// compile for both SPIR-V and GLSL, and the indexed read of a stage-input
// element must flow through to the output.
// See https://github.com/shader-slang/slang/issues/12169

//TEST:SIMPLE(filecheck=SPIRV): -profile sm_5_0 -stage geometry -entry main -target spirv
//TEST:SIMPLE(filecheck=GLSL): -profile sm_5_0 -stage geometry -entry main -target glsl

struct CoarseVertex
{
    float4 position : POSITION;
}

struct RasterVertex
{
    float4 position : POSITION;
}

in triangle CoarseVertex coarseVertices[3];

// SPIRV: OpExecutionMode %main Triangles
// SPIRV-DAG: OpTypeArray %v4float %int_3
// SPIRV-DAG: %coarseVertices_position = OpVariable %{{.*}} Input
// SPIRV-DAG: %outputStream_position = OpVariable %{{.*}} Output
// The indexed read must access-chain into the input array, load that element,
// and feed it to the output. The capture chain follows the value end to end:
// input element -> v.position -> reload of v -> extract -> output store, so a
// broken data flow fails the test.
// SPIRV: %[[DST:[0-9]+]] = OpAccessChain %_ptr_Function_v4float %v %int_0
// SPIRV: %[[PTR:[0-9]+]] = OpAccessChain %_ptr_Input_v4float %coarseVertices_position %{{[0-9]+}}
// SPIRV: %[[VAL:[0-9]+]] = OpLoad %v4float %[[PTR]]
// SPIRV: OpStore %[[DST]] %[[VAL]]
// SPIRV: %[[RV:[0-9]+]] = OpLoad %RasterVertex %v
// SPIRV: %[[OUT:[0-9]+]] = OpCompositeExtract %v4float %[[RV]] 0
// SPIRV: OpStore %outputStream_position %[[OUT]]

// GLSL: in vec4 {{.*}}coarseVertices_position{{[0-9_]*}}[3];
// GLSL: layout(triangles) in;
// The value read from the indexed input element is the one written to the output.
// GLSL: [[V:[A-Za-z_0-9]+]].position{{[0-9_]*}} = {{.*}}coarseVertices_position{{[0-9_]*}}[ii{{[0-9_]*}}];
// GLSL: outputStream_position{{[0-9_]*}} = [[V]].position{{[0-9_]*}};

[shader("geometry")]
[maxvertexcount(3)]
void main(inout TriangleStream<RasterVertex> outputStream)
{
    for (int ii = 0; ii < 3; ++ii)
    {
        RasterVertex v;
        v.position = coarseVertices[ii].position;
        outputStream.Append(v);
    }
}
