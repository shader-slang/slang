// sv-clip-distance-vector.slang

// Vector-typed SV_ClipDistanceN / SV_CullDistanceN values (issue #13931). HLSL passes
// them through. GLSL and SPIR-V store every value of one semantic in the single
// gl_ClipDistance / gl_CullDistance array, one component per element, packed in ascending
// semantic-index order as DXC does. A signature of only scalars keeps each scalar at the
// element named by its semantic index.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage vertex -entry vsVector
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage vertex -entry vsVector
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage vertex -entry vsVector
//TEST:SIMPLE(filecheck=FRAG): -target glsl -stage fragment -entry fsVector
//TEST:SIMPLE(filecheck=FRAGSPV): -target spirv-asm -stage fragment -entry fsVector
//TEST:SIMPLE(filecheck=CULL): -target glsl -stage vertex -entry vsCullScalar
//TEST:SIMPLE(filecheck=CULLSPV): -target spirv-asm -stage vertex -entry vsCullScalar
//TEST:SIMPLE(filecheck=MIXED): -target glsl -stage vertex -entry vsMixed
//TEST:SIMPLE(filecheck=MIXEDSPV): -target spirv-asm -stage vertex -entry vsMixed
//TEST:SIMPLE(filecheck=GAP): -target glsl -stage vertex -entry vsGap
//TEST:SIMPLE(filecheck=VECGAP): -target glsl -stage vertex -entry vsVectorGap
//TEST:SIMPLE(filecheck=FRAGHALF): -target spirv-asm -stage fragment -entry fsHalfDynamicIndex
//TEST:SIMPLE(filecheck=GS): -target glsl -stage geometry -entry gsVector
//TEST:SIMPLE(filecheck=PATCH): -target glsl -stage hull -entry hsMain
//TEST:SIMPLE(filecheck=NVX): -target glsl -stage vertex -entry vsPerView
//TEST:SIMPLE(filecheck=FRAGCULL): -target glsl -stage fragment -entry fsCull
//TEST:SIMPLE(filecheck=FRAGCULLSPV): -target spirv-asm -stage fragment -entry fsCull
//TEST:SIMPLE(filecheck=HALFOUT): -target glsl -stage vertex -entry vsHalf
//TEST:SIMPLE(filecheck=HALFIN): -target glsl -stage fragment -entry fsHalf

// float4 at index 0 takes elements 0-3; float2 at index 1 takes elements 4-5.
struct VSVectorOut
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
    float2 clip1 : SV_ClipDistance1;
};

//HLSL-NOT: error[E
//HLSL: float4 clip0_{{[0-9]+}} : SV_ClipDistance0;
//HLSL: float2 clip1_{{[0-9]+}} : SV_ClipDistance1;

//GLSL: out float gl_ClipDistance[6];
//GLSL-DAG: gl_ClipDistance[0] = {{.*}}clip0{{.*}}[0];
//GLSL-DAG: gl_ClipDistance[1] = {{.*}}clip0{{.*}}[1];
//GLSL-DAG: gl_ClipDistance[2] = {{.*}}clip0{{.*}}[2];
//GLSL-DAG: gl_ClipDistance[3] = {{.*}}clip0{{.*}}[3];
//GLSL-DAG: gl_ClipDistance[4] = {{.*}}clip1{{.*}}[0];
//GLSL-DAG: gl_ClipDistance[5] = {{.*}}clip1{{.*}}[1];

//SPIRV-DAG: OpDecorate %gl_ClipDistance BuiltIn ClipDistance
//SPIRV-DAG: %gl_ClipDistance = OpVariable %_ptr_Output__arr_float_int_6 Output

VSVectorOut vsVector(uint vid: SV_VertexID)
{
    VSVectorOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip0 = float4(1, 2, 3, 4);
    o.clip1 = float2(5, 6);
    return o;
}

// The parameters are declared out of semantic-index order; the packing still follows the
// semantic index.

//FRAG: vec4(gl_ClipDistance[1], gl_ClipDistance[3], gl_ClipDistance[4], gl_ClipDistance[5])

//FRAGSPV-DAG: OpDecorate %gl_ClipDistance BuiltIn ClipDistance
//FRAGSPV-DAG: %gl_ClipDistance = OpVariable %_ptr_Input__arr_float_int_6 Input

float4 fsVector(float2 clip1: SV_ClipDistance1, float4 clip0: SV_ClipDistance0) : SV_Target
{
    return float4(clip0.y, clip0.w, clip1.x, clip1.y);
}

