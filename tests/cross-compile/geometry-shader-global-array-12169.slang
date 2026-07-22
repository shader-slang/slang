// GLSL-style global-scope geometry-shader varying inputs must legalize and
// compile for SPIR-V and GLSL, with each input value flowing through to the
// matching output. Three shapes are covered because legalization routes them by
// scalarized flavor: an array of multi-field struct and a standalone multi-field
// struct both scalarize to a `tuple` (rewritten field-by-field, one Input
// variable per field), while an array of vector scalarizes to an `address`.
// See https://github.com/shader-slang/slang/issues/12169

//TEST:SIMPLE(filecheck=ARR_SPIRV): -profile sm_5_0 -stage geometry -entry mainArrayStruct -target spirv
//TEST:SIMPLE(filecheck=ARR_GLSL): -profile sm_5_0 -stage geometry -entry mainArrayStruct -target glsl
//TEST:SIMPLE(filecheck=VEC_SPIRV): -profile sm_5_0 -stage geometry -entry mainVecArray -target spirv
//TEST:SIMPLE(filecheck=VEC_GLSL): -profile sm_5_0 -stage geometry -entry mainVecArray -target glsl
//TEST:SIMPLE(filecheck=STANDALONE_SPIRV): -profile sm_5_0 -stage geometry -entry mainPlainStruct -target spirv
//TEST:SIMPLE(filecheck=STANDALONE_GLSL): -profile sm_5_0 -stage geometry -entry mainPlainStruct -target glsl

struct CoarseVertex
{
    float4 position : POSITION;
    float3 color : COLOR;
}

struct RasterVertex
{
    float4 position : POSITION;
    float3 color : COLOR;
}

in triangle CoarseVertex coarseArray[3];
in triangle float4 coarseVecArray[3];
in triangle CoarseVertex coarseOne;

// A multi-field struct is SOA-ized into one Input variable per field, so a
// wrong-field-selection or SOA-reassembly defect surfaces here where a
// single-field struct would not. Each field's captured chain runs input element
// -> local `v` field -> reload of `v` -> composite-extract -> matching output.

// ARR_SPIRV: OpExecutionMode %mainArrayStruct Triangles
// ARR_SPIRV-DAG: %coarseArray_position = OpVariable %{{.*}} Input
// ARR_SPIRV-DAG: %coarseArray_color = OpVariable %{{.*}} Input
// ARR_SPIRV: %[[VPOS:[0-9]+]] = OpAccessChain %_ptr_Function_v4float %v %int_0
// ARR_SPIRV: %[[PP:[0-9]+]] = OpAccessChain %_ptr_Input_v4float %coarseArray_position %{{[0-9]+}}
// ARR_SPIRV: %[[CP:[0-9]+]] = OpAccessChain %_ptr_Input_v3float %coarseArray_color %{{[0-9]+}}
// ARR_SPIRV: %[[PV:[0-9]+]] = OpLoad %v4float %[[PP]]
// ARR_SPIRV: OpStore %[[VPOS]] %[[PV]]
// ARR_SPIRV: %[[VCOL:[0-9]+]] = OpAccessChain %_ptr_Function_v3float %v %int_1
// ARR_SPIRV: %[[CV:[0-9]+]] = OpLoad %v3float %[[CP]]
// ARR_SPIRV: OpStore %[[VCOL]] %[[CV]]
// ARR_SPIRV: %[[RV:[0-9]+]] = OpLoad %RasterVertex %v
// ARR_SPIRV: %[[OPOS:[0-9]+]] = OpCompositeExtract %v4float %[[RV]] 0
// ARR_SPIRV: OpStore %outputStream_position %[[OPOS]]
// ARR_SPIRV: %[[OCOL:[0-9]+]] = OpCompositeExtract %v3float %[[RV]] 1
// ARR_SPIRV: OpStore %outputStream_color %[[OCOL]]

// ARR_GLSL-DAG: in vec4 {{.*}}coarseArray_position{{[0-9_]*}}[3];
// ARR_GLSL-DAG: in vec3 {{.*}}coarseArray_color{{[0-9_]*}}[3];
// `v` receives both fields from the indexed input; the output reads position
// from `v` and color from the whole-struct copy `[[S]] = v`, so both fields are
// bound from input element to their matching output.
// ARR_GLSL: [[V:v[0-9_]+]].position{{[0-9_]*}} = {{.*}}coarseArray_position{{[0-9_]*}}[ii{{[0-9_]*}}];
// ARR_GLSL: [[V]].color{{[0-9_]*}} = {{.*}}coarseArray_color{{[0-9_]*}}[ii{{[0-9_]*}}];
// ARR_GLSL: [[S:[A-Za-z_0-9]+]] = [[V]];
// ARR_GLSL: outputStream_position{{[0-9_]*}} = [[V]].position{{[0-9_]*}};
// ARR_GLSL: outputStream_color{{[0-9_]*}} = [[S]].color{{[0-9_]*}};

