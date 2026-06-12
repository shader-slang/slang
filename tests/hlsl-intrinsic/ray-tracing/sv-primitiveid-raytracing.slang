// Regression test for #11197: SV_PrimitiveID is allowed in intersection,
// any-hit, and closest-hit stages.

//TEST:SIMPLE(filecheck=ISECT_SPIRV):-target spirv-assembly -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_SPIRV):-target spirv-assembly -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_SPIRV):-target spirv-assembly -entry main_chit -stage closesthit
//TEST:SIMPLE(filecheck=ISECT_GLSL):-target glsl -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_GLSL):-target glsl -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_GLSL):-target glsl -entry main_chit -stage closesthit
//TEST:SIMPLE(filecheck=ISECT_DXIL):-target dxil-assembly -entry main_isect -stage intersection -profile sm_6_5
//TEST:SIMPLE(filecheck=AHIT_DXIL):-target dxil-assembly -entry main_ahit -stage anyhit -profile sm_6_5
//TEST:SIMPLE(filecheck=CHIT_DXIL):-target dxil-assembly -entry main_chit -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=MULTI_DXIL):-target dxil-assembly -entry main_isect -stage intersection -entry main_chit -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=FORWARD_SPIRV):-target spirv-assembly -entry main_chit_forward -stage closesthit
//TEST:SIMPLE(filecheck=FORWARD_GLSL):-target glsl -entry main_chit_forward -stage closesthit
//TEST:SIMPLE(filecheck=FORWARD_HLSL):-target hlsl -entry main_chit_forward -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=FORWARD_DXIL):-target dxil-assembly -entry main_chit_forward -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=NAME_COLLISION_DXIL):-target dxil-assembly -entry main_chit_name_collision -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=NAME_COLLISION_SPIRV):-target spirv-assembly -entry main_chit_name_collision -stage closesthit
//TEST:SIMPLE(filecheck=NAME_COLLISION_GLSL):-target glsl -entry main_chit_name_collision -stage closesthit
//TEST:SIMPLE(filecheck=NAME_COLLISION_CUDA):-target cuda -entry main_chit_name_collision -stage closesthit
//TEST:SIMPLE(filecheck=ADDRESS_HLSL):-target hlsl -entry main_chit_inout -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=ADDRESS_DXIL):-target dxil-assembly -entry main_chit_inout -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=STRUCT_HLSL):-target hlsl -entry main_chit_struct -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=STRUCT_DXIL):-target dxil-assembly -entry main_chit_struct -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=ISECT_CUDA):-target cuda -entry main_isect_cuda -stage intersection
//TEST:SIMPLE(filecheck=AHIT_CUDA):-target cuda -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_CUDA):-target cuda -entry main_chit -stage closesthit

struct Payload
{
    float val;
};

struct Attribs
{
    float v;
};

float getPrimitiveIDAsFloat(in int value)
{
    return float(value);
}

void adjustPrimitiveID(inout uint value)
{
    value += 3;
}

uint __slang_ray_tracing_primitive_index()
{
    return 7;
}

[shader("intersection")]
void main_isect(int pid : SV_PrimitiveID)
{
    Attribs attr = { float(pid) };
    ReportHit<Attribs>(1.0, 0, attr);
}

// ISECT_SPIRV: OpEntryPoint IntersectionKHR
// ISECT_SPIRV: BuiltIn PrimitiveId
// ISECT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

// ISECT_GLSL: #version 460
// ISECT_GLSL-NOT: pid_
// ISECT_GLSL: gl_PrimitiveID
// ISECT_GLSL-NOT: pid_
// ISECT_GLSL: reportIntersectionEXT

// ISECT_DXIL: call i32 @dx.op.primitiveIndex.i32
// ISECT_DXIL: sitofp i32
// ISECT_DXIL: call i1 @dx.op.reportHit.struct.Attribs_0

// ISECT_CUDA: __global__ void __intersection__main_isect_cuda
// ISECT_CUDA-NOT: pid
// ISECT_CUDA: optixGetPrimitiveIndex()

[shader("intersection")]
void main_isect_cuda(int pid : SV_PrimitiveID)
{
    ReportHitOptix(1.0 + float(pid) * 0.0, 0, 1);
}

