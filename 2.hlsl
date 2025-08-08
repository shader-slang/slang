#pragma pack_matrix(row_major)

#line 18 "tests/compute/dynamic-dispatch-17.slang"
struct UserDefinedPackedType_0
{
    float3 val_0;
    uint flags_0;
};


#line 28
RWStructuredBuffer<UserDefinedPackedType_0 > gObj_0 : register(u1);


#line 25
RWStructuredBuffer<float > gOutputBuffer_0 : register(u0);


#line 25
struct AnyValue16
{
    uint field0_0;
    uint field1_0;
    uint field2_0;
    uint field3_0;
};


#line 25
AnyValue16 packAnyValue16_0(UserDefinedPackedType_0 _S1)
{

#line 25
    AnyValue16 _S2;

#line 25
    _S2.field0_0 = 0U;

#line 25
    _S2.field1_0 = 0U;

#line 25
    _S2.field2_0 = 0U;

#line 25
    _S2.field3_0 = 0U;

#line 25
    _S2.field0_0 = (uint)(asuint(_S1.val_0[int(0)]));

#line 25
    _S2.field1_0 = (uint)(asuint(_S1.val_0[int(1)]));

#line 25
    _S2.field2_0 = (uint)(asuint(_S1.val_0[int(2)]));

#line 25
    _S2.field3_0 = _S1.flags_0;

#line 25
    return _S2;
}


#line 39
uint _S3(uint _S4)
{

#line 39
    switch(_S4)
    {
    case 3U:
        {

#line 39
            return 0U;
        }
    case 4U:
        {

#line 39
            return 1U;
        }
    default:
        {

#line 39
            return 0U;
        }
    }

#line 39
}


#line 50
struct FloatVal_0
{
    float val_1;
};


#line 48
float ReturnsZero_get_0()
{

#line 48
    return 0.0f;
}



float FloatVal_run_0(FloatVal_0 this_0)
{

    float _S5 = ReturnsZero_get_0();

#line 56
    return this_0.val_1 + _S5;
}


#line 56
float U_S4main8FloatVal3rung2TCGP04main12IReturnsZerop0pfG0_ST4main11ReturnsZero1_SW4main11ReturnsZero4main12IReturnsZero_wtwrapper_0(AnyValue16 _S6)
{

#line 56
    float _S7 = FloatVal_run_0(_S6);

#line 56
    return _S7;
}

struct Float4Struct_0
{
    float4 val_2;
};


#line 60
struct Float4Val_0
{
    Float4Struct_0 val_3;
};


#line 63
float Float4Val_run_0(Float4Val_0 this_1)
{

    float _S8 = this_1.val_3.val_2.x + this_1.val_3.val_2.y;

#line 66
    float _S9 = ReturnsZero_get_0();

#line 66
    return _S8 + _S9;
}


#line 66
float U_S4main9Float4Val3rung2TCGP04main12IReturnsZerop0pfG0_ST4main11ReturnsZero1_SW4main11ReturnsZero4main12IReturnsZero_wtwrapper_0(AnyValue16 _S10)
{

#line 66
    float _S11 = Float4Val_run_0(_S10);

#line 66
    return _S11;
}


#line 66
float _S12(uint _S13, AnyValue16 _S14)
{

#line 66
    switch(_S13)
    {
    case 0U:
        {

#line 66
            float _S15 = U_S4main8FloatVal3rung2TCGP04main12IReturnsZerop0pfG0_ST4main11ReturnsZero1_SW4main11ReturnsZero4main12IReturnsZero_wtwrapper_0(_S14);

#line 66
            return _S15;
        }
    case 1U:
        {

#line 66
            float _S16 = U_S4main9Float4Val3rung2TCGP04main12IReturnsZerop0pfG0_ST4main11ReturnsZero1_SW4main11ReturnsZero4main12IReturnsZero_wtwrapper_0(_S14);

#line 66
            return _S16;
        }
    default:
        {

#line 66
            return 0.0f;
        }
    }

#line 66
}


#line 34
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{
    int i_0 = int(0);

#line 37
    float result_0 = 0.0f;

#line 37
    for(;;)
    {

#line 37
        if(i_0 < int(2))
        {
        }
        else
        {

#line 37
            break;
        }
        UserDefinedPackedType_0 rawObj_0 = gObj_0.Load(i_0);
        uint _S17 = _S3(rawObj_0.flags_0);

#line 40
        uint  _S18[int(2)] = { 0U, 1U };

#line 40
        AnyValue16 _S19 = packAnyValue16_0(rawObj_0);
        float _S20 = _S12(_S18[_S17], _S19);

#line 41
        float result_1 = result_0 + _S20;

#line 37
        int i_1 = i_0 + int(1);

#line 37
        i_0 = i_1;

#line 37
        result_0 = result_1;

#line 37
    }

#line 43
    gOutputBuffer_0[int(0)] = result_0;
    return;
}

