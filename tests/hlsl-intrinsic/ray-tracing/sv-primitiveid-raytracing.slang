// Regression test for #11197: SV_PrimitiveID is allowed in intersection,
// any-hit, and closest-hit stages.

//TEST:SIMPLE(filecheck=ISECT_SPIRV):-target spirv-assembly -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_SPIRV):-target spirv-assembly -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_SPIRV):-target spirv-assembly -entry main_chit -stage closesthit
//TEST:SIMPLE(filecheck=ISECT_DXIL):-target dxil-assembly -entry main_isect -stage intersection -profile sm_6_5
//TEST:SIMPLE(filecheck=AHIT_DXIL):-target dxil-assembly -entry main_ahit -stage anyhit -profile sm_6_5
//TEST:SIMPLE(filecheck=CHIT_DXIL):-target dxil-assembly -entry main_chit -stage closesthit -profile sm_6_5
//TEST:SIMPLE(filecheck=MULTI_DXIL):-target dxil-assembly -entry main_isect -stage intersection -entry main_chit -stage closesthit -profile sm_6_5
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

[shader("intersection")]
void main_isect(int pid : SV_PrimitiveID)
{
    Attribs attr = { float(pid) };
    ReportHit<Attribs>(1.0, 0, attr);
}

// ISECT_SPIRV: OpEntryPoint IntersectionKHR
// ISECT_SPIRV: BuiltIn PrimitiveId
// ISECT_SPIRV: %{{.*}} = OpVariable %_ptr_Input_int Input

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

// CHIT_DXIL: call i32 @dx.op.primitiveIndex.i32
// CHIT_DXIL: sitofp i32
// MULTI_DXIL-COUNT-2: call i32 @dx.op.primitiveIndex.i32

// CHIT_CUDA: __global__ void __closesthit__main_chit
// CHIT_CUDA-NOT: pid
// CHIT_CUDA: optixGetPrimitiveIndex()
