//TEST:SIMPLE(filecheck=CHK): -target spirv

//struct State {
//    float3 v;
//    float3 s;
//    int rnd;

//    //CHK-DAG: warning 41022: parameter 'this' has an inout qualifier, but is never written to
//    [mutating]
//    float3 rotate(float3 n)
//    {
//        float3 w = n;
//        float3 a = float3(0, 1, 0);
//        float3 u = normalize(cross(w, a));
//        float3 v = normalize(cross(w, u));
//        return u * s.x + v * s.y + w * s.z;
//    }
//};

////CHK-DAG: warning 41022: parameter 'x' has an inout qualifier, but is never written to
//void int_never_assigned(inout int x) {}

////CHK-DAG: warning 41022: parameter 'state' has an inout qualifier, but is never written to
//void state_never_assigned(inout State state, inout float v)
//{
//    v = state.v.x;
//}

//void state_assigned(inout State state)
//{
//    int3 vi = __slang_noop_cast <int3, float3> (state.v);
//    int3 si = __slang_noop_cast <int3, float3> (state.s);
//    state.rnd = dot(vi, si);
//}

__generic<payload_t>
[require(glsl_hlsl_spirv, raytracing)]
void __traceRayHLSL(
        RaytracingAccelerationStructure AccelerationStructure,
        uint RayFlags,
        uint InstanceInclusionMask,
        uint RayContributionToHitGroupIndex,
        uint MultiplierForGeometryContributionToHitGroupIndex,
        uint MissShaderIndex,
        RayDesc Ray,
        inout payload_t Payload)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "TraceRay";
    case glsl: __intrinsic_asm "TraceRayGLSL";
    default: __intrinsic_asm "TraceRaySPIRV";
    }
}

// TODO: shade entry points

//CHK-NOT: warning 41022
