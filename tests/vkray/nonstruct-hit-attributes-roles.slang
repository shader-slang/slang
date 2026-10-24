// Wrapper structs are cached per value type and per role. A float4 ray payload and float4 hit
// attributes in one module therefore get distinct wrappers, and at SM 6.7 only the ray payload
// wrapper gets access qualifiers. A ReportHit inside an ordinary helper is wrapped at that call.

//TEST:SIMPLE(filecheck=PAQ): -target hlsl -profile lib_6_7 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -profile lib_6_7 -whole-program

[noinline]
void reportFromHelper(float4 attributes)
{
    ReportHit(1.0, 0, attributes);
}

[shader("intersection")]
void isect()
{
    reportFromHelper(float4(0.25, 0.5, 0.75, 1.0));
}

[shader("closesthit")]
void closestHit(inout float4 payload, in float4 attributes)
{
    payload = attributes;
}

// PAQ: struct [[attrType:HitAttributes_t[A-Za-z0-9_]*]]
// PAQ-NEXT: {
// PAQ-NEXT: float4 {{[A-Za-z0-9_]+}};{{$}}
// PAQ: void reportFromHelper{{[A-Za-z0-9_]*}}(float4 [[helperParam:[A-Za-z0-9_]+]])
// PAQ: [[attrType]] [[wrapped:[A-Za-z0-9_]+]] = { [[helperParam]] };
// PAQ: ReportHit((1.0f), (0U), ([[wrapped]]))
// PAQ: struct [raypayload] [[payloadType:RayPayload_t[A-Za-z0-9_]*]]
// PAQ-NEXT: {
// PAQ-NEXT: float4 {{[A-Za-z0-9_]+}} : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// PAQ: void closestHit(inout [[payloadType]] {{[A-Za-z0-9_]+}}, [[attrType]] {{[A-Za-z0-9_]+}})

// DXIL-DAG: %struct.[[attr:HitAttributes_t[A-Za-z0-9_]*]] = type { <4 x float> }
// DXIL-DAG: %struct.[[payload:RayPayload_t[A-Za-z0-9_]*]] = type { <4 x float> }
// DXIL: call i1 @dx.op.reportHit.struct.[[attr]](
// DXIL: define void @{{.*}}closestHit{{.*}}(%struct.[[payload]]* {{.*}}, %struct.[[attr]]*