[shader("anyhit")]
void main_ahit(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes,
    int pid : SV_PrimitiveID)
{
    payload.val = float(pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// AHIT_SPIRV: OpEntryPoint AnyHitKHR
// AHIT_SPIRV: BuiltIn PrimitiveId
// AHIT_SPIRV-DAG: OpTypePointer HitAttribute{{NV|KHR}}
// AHIT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

// AHIT_GLSL: #version 460
// AHIT_GLSL-NOT: pid_
// AHIT_GLSL: gl_PrimitiveID
// AHIT_GLSL-NOT: pid_
// AHIT_GLSL: payload_0.val_0

// AHIT_DXIL: call i32 @dx.op.primitiveIndex.i32
// AHIT_DXIL: sitofp i32

// AHIT_CUDA: __global__ void __anyhit__main_ahit
// AHIT_CUDA-NOT: pid
// AHIT_CUDA: optixGetPrimitiveIndex()

[shader("closesthit")]
void main_chit(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    int pid : SV_PrimitiveID)
{
    payload.val = float(pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// CHIT_SPIRV: OpEntryPoint ClosestHitKHR
// CHIT_SPIRV: BuiltIn PrimitiveId
// CHIT_SPIRV-DAG: OpTypePointer HitAttribute{{NV|KHR}}
// CHIT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

// CHIT_GLSL: #version 460
// CHIT_GLSL-NOT: pid_
// CHIT_GLSL: gl_PrimitiveID
// CHIT_GLSL-NOT: pid_
// CHIT_GLSL: payload_0.val_0

// CHIT_DXIL: call i32 @dx.op.primitiveIndex.i32
// CHIT_DXIL: sitofp i32
// MULTI_DXIL-COUNT-2: call i32 @dx.op.primitiveIndex.i32

// CHIT_CUDA: __global__ void __closesthit__main_chit
// CHIT_CUDA-NOT: pid
// CHIT_CUDA: optixGetPrimitiveIndex()

[shader("closesthit")]
void main_chit_forward(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    int pid : SV_PrimitiveID)
{
    payload.val =
        getPrimitiveIDAsFloat(pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// FORWARD_SPIRV: OpEntryPoint ClosestHitKHR
// FORWARD_SPIRV: BuiltIn PrimitiveId
// FORWARD_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

// FORWARD_GLSL: #version 460
// FORWARD_GLSL-NOT: pid_
// FORWARD_GLSL: gl_PrimitiveID
// FORWARD_GLSL-NOT: pid_
// FORWARD_GLSL: payload_0.val_0

// FORWARD_HLSL: [shader("closesthit")]void main_chit_forward
// FORWARD_HLSL-NOT: SV_PrimitiveID
// FORWARD_HLSL: uint [[PRIMITIVE_ID:[A-Za-z_][A-Za-z_0-9]*]] = PrimitiveIndex()
// FORWARD_HLSL: getPrimitiveIDAsFloat_0(int([[PRIMITIVE_ID]]))

// FORWARD_DXIL: call i32 @dx.op.primitiveIndex.i32
// FORWARD_DXIL: sitofp i32

[shader("closesthit")]
void main_chit_name_collision(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    int pid : SV_PrimitiveID)
{
    payload.val = float(pid) + float(__slang_ray_tracing_primitive_index()) +
                  attribs.barycentrics.x + attribs.barycentrics.y;
}

// NAME_COLLISION_DXIL: call i32 @dx.op.primitiveIndex.i32

// NAME_COLLISION_SPIRV: OpEntryPoint ClosestHitKHR
// NAME_COLLISION_SPIRV: BuiltIn PrimitiveId
// NAME_COLLISION_SPIRV: OpLoad %int %gl_PrimitiveID
// NAME_COLLISION_SPIRV: OpConvertUToF %float %uint_7

// NAME_COLLISION_GLSL: #version 460
// NAME_COLLISION_GLSL-DAG: uint [[USER_FUNC:[A-Za-z_][A-Za-z_0-9]*]]()
// NAME_COLLISION_GLSL: gl_PrimitiveID
// NAME_COLLISION_GLSL: float([[USER_FUNC]]())

// NAME_COLLISION_CUDA-DAG: __device__ uint [[USER_FUNC:[A-Za-z_][A-Za-z_0-9]*]]()
// NAME_COLLISION_CUDA: __global__ void __closesthit__main_chit_name_collision
// NAME_COLLISION_CUDA: optixGetPrimitiveIndex()
// NAME_COLLISION_CUDA: float([[USER_FUNC]]())

[shader("closesthit")]
void main_chit_inout(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    uint pid : SV_PrimitiveID)
{
    adjustPrimitiveID(pid);
    payload.val = float(pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// ADDRESS_HLSL: uint [[PRIMITIVE_ID:[A-Za-z_][A-Za-z_0-9]*]] = PrimitiveIndex()
// ADDRESS_HLSL: uint [[MUTABLE_PID:[A-Za-z_][A-Za-z_0-9]*]] = [[PRIMITIVE_ID]]
// ADDRESS_HLSL: adjustPrimitiveID_0([[MUTABLE_PID]])
// ADDRESS_HLSL: float([[MUTABLE_PID]])

// ADDRESS_DXIL: call i32 @dx.op.primitiveIndex.i32

struct ChitPrimitiveInput
{
    int pid : SV_PrimitiveID;
};

[shader("closesthit")]
void main_chit_struct(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attribs,
    in ChitPrimitiveInput input)
{
    payload.val = float(input.pid) + attribs.barycentrics.x + attribs.barycentrics.y;
}

// STRUCT_HLSL: [shader("closesthit")]void main_chit_struct
// STRUCT_HLSL-NOT: SV_PrimitiveID
// STRUCT_HLSL: uint [[PRIMITIVE_ID:[A-Za-z_][A-Za-z_0-9]*]] = PrimitiveIndex()
// STRUCT_HLSL: float(int([[PRIMITIVE_ID]]))

// STRUCT_DXIL: call i32 @dx.op.primitiveIndex.i32
