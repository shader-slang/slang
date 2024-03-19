//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage intersection -profile glsl_460+GL_EXT_ray_tracing
//   Motion rays not supported on HLSL impl currently, so disable with NOMOTION define:
//TEST:CROSS_COMPILE:-target dxil-assembly -entry main -stage intersection -profile sm_6_5 -DNOMOTION


float CheckRayDispatchValues() // all
{
    float val = 0.0f;

    uint3 ri = DispatchRaysIndex();
    uint3 rd = DispatchRaysDimensions();

    val += ri.x; val += ri.y; val += ri.z;
    val += rd.x; val += rd.y; val += rd.z;

    return val;
}

float CheckRaySystemValues() // chit/anyhit/intersection
{
    float val = 0.0f;

    float3 wro = WorldRayOrigin();
    val += wro.x; val += wro.y; val += wro.z;

    float3 wrd = WorldRayDirection();
    val += wrd.x; val += wrd.y; val += wrd.z;

    float rayTMin = RayTMin();
    val += rayTMin;

    float rayTCurrent = RayTCurrent();
    val += rayTCurrent;

    uint rayFlags = RayFlags();
    switch (rayFlags)
    {
        case RAY_FLAG_NONE:
        case RAY_FLAG_FORCE_OPAQUE:
        case RAY_FLAG_FORCE_NON_OPAQUE:
        case RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH:
        case RAY_FLAG_SKIP_CLOSEST_HIT_SHADER:
        case RAY_FLAG_CULL_BACK_FACING_TRIANGLES:
        case RAY_FLAG_CULL_FRONT_FACING_TRIANGLES:
        case RAY_FLAG_CULL_OPAQUE:
        case RAY_FLAG_CULL_NON_OPAQUE:
        case RAY_FLAG_SKIP_TRIANGLES:
        case RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES:
            val += 1.0f;
            break;

        default:
            break;
    }

    return val;
}

float CheckObjectSpaceSystemValues()
{
    float val = 0.0f;

    val += InstanceIndex();
    val += InstanceID();
    val += GeometryIndex();
    val += PrimitiveIndex();

    float3 oro = ObjectRayOrigin();
    val += oro.x; val += oro.y; val += oro.z;

    float3 ord = ObjectRayDirection();
    val += ord.x; val += ord.y; val += ord.z;

    float3x4 f3x4 = ObjectToWorld3x4();
    val += f3x4._11; val += f3x4._12; val += f3x4._13; val += f3x4._14;
    val += f3x4._21; val += f3x4._22; val += f3x4._23; val += f3x4._24;
    val += f3x4._31; val += f3x4._32; val += f3x4._33; val += f3x4._34;

    float4x3 f4x3 = ObjectToWorld4x3();
    val += f4x3._11; val += f4x3._12; val += f4x3._13;
    val += f4x3._21; val += f4x3._22; val += f4x3._23;
    val += f4x3._31; val += f4x3._32; val += f4x3._33;
    val += f4x3._41; val += f4x3._42; val += f4x3._43;

    f3x4 = WorldToObject3x4();
    val += f3x4._11; val += f3x4._12; val += f3x4._13; val += f3x4._14;
    val += f3x4._21; val += f3x4._22; val += f3x4._23; val += f3x4._24;
    val += f3x4._31; val += f3x4._32; val += f3x4._33; val += f3x4._34;

    f4x3 = WorldToObject4x3();
    val += f4x3._11; val += f4x3._12; val += f4x3._13;
    val += f4x3._21; val += f4x3._22; val += f4x3._23;
    val += f4x3._31; val += f4x3._32; val += f4x3._33;
    val += f4x3._41; val += f4x3._42; val += f4x3._43;

    return val;
}

float CheckSysValueIntrinsics()
{
    float val = 0.0f;

    val += CheckRayDispatchValues();
    val += CheckRaySystemValues();
    val += CheckObjectSpaceSystemValues();

    return val;
}

struct MyAttributes
{
    float attr;
};

[shader("intersection")]
void main()
{
    float val = 0.0f;
    MyAttributes IDs;

    float3 center = float3( 0.0f, 0.0f, 1.0f );
    float radius = 5.0f;

    float3 oc = WorldRayOrigin() - center;
    float a = dot(WorldRayDirection(), WorldRayDirection());
    float b = dot(oc, WorldRayDirection());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;

    val += CheckSysValueIntrinsics();

    if (discriminant >= 0.0f)
    {
        val += discriminant;

        float t1 = (-b - sqrt(discriminant)) / a;
        float t2 = (-b + sqrt(discriminant)) / a;

        float rayTMin = RayTMin();
        float rayTMax = RayTCurrent();
        if ((rayTMin <= t1 && t1 < rayTMax) || (rayTMin <= t2 && t2 < rayTMax))
        {
            IDs.attr = val;
            ReportHit( (rayTMin <= t1 && t1 < rayTMax) ? t1 : t2, 0, IDs );
        }
    }
}