[shader("geometry")]
[maxvertexcount(3)]
void mainArrayStruct(inout TriangleStream<RasterVertex> outputStream)
{
    for (int ii = 0; ii < 3; ++ii)
    {
        RasterVertex v;
        v.position = coarseArray[ii].position;
        v.color = coarseArray[ii].color;
        outputStream.Append(v);
    }
}

// An array of vector scalarizes to an `address`: a single Input array (not
// SOA-ized per field). The captured chain proves the loaded element reaches the
// position output.

// VEC_SPIRV: OpExecutionMode %mainVecArray Triangles
// VEC_SPIRV-DAG: OpTypeArray %v4float %int_3
// VEC_SPIRV: %[[VPOS:[0-9]+]] = OpAccessChain %_ptr_Function_v4float %v %int_0
// VEC_SPIRV: %[[VP:[0-9]+]] = OpAccessChain %_ptr_Input_v4float %coarseVecArray %{{[0-9]+}}
// VEC_SPIRV: %[[EV:[0-9]+]] = OpLoad %v4float %[[VP]]
// VEC_SPIRV: OpStore %[[VPOS]] %[[EV]]
// VEC_SPIRV: %[[RV:[0-9]+]] = OpLoad %RasterVertex %v
// VEC_SPIRV: %[[OPOS:[0-9]+]] = OpCompositeExtract %v4float %[[RV]] 0
// VEC_SPIRV: OpStore %outputStream_position %[[OPOS]]

// VEC_GLSL: in vec4 {{.*}}coarseVecArray{{[0-9_]*}}[3];
// VEC_GLSL: [[V:[A-Za-z_0-9]+]].position{{[0-9_]*}} = {{.*}}coarseVecArray{{[0-9_]*}}[ii{{[0-9_]*}}];
// VEC_GLSL: outputStream_position{{[0-9_]*}} = [[V]].position{{[0-9_]*}};

[shader("geometry")]
[maxvertexcount(3)]
void mainVecArray(inout TriangleStream<RasterVertex> outputStream)
{
    for (int ii = 0; ii < 3; ++ii)
    {
        RasterVertex v;
        v.position = coarseVecArray[ii];
        v.color = float3(0);
        outputStream.Append(v);
    }
}

// A standalone multi-field struct scalarizes to a `tuple` without array nesting:
// each field is a scalar Input variable read directly (no access chain). Each
// field flows from its Input variable through `v` to the matching output.

// STANDALONE_SPIRV: OpExecutionMode %mainPlainStruct Triangles
// STANDALONE_SPIRV-DAG: %coarseOne_position = OpVariable %_ptr_Input_v4float Input
// STANDALONE_SPIRV-DAG: %coarseOne_color = OpVariable %_ptr_Input_v3float Input
// STANDALONE_SPIRV: %[[VPOS:[0-9]+]] = OpAccessChain %_ptr_Function_v4float %v %int_0
// STANDALONE_SPIRV: %[[PV:[0-9]+]] = OpLoad %v4float %coarseOne_position
// STANDALONE_SPIRV: OpStore %[[VPOS]] %[[PV]]
// STANDALONE_SPIRV: %[[VCOL:[0-9]+]] = OpAccessChain %_ptr_Function_v3float %v %int_1
// STANDALONE_SPIRV: %[[CV:[0-9]+]] = OpLoad %v3float %coarseOne_color
// STANDALONE_SPIRV: OpStore %[[VCOL]] %[[CV]]
// STANDALONE_SPIRV: %[[RV:[0-9]+]] = OpLoad %RasterVertex %v
// STANDALONE_SPIRV: %[[OPOS:[0-9]+]] = OpCompositeExtract %v4float %[[RV]] 0
// STANDALONE_SPIRV: OpStore %outputStream_position %[[OPOS]]
// STANDALONE_SPIRV: %[[OCOL:[0-9]+]] = OpCompositeExtract %v3float %[[RV]] 1
// STANDALONE_SPIRV: OpStore %outputStream_color %[[OCOL]]

// STANDALONE_GLSL-DAG: in vec4 {{.*}}coarseOne_position{{[0-9_]*}};
// STANDALONE_GLSL-DAG: in vec3 {{.*}}coarseOne_color{{[0-9_]*}};
// The local `v` (not the proxy copy) carries each input field; the output reads
// position from `v` and color from the whole-struct copy `[[S]] = v`.
// STANDALONE_GLSL: [[V:v[0-9_]+]].position{{[0-9_]*}} = {{.*}}coarseOne_position{{[0-9_]*}};
// STANDALONE_GLSL: [[V]].color{{[0-9_]*}} = {{.*}}coarseOne_color{{[0-9_]*}};
// STANDALONE_GLSL: [[S:[A-Za-z_0-9]+]] = [[V]];
// STANDALONE_GLSL: outputStream_position{{[0-9_]*}} = [[V]].position{{[0-9_]*}};
// STANDALONE_GLSL: outputStream_color{{[0-9_]*}} = [[S]].color{{[0-9_]*}};

[shader("geometry")]
[maxvertexcount(3)]
void mainPlainStruct(inout TriangleStream<RasterVertex> outputStream)
{
    RasterVertex v;
    v.position = coarseOne.position;
    v.color = coarseOne.color;
    outputStream.Append(v);
}
