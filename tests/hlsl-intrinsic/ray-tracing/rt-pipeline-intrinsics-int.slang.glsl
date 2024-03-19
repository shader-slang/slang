#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 110 0
struct MyAttributes_0
{
    float attr_0;
};


#line 9790 1
hitAttributeEXT
MyAttributes_0 a_0;


#line 6 0
float CheckRayDispatchValues_0()
{


    uvec3 ri_0 = ((gl_LaunchIDEXT));
    uvec3 rd_0 = ((gl_LaunchSizeEXT));

#line 16
    return float(ri_0.x) + float(ri_0.y) + float(ri_0.z) + float(rd_0.x) + float(rd_0.y) + float(rd_0.z);
}

float CheckRaySystemValues_0()
{


    vec3 wro_0 = ((gl_WorldRayOriginEXT));
    float val_0 = wro_0.x + wro_0.y + wro_0.z;

    vec3 wrd_0 = ((gl_WorldRayDirectionEXT));
    float val_1 = val_0 + wrd_0.x + wrd_0.y + wrd_0.z;

    float rayTMin_0 = ((gl_RayTminEXT));
    float val_2 = val_1 + rayTMin_0;

    float rayTCurrent_0 = ((gl_RayTmaxEXT));
    float val_3 = val_2 + rayTCurrent_0;

    uint rayFlags_0 = ((gl_IncomingRayFlagsEXT));

#line 35
    float val_4;
    switch(rayFlags_0)
    {
    case 0U:
    case 1U:
    case 2U:
    case 4U:
    case 8U:
    case 16U:
    case 32U:
    case 64U:
    case 128U:
    case 256U:
    case 512U:
        {

#line 36
            val_4 = val_3 + 1.0;

#line 36
            break;
        }
    default:
        {

#line 36
            val_4 = val_3;

#line 36
            break;
        }
    }

#line 56
    return val_4;
}

float CheckObjectSpaceSystemValues_0()
{


    uint _S1 = ((gl_InstanceID));

#line 63
    float _S2 = float(_S1);
    uint _S3 = ((gl_InstanceCustomIndexEXT));

#line 64
    float val_5 = _S2 + float(_S3);
    uint _S4 = ((gl_GeometryIndexEXT));

#line 65
    float val_6 = val_5 + float(_S4);
    uint _S5 = ((gl_PrimitiveID));

#line 66
    float val_7 = val_6 + float(_S5);

    vec3 oro_0 = ((gl_ObjectRayOriginEXT));
    float val_8 = val_7 + oro_0.x + oro_0.y + oro_0.z;

    vec3 ord_0 = ((gl_ObjectRayDirectionEXT));
    float val_9 = val_8 + ord_0.x + ord_0.y + ord_0.z;

    mat3x4 f3x4_0 = (transpose(gl_ObjectToWorldEXT));


    float val_10 = val_9 + float(f3x4_0[0][0]) + float(f3x4_0[0][1]) + float(f3x4_0[0][2]) + float(f3x4_0[0][3]) + float(f3x4_0[1][0]) + float(f3x4_0[1][1]) + float(f3x4_0[1][2]) + float(f3x4_0[1][3]) + float(f3x4_0[2][0]) + float(f3x4_0[2][1]) + float(f3x4_0[2][2]) + float(f3x4_0[2][3]);

    mat4x3 f4x3_0 = ((gl_ObjectToWorldEXT));



    float val_11 = val_10 + float(f4x3_0[0][0]) + float(f4x3_0[0][1]) + float(f4x3_0[0][2]) + float(f4x3_0[1][0]) + float(f4x3_0[1][1]) + float(f4x3_0[1][2]) + float(f4x3_0[2][0]) + float(f4x3_0[2][1]) + float(f4x3_0[2][2]) + float(f4x3_0[3][0]) + float(f4x3_0[3][1]) + float(f4x3_0[3][2]);

    mat3x4 f3x4_1 = (transpose(gl_WorldToObjectEXT));


    float val_12 = val_11 + float(f3x4_1[0][0]) + float(f3x4_1[0][1]) + float(f3x4_1[0][2]) + float(f3x4_1[0][3]) + float(f3x4_1[1][0]) + float(f3x4_1[1][1]) + float(f3x4_1[1][2]) + float(f3x4_1[1][3]) + float(f3x4_1[2][0]) + float(f3x4_1[2][1]) + float(f3x4_1[2][2]) + float(f3x4_1[2][3]);

    mat4x3 f4x3_1 = ((gl_WorldToObjectEXT));

#line 96
    return val_12 + float(f4x3_1[0][0]) + float(f4x3_1[0][1]) + float(f4x3_1[0][2]) + float(f4x3_1[1][0]) + float(f4x3_1[1][1]) + float(f4x3_1[1][2]) + float(f4x3_1[2][0]) + float(f4x3_1[2][1]) + float(f4x3_1[2][2]) + float(f4x3_1[3][0]) + float(f4x3_1[3][1]) + float(f4x3_1[3][2]);
}