// Scalar SV_CullDistanceN values share one gl_CullDistance array, as SV_ClipDistanceN do.
struct VSCullOut
{
    float4 position : SV_Position;
    float cull0 : SV_CullDistance0;
    float cull1 : SV_CullDistance1;
};

//CULL-NOT: #extension {{.*}}cull_distance
//CULL-NOT: gl_CullDistance =
//CULL-DAG: gl_CullDistance[0] = {{.*}}cull0
//CULL-DAG: gl_CullDistance[1] = {{.*}}cull1

//CULLSPV-DAG: OpDecorate %gl_CullDistance BuiltIn CullDistance
//CULLSPV-DAG: %gl_CullDistance = OpVariable %_ptr_Output__arr_float_int_2 Output

VSCullOut vsCullScalar(uint vid: SV_VertexID)
{
    VSCullOut o;
    o.position = float4(vid, 0, 0, 1);
    o.cull0 = 7;
    o.cull1 = 8;
    return o;
}

// Clip and cull values are placed independently, across the return value and `out`
// parameters: clip0 takes clip elements 0-1 and clip1 takes elements 2-3; cull0 takes cull
// elements 0-2 and cull1 takes element 3.
struct VSMixedOut
{
    float4 position : SV_Position;
    float2 clip0 : SV_ClipDistance0;
    float3 cull0 : SV_CullDistance0;
};

//MIXED: out float gl_ClipDistance[4];
//MIXED-DAG: gl_ClipDistance[0] = {{.*}}clip0{{.*}}[0];
//MIXED-DAG: gl_ClipDistance[1] = {{.*}}clip0{{.*}}[1];
//MIXED-DAG: gl_ClipDistance[2] = 3.0;
//MIXED-DAG: gl_ClipDistance[3] = 8.0;
//MIXED-DAG: gl_CullDistance[0] = {{.*}}cull0{{.*}}[0];
//MIXED-DAG: gl_CullDistance[1] = {{.*}}cull0{{.*}}[1];
//MIXED-DAG: gl_CullDistance[2] = {{.*}}cull0{{.*}}[2];
//MIXED-DAG: gl_CullDistance[3] = 7.0;

//MIXEDSPV-DAG: %gl_ClipDistance = OpVariable %_ptr_Output__arr_float_int_4 Output
//MIXEDSPV-DAG: %gl_CullDistance = OpVariable %_ptr_Output__arr_float_int_4 Output

VSMixedOut vsMixed(
    uint vid: SV_VertexID,
    out float2 clip1: SV_ClipDistance1,
    out float cull1: SV_CullDistance1)
{
    VSMixedOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip0 = float2(1, 2);
    o.cull0 = float3(4, 5, 6);
    clip1 = float2(3, 8);
    cull1 = 7;
    return o;
}

// A scalar keeps the element named by its semantic index, even across a gap in the indices.
struct VSGapOut
{
    float4 position : SV_Position;
    float clip0 : SV_ClipDistance0;
    float clip3 : SV_ClipDistance3;
};

//GAP: out float gl_ClipDistance[4];
//GAP-DAG: gl_ClipDistance[0] = {{.*}}clip0
//GAP-DAG: gl_ClipDistance[3] = {{.*}}clip3

VSGapOut vsGap(uint vid: SV_VertexID)
{
    VSGapOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip0 = 1;
    o.clip3 = 2;
    return o;
}

// Once a signature has a vector, a gap in the semantic indices is packed away, as DXC does.
struct VSVectorGapOut
{
    float4 position : SV_Position;
    float2 clip0 : SV_ClipDistance0;
    float clip2 : SV_ClipDistance2;
    float2 clip5 : SV_ClipDistance5;
};

//VECGAP: out float gl_ClipDistance[5];
//VECGAP-DAG: gl_ClipDistance[0] = {{.*}}clip0{{.*}}[0];
//VECGAP-DAG: gl_ClipDistance[1] = {{.*}}clip0{{.*}}[1];
//VECGAP-DAG: gl_ClipDistance[2] = {{.*}}clip2
//VECGAP-DAG: gl_ClipDistance[3] = {{.*}}clip5{{.*}}[0];
//VECGAP-DAG: gl_ClipDistance[4] = {{.*}}clip5{{.*}}[1];

VSVectorGapOut vsVectorGap(uint vid: SV_VertexID)
{
    VSVectorGapOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip0 = float2(1, 2);
    o.clip2 = 3;
    o.clip5 = float2(4, 5);
    return o;
}