float CheckSysValueIntrinsics_0()
{


    float _S6 = CheckRayDispatchValues_0();
    float _S7 = CheckRaySystemValues_0();

#line 104
    float val_13 = _S6 + _S7;
    float _S8 = CheckObjectSpaceSystemValues_0();

    return val_13 + _S8;
}


#line 9767 1
bool ReportHit_0(float tHit_0, uint hitKind_0, MyAttributes_0 attributes_0)
{

#line 9792
    a_0 = attributes_0;
    bool _S9 = reportIntersectionEXT(tHit_0, hitKind_0);

#line 9793
    return _S9;
}


#line 116 0
void main()
{

    MyAttributes_0 IDs_0;

    const vec3 center_0 = vec3(0.0, 0.0, 1.0);


    vec3 _S10 = ((gl_WorldRayOriginEXT));

#line 124
    vec3 oc_0 = _S10 - center_0;
    vec3 _S11 = ((gl_WorldRayDirectionEXT));

#line 125
    vec3 _S12 = ((gl_WorldRayDirectionEXT));

#line 125
    float a_1 = dot(_S11, _S12);
    vec3 _S13 = ((gl_WorldRayDirectionEXT));

#line 126
    float b_0 = dot(oc_0, _S13);

    float discriminant_0 = b_0 * b_0 - a_1 * (dot(oc_0, oc_0) - 25.0);

    float _S14 = CheckSysValueIntrinsics_0();

    if(discriminant_0 >= 0.0)
    {
        float val_14 = _S14 + discriminant_0;

        float _S15 = - b_0;

#line 136
        float _S16 = sqrt(discriminant_0);

#line 136
        float t1_0 = (_S15 - _S16) / a_1;
        float t2_0 = (_S15 + _S16) / a_1;

        float rayTMin_1 = ((gl_RayTminEXT));
        float rayTMax_0 = ((gl_RayTmaxEXT));
        bool _S17 = rayTMin_1 <= t1_0;

#line 141
        bool _S18;

#line 141
        if(_S17)
        {

#line 141
            _S18 = t1_0 < rayTMax_0;

#line 141
        }
        else
        {

#line 141
            _S18 = false;

#line 141
        }

#line 141
        if(_S18)
        {

#line 141
            _S18 = true;

#line 141
        }
        else
        {

#line 141
            if(rayTMin_1 <= t2_0)
            {

#line 141
                _S18 = t2_0 < rayTMax_0;

#line 141
            }
            else
            {

#line 141
                _S18 = false;

#line 141
            }

#line 141
        }

#line 141
        if(_S18)
        {
            IDs_0.attr_0 = val_14;
            if(_S17)
            {

#line 144
                _S18 = t1_0 < rayTMax_0;

#line 144
            }
            else
            {

#line 144
                _S18 = false;

#line 144
            }

#line 144
            float _S19;

#line 144
            if(_S18)
            {

#line 144
                _S19 = t1_0;

#line 144
            }
            else
            {

#line 144
                _S19 = t2_0;

#line 144
            }

#line 144
            bool _S20 = ReportHit_0(_S19, 0U, IDs_0);

#line 141
        }

#line 132
    }

#line 147
    return;
}