// A vector whose type differs from floatN is converted, and a dynamic component index selects
// the matching element.
cbuffer Constants
{
    int componentIndex;
}

//FRAGHALF: %gl_ClipDistance = OpVariable %_ptr_Input__arr_float_int_6 Input
//FRAGHALF: %[[ELEMENT:[0-9]+]] = OpIAdd %int %int_4
//FRAGHALF: OpAccessChain %_ptr_Input_float %gl_ClipDistance %[[ELEMENT]]

float4 fsHalfDynamicIndex(float4 clip0: SV_ClipDistance0, half2 clip1: SV_ClipDistance1) : SV_Target
{
    return float4(clip0.x, clip1[componentIndex], 0, 1);
}

// Geometry-shader stream outputs use the same placement as vertex-shader outputs.
struct GSVectorOut
{
    float4 position : SV_Position;
    float3 clip0 : SV_ClipDistance0;
    float clip1 : SV_ClipDistance1;
};

//GS: out float gl_ClipDistance[4];
//GS-DAG: gl_ClipDistance[0] = {{.*}}clip0{{.*}}[0];
//GS-DAG: gl_ClipDistance[1] = {{.*}}clip0{{.*}}[1];
//GS-DAG: gl_ClipDistance[2] = {{.*}}clip0{{.*}}[2];
//GS-DAG: gl_ClipDistance[3] = {{.*}}clip1

struct GSIn
{
    float4 position : SV_Position;
};

[maxvertexcount(1)]
void gsVector(point GSIn input[1], inout PointStream<GSVectorOut> stream)
{
    GSVectorOut o;
    o.position = input[0].position;
    o.clip0 = float3(1, 2, 3);
    o.clip1 = 4;
    stream.Append(o);
}

// Patch-constant outputs need their own placement table; otherwise the clip offset
// getValue lookup fails internally. This test does not validate the emitted GLSL.
struct HSControlPoint
{
    float4 position : SV_Position;
};

struct HSPatchConstants
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
    float clip : SV_ClipDistance;
};

//PATCH: gl_ClipDistance[0] = {{.*}}clip

HSPatchConstants patchConstants(InputPatch<HSControlPoint, 3> patch)
{
    HSPatchConstants o;
    o.edges[0] = 1;
    o.edges[1] = 1;
    o.edges[2] = 1;
    o.inside = 1;
    o.clip = 2;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchConstants")]
HSControlPoint hsMain(InputPatch<HSControlPoint, 3> patch, uint id: SV_OutputControlPointID)
{
    return patch[id];
}

// Other system values that use one element of a builtin array keep storing a vector in a
// single element.
struct VSPerViewOut
{
    float4 left : SV_Position;
    float4 right : NV_X_RIGHT;
};

//NVX: gl_PositionPerViewNV[1]{{.*}} = {{[^\[]*}}right{{[_0-9]*}};

VSPerViewOut vsPerView(float4 left: A, float4 right: B)
{
    VSPerViewOut o;
    o.left = left;
    o.right = right;
    return o;
}

// Fragment-shader cull-distance inputs use the same placement as clip-distance inputs.
//FRAGCULL-NOT: #extension {{.*}}cull_distance
//FRAGCULL: vec4(vec2(gl_CullDistance[0], gl_CullDistance[1]), gl_CullDistance[2], 1.0)

//FRAGCULLSPV-DAG: OpDecorate %gl_CullDistance BuiltIn CullDistance
//FRAGCULLSPV-DAG: %gl_CullDistance = OpVariable %_ptr_Input__arr_float_int_3 Input

float4 fsCull(float2 cull0: SV_CullDistance0, float cull1: SV_CullDistance1) : SV_Target
{
    return float4(cull0, cull1, 1);
}

// A vector whose type differs from floatN is converted as a whole on the way out and in.
struct VSHalfOut
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
    half2 clip1 : SV_ClipDistance1;
};

//HALFOUT: out float gl_ClipDistance[6];
//HALFOUT-DAG: gl_ClipDistance[4] =
//HALFOUT-DAG: gl_ClipDistance[5] =

VSHalfOut vsHalf(uint vid: SV_VertexID)
{
    VSHalfOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip0 = float4(1, 2, 3, 4);
    o.clip1 = half2(5, 6);
    return o;
}

//HALFIN: gl_ClipDistance[4]
//HALFIN: gl_ClipDistance[5]

float4 fsHalf(float4 clip0: SV_ClipDistance0, half2 clip1: SV_ClipDistance1) : SV_Target
{
    return float4(clip1, clip0.x, 1);
}
