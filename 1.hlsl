#pragma pack_matrix(row_major)
RWStructuredBuffer<float > outputBuffer_0 : register(u0);

struct MaterialHeader_0
{
    uint4 header_0;
};

struct MaterialPayload_0
{
    float4 data_0;
};

struct MaterialDataBlob_0
{
    MaterialHeader_0 header_1;
    MaterialPayload_0 payload_0;
};

RWStructuredBuffer<MaterialDataBlob_0 > g_materials_0 : register(u1);

struct AnyValue40
{
    uint field0_0;
    uint field1_0;
    uint field2_0;
    uint field3_0;
    uint field4_0;
    uint field5_0;
    uint field6_0;
    uint field7_0;
    uint field8_0;
    uint field9_0;
};

struct ShadingInput_0
{
    float scale_0;
};

struct AnyValue28
{
    uint field0_1;
    uint field1_1;
    uint field2_1;
    uint field3_1;
    uint field4_1;
    uint field5_1;
    uint field6_1;
};

struct Material2_0
{
    MaterialHeader_0 header_2;
    float a_0;
    float b_0;
};

struct Material2_Differential_0
{
    float a_1;
    float b_1;
};

struct DiffPair_Material2_0
{
    Material2_0 primal_0;
    Material2_Differential_0 differential_0;
};

DiffPair_Material2_0 unpackAnyValue40_0(AnyValue40 _S1)
{
    DiffPair_Material2_0 _S2;
    _S2.primal_0.header_2.header_0[int(0)] = _S1.field0_0;
    _S2.primal_0.header_2.header_0[int(1)] = _S1.field1_0;
    _S2.primal_0.header_2.header_0[int(2)] = _S1.field2_0;
    _S2.primal_0.header_2.header_0[int(3)] = _S1.field3_0;
    _S2.primal_0.a_0 = asfloat(_S1.field4_0);
    _S2.primal_0.b_0 = asfloat(_S1.field5_0);
    _S2.differential_0.a_1 = asfloat(_S1.field6_0);
    _S2.differential_0.b_1 = asfloat(_S1.field7_0);
    return _S2;
}

struct MaterialInstance2_0
{
    float a_2;
    float b_2;
};

MaterialInstance2_0 unpackAnyValue28_0(AnyValue28 _S3)
{
    MaterialInstance2_0 _S4;
    _S4.a_2 = asfloat(_S3.field0_1);
    _S4.b_2 = asfloat(_S3.field1_1);
    return _S4;
}

Material2_Differential_0 Material2_x24_syn_dzero_0()
{
    Material2_Differential_0 result_0;
    result_0.a_1 = 0.0;
    result_0.b_1 = 0.0;
    return result_0;
}

void s_bwd_prop_Material2_setupMaterialInstance_0(inout DiffPair_Material2_0 dpthis_0, ShadingInput_0 input_0, MaterialInstance2_0 _s_dOut_0)
{
    float _S5 = input_0.scale_0 * (input_0.scale_0 * _s_dOut_0.b_2);
    float _S6 = input_0.scale_0 * (input_0.scale_0 * _s_dOut_0.a_2);
    Material2_Differential_0 _S7 = Material2_x24_syn_dzero_0();
    _S7.b_1 = _S5;
    _S7.a_1 = _S6;
    dpthis_0.primal_0 = dpthis_0.primal_0;
    dpthis_0.differential_0 = _S7;
    return;
}

void s_bwd_Material2_setupMaterialInstance_0(inout DiffPair_Material2_0 _S8, ShadingInput_0 _S9, MaterialInstance2_0 _S10)
{
    s_bwd_prop_Material2_setupMaterialInstance_0(_S8, _S9, _S10);
    return;
}

AnyValue40 packAnyValue40_0(DiffPair_Material2_0 _S11)
{
    AnyValue40 _S12;
    _S12.field0_0 = 0U;
    _S12.field1_0 = 0U;
    _S12.field2_0 = 0U;
    _S12.field3_0 = 0U;
    _S12.field4_0 = 0U;
    _S12.field5_0 = 0U;
    _S12.field6_0 = 0U;
    _S12.field7_0 = 0U;
    _S12.field8_0 = 0U;
    _S12.field9_0 = 0U;
    _S12.field0_0 = _S11.primal_0.header_2.header_0[int(0)];
    _S12.field1_0 = _S11.primal_0.header_2.header_0[int(1)];
    _S12.field2_0 = _S11.primal_0.header_2.header_0[int(2)];
    _S12.field3_0 = _S11.primal_0.header_2.header_0[int(3)];
    _S12.field4_0 = (uint)(asuint(_S11.primal_0.a_0));
    _S12.field5_0 = (uint)(asuint(_S11.primal_0.b_0));
    _S12.field6_0 = (uint)(asuint(_S11.differential_0.a_1));
    _S12.field7_0 = (uint)(asuint(_S11.differential_0.b_1));
    return _S12;
}

void s_bwd_Material2_setupMaterialInstance_wtwrapper_0(inout AnyValue40 _S13, ShadingInput_0 _S14, AnyValue28 _S15)
{
    DiffPair_Material2_0 _S16 = unpackAnyValue40_0(_S13);
    s_bwd_Material2_setupMaterialInstance_0(_S16, _S14, unpackAnyValue28_0(_S15));
    _S13 = packAnyValue40_0(_S16);
    return;
}

struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};

struct DiffPair_MaterialInstance2_0
{
    MaterialInstance2_0 primal_0;
    MaterialInstance2_0 differential_0;
};

DiffPair_MaterialInstance2_0 unpackAnyValue40_1(AnyValue40 _S17)
{
    DiffPair_MaterialInstance2_0 _S18;
    _S18.primal_0.a_2 = asfloat(_S17.field0_0);
    _S18.primal_0.b_2 = asfloat(_S17.field1_0);
    _S18.differential_0.a_2 = asfloat(_S17.field2_0);
    _S18.differential_0.b_2 = asfloat(_S17.field3_0);
    return _S18;
}

MaterialInstance2_0 MaterialInstance2_x24_syn_dzero_0()
{
    MaterialInstance2_0 result_1;
    result_1.a_2 = 0.0;
    result_1.b_2 = 0.0;
    return result_1;
}

void s_bwd_prop_MaterialInstance2_eval_0(inout DiffPair_MaterialInstance2_0 dpthis_1, inout DiffPair_float_0 dpx_0, float _s_dOut_1)
{
    float _S19 = dpthis_1.primal_0.a_2 * _s_dOut_1;
    float _S20 = dpx_0.primal_0 * _s_dOut_1;
    dpx_0.primal_0 = dpx_0.primal_0;
    dpx_0.differential_0 = _S19;
    MaterialInstance2_0 _S21 = MaterialInstance2_x24_syn_dzero_0();
    _S21.b_2 = _s_dOut_1;
    _S21.a_2 = _S20;
    dpthis_1.primal_0 = dpthis_1.primal_0;
    dpthis_1.differential_0 = _S21;
    return;
}

void s_bwd_MaterialInstance2_eval_0(inout DiffPair_MaterialInstance2_0 _S22, inout DiffPair_float_0 _S23, float _S24)
{
    s_bwd_prop_MaterialInstance2_eval_0(_S22, _S23, _S24);
    return;
}

AnyValue40 packAnyValue40_1(DiffPair_MaterialInstance2_0 _S25)
{
    AnyValue40 _S26;
    _S26.field0_0 = 0U;
    _S26.field1_0 = 0U;
    _S26.field2_0 = 0U;
    _S26.field3_0 = 0U;
    _S26.field4_0 = 0U;
    _S26.field5_0 = 0U;
    _S26.field6_0 = 0U;
    _S26.field7_0 = 0U;
    _S26.field8_0 = 0U;
    _S26.field9_0 = 0U;
    _S26.field0_0 = (uint)(asuint(_S25.primal_0.a_2));
    _S26.field1_0 = (uint)(asuint(_S25.primal_0.b_2));
    _S26.field2_0 = (uint)(asuint(_S25.differential_0.a_2));
    _S26.field3_0 = (uint)(asuint(_S25.differential_0.b_2));
    return _S26;
}

void s_bwd_MaterialInstance2_eval_wtwrapper_0(inout AnyValue40 _S27, inout DiffPair_float_0 _S28, float _S29)
{
    DiffPair_MaterialInstance2_0 _S30 = unpackAnyValue40_1(_S27);
    s_bwd_MaterialInstance2_eval_0(_S30, _S28, _S29);
    _S27 = packAnyValue40_1(_S30);
    return;
}

struct Material1_0
{
    MaterialHeader_0 header_3;
    float a_3;
    float b_3;
    float c_0;
};

struct Material1_Differential_0
{
    float a_4;
    float b_4;
    float c_1;
};

struct DiffPair_Material1_0
{
    Material1_0 primal_0;
    Material1_Differential_0 differential_0;
};

DiffPair_Material1_0 unpackAnyValue40_2(AnyValue40 _S31)
{
    DiffPair_Material1_0 _S32;
    _S32.primal_0.header_3.header_0[int(0)] = _S31.field0_0;
    _S32.primal_0.header_3.header_0[int(1)] = _S31.field1_0;
    _S32.primal_0.header_3.header_0[int(2)] = _S31.field2_0;
    _S32.primal_0.header_3.header_0[int(3)] = _S31.field3_0;
    _S32.primal_0.a_3 = asfloat(_S31.field4_0);
    _S32.primal_0.b_3 = asfloat(_S31.field5_0);
    _S32.primal_0.c_0 = asfloat(_S31.field6_0);
    _S32.differential_0.a_4 = asfloat(_S31.field7_0);
    _S32.differential_0.b_4 = asfloat(_S31.field8_0);
    _S32.differential_0.c_1 = asfloat(_S31.field9_0);
    return _S32;
}

struct MaterialInstance1_0
{
    float a_5;
    float b_5;
    float c_2;
};

MaterialInstance1_0 unpackAnyValue28_1(AnyValue28 _S33)
{
    MaterialInstance1_0 _S34;
    _S34.a_5 = asfloat(_S33.field0_1);
    _S34.b_5 = asfloat(_S33.field1_1);
    _S34.c_2 = asfloat(_S33.field2_1);
    return _S34;
}

Material1_Differential_0 Material1_x24_syn_dzero_0()
{
    Material1_Differential_0 result_2;
    result_2.a_4 = 0.0;
    result_2.b_4 = 0.0;
    result_2.c_1 = 0.0;
    return result_2;
}

void s_bwd_prop_Material1_setupMaterialInstance_0(inout DiffPair_Material1_0 dpthis_2, ShadingInput_0 input_1, MaterialInstance1_0 _s_dOut_2)
{
    float _S35 = input_1.scale_0 * _s_dOut_2.c_2;
    float _S36 = input_1.scale_0 * _s_dOut_2.b_5;
    float _S37 = input_1.scale_0 * _s_dOut_2.a_5;
    Material1_Differential_0 _S38 = Material1_x24_syn_dzero_0();
    _S38.c_1 = _S35;
    _S38.b_4 = _S36;
    _S38.a_4 = _S37;
    dpthis_2.primal_0 = dpthis_2.primal_0;
    dpthis_2.differential_0 = _S38;
    return;
}

void s_bwd_Material1_setupMaterialInstance_0(inout DiffPair_Material1_0 _S39, ShadingInput_0 _S40, MaterialInstance1_0 _S41)
{
    s_bwd_prop_Material1_setupMaterialInstance_0(_S39, _S40, _S41);
    return;
}

AnyValue40 packAnyValue40_2(DiffPair_Material1_0 _S42)
{
    AnyValue40 _S43;
    _S43.field0_0 = 0U;
    _S43.field1_0 = 0U;
    _S43.field2_0 = 0U;
    _S43.field3_0 = 0U;
    _S43.field4_0 = 0U;
    _S43.field5_0 = 0U;
    _S43.field6_0 = 0U;
    _S43.field7_0 = 0U;
    _S43.field8_0 = 0U;
    _S43.field9_0 = 0U;
    _S43.field0_0 = _S42.primal_0.header_3.header_0[int(0)];
    _S43.field1_0 = _S42.primal_0.header_3.header_0[int(1)];
    _S43.field2_0 = _S42.primal_0.header_3.header_0[int(2)];
    _S43.field3_0 = _S42.primal_0.header_3.header_0[int(3)];
    _S43.field4_0 = (uint)(asuint(_S42.primal_0.a_3));
    _S43.field5_0 = (uint)(asuint(_S42.primal_0.b_3));
    _S43.field6_0 = (uint)(asuint(_S42.primal_0.c_0));
    _S43.field7_0 = (uint)(asuint(_S42.differential_0.a_4));
    _S43.field8_0 = (uint)(asuint(_S42.differential_0.b_4));
    _S43.field9_0 = (uint)(asuint(_S42.differential_0.c_1));
    return _S43;
}

void s_bwd_Material1_setupMaterialInstance_wtwrapper_0(inout AnyValue40 _S44, ShadingInput_0 _S45, AnyValue28 _S46)
{
    DiffPair_Material1_0 _S47 = unpackAnyValue40_2(_S44);
    s_bwd_Material1_setupMaterialInstance_0(_S47, _S45, unpackAnyValue28_1(_S46));
    _S44 = packAnyValue40_2(_S47);
    return;
}

struct DiffPair_MaterialInstance1_0
{
    MaterialInstance1_0 primal_0;
    MaterialInstance1_0 differential_0;
};

DiffPair_MaterialInstance1_0 unpackAnyValue40_3(AnyValue40 _S48)
{
    DiffPair_MaterialInstance1_0 _S49;
    _S49.primal_0.a_5 = asfloat(_S48.field0_0);
    _S49.primal_0.b_5 = asfloat(_S48.field1_0);
    _S49.primal_0.c_2 = asfloat(_S48.field2_0);
    _S49.differential_0.a_5 = asfloat(_S48.field3_0);
    _S49.differential_0.b_5 = asfloat(_S48.field4_0);
    _S49.differential_0.c_2 = asfloat(_S48.field5_0);
    return _S49;
}

MaterialInstance1_0 MaterialInstance1_x24_syn_dzero_0()
{
    MaterialInstance1_0 result_3;
    result_3.a_5 = 0.0;
    result_3.b_5 = 0.0;
    result_3.c_2 = 0.0;
    return result_3;
}

void s_bwd_prop_MaterialInstance1_eval_0(inout DiffPair_MaterialInstance1_0 dpthis_3, inout DiffPair_float_0 dpx_1, float _s_dOut_3)
{
    float _S50 = dpx_1.primal_0 * _s_dOut_3;
    float _S51 = dpx_1.primal_0 * _S50;
    float _S52 = dpthis_3.primal_0.b_5 * _s_dOut_3 + dpthis_3.primal_0.a_5 * dpx_1.primal_0 * _s_dOut_3 + dpthis_3.primal_0.a_5 * _S50;
    dpx_1.primal_0 = dpx_1.primal_0;
    dpx_1.differential_0 = _S52;
    MaterialInstance1_0 _S53 = MaterialInstance1_x24_syn_dzero_0();
    _S53.c_2 = _s_dOut_3;
    _S53.b_5 = _S50;
    _S53.a_5 = _S51;
    dpthis_3.primal_0 = dpthis_3.primal_0;
    dpthis_3.differential_0 = _S53;
    return;
}

void s_bwd_MaterialInstance1_eval_0(inout DiffPair_MaterialInstance1_0 _S54, inout DiffPair_float_0 _S55, float _S56)
{
    s_bwd_prop_MaterialInstance1_eval_0(_S54, _S55, _S56);
    return;
}

AnyValue40 packAnyValue40_3(DiffPair_MaterialInstance1_0 _S57)
{
    AnyValue40 _S58;
    _S58.field0_0 = 0U;
    _S58.field1_0 = 0U;
    _S58.field2_0 = 0U;
    _S58.field3_0 = 0U;
    _S58.field4_0 = 0U;
    _S58.field5_0 = 0U;
    _S58.field6_0 = 0U;
    _S58.field7_0 = 0U;
    _S58.field8_0 = 0U;
    _S58.field9_0 = 0U;
    _S58.field0_0 = (uint)(asuint(_S57.primal_0.a_5));
    _S58.field1_0 = (uint)(asuint(_S57.primal_0.b_5));
    _S58.field2_0 = (uint)(asuint(_S57.primal_0.c_2));
    _S58.field3_0 = (uint)(asuint(_S57.differential_0.a_5));
    _S58.field4_0 = (uint)(asuint(_S57.differential_0.b_5));
    _S58.field5_0 = (uint)(asuint(_S57.differential_0.c_2));
    return _S58;
}

void s_bwd_MaterialInstance1_eval_wtwrapper_0(inout AnyValue40 _S59, inout DiffPair_float_0 _S60, float _S61)
{
    DiffPair_MaterialInstance1_0 _S62 = unpackAnyValue40_3(_S59);
    s_bwd_MaterialInstance1_eval_0(_S62, _S60, _S61);
    _S59 = packAnyValue40_3(_S62);
    return;
}

uint2 U_SR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstanceI4core15IDifferentiable_0(uint2 _S63)
{
    switch(_S63.x)
    {
    case 0U:
        {
            return uint2(5U, 0U);
        }
    default:
        {
            return uint2(2U, 0U);
        }
    }
}

struct Tuple_0
{
    uint2 value0_0;
    uint2 value1_0;
    AnyValue28 value2_0;
};

void getMaterial_bwd_0(int id_0, Tuple_0 d_0)
{
    outputBuffer_0[id_0] = 2.0;
    return;
}

AnyValue28 packAnyValue28_0(MaterialDataBlob_0 _S64)
{
    AnyValue28 _S65;
    _S65.field0_1 = 0U;
    _S65.field1_1 = 0U;
    _S65.field2_1 = 0U;
    _S65.field3_1 = 0U;
    _S65.field4_1 = 0U;
    _S65.field5_1 = 0U;
    _S65.field6_1 = 0U;
    _S65.field0_1 = _S64.header_1.header_0[int(0)];
    _S65.field1_1 = _S64.header_1.header_0[int(1)];
    _S65.field2_1 = _S64.header_1.header_0[int(2)];
    _S65.field3_1 = _S64.header_1.header_0[int(3)];
    _S65.field4_1 = (uint)(asuint(_S64.payload_0.data_0[int(0)]));
    _S65.field5_1 = (uint)(asuint(_S64.payload_0.data_0[int(1)]));
    _S65.field6_1 = (uint)(asuint(_S64.payload_0.data_0[int(2)]));
    return _S65;
}

Tuple_0 getMaterial_0(int id_1)
{
    Tuple_0 _S66 = { uint2(0U, 0U), uint2(uint(id_1), 0U), packAnyValue28_0(g_materials_0[id_1]) };
    return _S66;
}

struct AnyValue12
{
    uint field0_2;
    uint field1_2;
    uint field2_2;
};

Material1_0 unpackAnyValue28_2(AnyValue28 _S67)
{
    Material1_0 _S68;
    _S68.header_3.header_0[int(0)] = _S67.field0_1;
    _S68.header_3.header_0[int(1)] = _S67.field1_1;
    _S68.header_3.header_0[int(2)] = _S67.field2_1;
    _S68.header_3.header_0[int(3)] = _S67.field3_1;
    _S68.a_3 = asfloat(_S67.field4_1);
    _S68.b_3 = asfloat(_S67.field5_1);
    _S68.c_0 = asfloat(_S67.field6_1);
    return _S68;
}

MaterialInstance1_0 Material1_setupMaterialInstance_0(Material1_0 this_0, ShadingInput_0 input_2)
{
    MaterialInstance1_0 instance_0;
    float _S69 = this_0.a_3 * input_2.scale_0;
    instance_0.a_5 = _S69;
    float _S70 = this_0.b_3 * input_2.scale_0;
    instance_0.b_5 = _S70;
    float _S71 = this_0.c_0 * input_2.scale_0;
    instance_0.c_2 = _S71;
    return instance_0;
}

AnyValue12 packAnyValue12_0(MaterialInstance1_0 _S72)
{
    AnyValue12 _S73;
    _S73.field0_2 = 0U;
    _S73.field1_2 = 0U;
    _S73.field2_2 = 0U;
    _S73.field0_2 = (uint)(asuint(_S72.a_5));
    _S73.field1_2 = (uint)(asuint(_S72.b_5));
    _S73.field2_2 = (uint)(asuint(_S72.c_2));
    return _S73;
}

Material2_0 unpackAnyValue28_3(AnyValue28 _S74)
{
    Material2_0 _S75;
    _S75.header_2.header_0[int(0)] = _S74.field0_1;
    _S75.header_2.header_0[int(1)] = _S74.field1_1;
    _S75.header_2.header_0[int(2)] = _S74.field2_1;
    _S75.header_2.header_0[int(3)] = _S74.field3_1;
    _S75.a_0 = asfloat(_S74.field4_1);
    _S75.b_0 = asfloat(_S74.field5_1);
    return _S75;
}

MaterialInstance2_0 Material2_setupMaterialInstance_0(Material2_0 this_1, ShadingInput_0 input_3)
{
    MaterialInstance2_0 instance_1;
    float _S76 = this_1.a_0 * input_3.scale_0 * input_3.scale_0;
    instance_1.a_2 = _S76;
    float _S77 = this_1.b_0 * input_3.scale_0 * input_3.scale_0;
    instance_1.b_2 = _S77;
    return instance_1;
}

AnyValue12 packAnyValue12_1(MaterialInstance2_0 _S78)
{
    AnyValue12 _S79;
    _S79.field0_2 = 0U;
    _S79.field1_2 = 0U;
    _S79.field2_2 = 0U;
    _S79.field0_2 = (uint)(asuint(_S78.a_2));
    _S79.field1_2 = (uint)(asuint(_S78.b_2));
    return _S79;
}

AnyValue12 _S80(Tuple_0 _S81, ShadingInput_0 _S82)
{
    switch(_S81.value1_0.x)
    {
    case 0U:
        {
            MaterialInstance1_0 _S83 = Material1_setupMaterialInstance_0(unpackAnyValue28_2(_S81.value2_0), _S82);
            return packAnyValue12_0(_S83);
        }
    default:
        {
            MaterialInstance2_0 _S84 = Material2_setupMaterialInstance_0(unpackAnyValue28_3(_S81.value2_0), _S82);
            return packAnyValue12_1(_S84);
        }
    }
}

uint2 U_SR31dynamic_2Dxdispatch_2Dxmaterial9IMaterial16MaterialInstanceIR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance_0(uint2 _S85)
{
    switch(_S85.x)
    {
    case 0U:
        {
            return uint2(0U, 0U);
        }
    default:
        {
            return uint2(1U, 0U);
        }
    }
}

MaterialInstance1_0 unpackAnyValue12_0(AnyValue12 _S86)
{
    MaterialInstance1_0 _S87;
    _S87.a_5 = asfloat(_S86.field0_2);
    _S87.b_5 = asfloat(_S86.field1_2);
    _S87.c_2 = asfloat(_S86.field2_2);
    return _S87;
}

float MaterialInstance1_eval_0(MaterialInstance1_0 this_2, float x_0)
{
    return this_2.a_5 * x_0 * x_0 + this_2.b_5 * x_0 + this_2.c_2;
}

float U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance14evalp1pi_ffb_wtwrapper_0(AnyValue12 _S88, float _S89)
{
    float _S90 = MaterialInstance1_eval_0(unpackAnyValue12_0(_S88), _S89);
    return _S90;
}

MaterialInstance2_0 unpackAnyValue12_1(AnyValue12 _S91)
{
    MaterialInstance2_0 _S92;
    _S92.a_2 = asfloat(_S91.field0_2);
    _S92.b_2 = asfloat(_S91.field1_2);
    return _S92;
}

float MaterialInstance2_eval_0(MaterialInstance2_0 this_3, float x_1)
{
    return this_3.a_2 * x_1 + this_3.b_2;
}

float U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance24evalp1pi_ffb_wtwrapper_0(AnyValue12 _S93, float _S94)
{
    float _S95 = MaterialInstance2_eval_0(unpackAnyValue12_1(_S93), _S94);
    return _S95;
}

float U_SR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance4evalp1pi_ffb_0(uint2 _S96, AnyValue12 _S97, float _S98)
{
    switch(_S96.x)
    {
    case 0U:
        {
            float _S99 = U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance14evalp1pi_ffb_wtwrapper_0(_S97, _S98);
            return _S99;
        }
    default:
        {
            float _S100 = U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance24evalp1pi_ffb_wtwrapper_0(_S97, _S98);
            return _S100;
        }
    }
}

float shade_0(int material_0, ShadingInput_0 input_4, float x_2)
{
    Tuple_0 m_0 = getMaterial_0(material_0);
    uint2 _S101 = U_SR31dynamic_2Dxdispatch_2Dxmaterial9IMaterial16MaterialInstanceIR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance_0(m_0.value1_0);
    AnyValue12 _S102 = _S80(m_0, input_4);
    float _S103 = U_SR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance4evalp1pi_ffb_0(_S101, _S102, x_2);
    return _S103;
}

struct s_bwd_prop_shade_Intermediates_0
{
    Tuple_0 _S104;
};

Tuple_0 s_primal_ctx_getMaterial_0(int _S105)
{
    Tuple_0 _S106 = getMaterial_0(_S105);
    return _S106;
}

float s_primal_ctx_shade_0(int material_1, ShadingInput_0 input_5, float dpx_2, out s_bwd_prop_shade_Intermediates_0 _s_diff_ctx_0)
{
    uint2 _S107 = (uint2)0U;
    AnyValue28 _S108 = { 0U, 0U, 0U, 0U, 0U, 0U, 0U };
    Tuple_0 _S109 = { _S107, _S107, _S108 };
    _s_diff_ctx_0._S104 = _S109;
    _s_diff_ctx_0._S104.value0_0 = _S107;
    _s_diff_ctx_0._S104.value1_0 = _S107;
    _s_diff_ctx_0._S104.value2_0 = _S108;
    Tuple_0 _S110 = s_primal_ctx_getMaterial_0(material_1);
    uint2 _S111 = U_SR31dynamic_2Dxdispatch_2Dxmaterial9IMaterial16MaterialInstanceIR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance_0(_S110.value1_0);
    _s_diff_ctx_0._S104 = _S110;
    AnyValue12 _S112 = _S80(_S110, input_5);
    float _S113 = U_SR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance4evalp1pi_ffb_0(_S111, _S112, dpx_2);
    return _S113;
}

Material2_Differential_0 Material2_Differential_x24_syn_dzero_0()
{
    Material2_Differential_0 result_4;
    result_4.a_1 = 0.0;
    result_4.b_1 = 0.0;
    return result_4;
}

AnyValue28 packAnyValue28_1(Material2_Differential_0 _S114)
{
    AnyValue28 _S115;
    _S115.field0_1 = 0U;
    _S115.field1_1 = 0U;
    _S115.field2_1 = 0U;
    _S115.field3_1 = 0U;
    _S115.field4_1 = 0U;
    _S115.field5_1 = 0U;
    _S115.field6_1 = 0U;
    _S115.field0_1 = (uint)(asuint(_S114.a_1));
    _S115.field1_1 = (uint)(asuint(_S114.b_1));
    return _S115;
}

AnyValue28 U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material212DifferentialR18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material212Differentialb_wtwrapper_0()
{
    return packAnyValue28_1(Material2_Differential_x24_syn_dzero_0());
}

AnyValue28 U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material2R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material212Differentialb_wtwrapper_0()
{
    return packAnyValue28_1(Material2_x24_syn_dzero_0());
}

AnyValue28 packAnyValue28_2(MaterialInstance2_0 _S116)
{
    AnyValue28 _S117;
    _S117.field0_1 = 0U;
    _S117.field1_1 = 0U;
    _S117.field2_1 = 0U;
    _S117.field3_1 = 0U;
    _S117.field4_1 = 0U;
    _S117.field5_1 = 0U;
    _S117.field6_1 = 0U;
    _S117.field0_1 = (uint)(asuint(_S116.a_2));
    _S117.field1_1 = (uint)(asuint(_S116.b_2));
    return _S117;
}

AnyValue28 U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance2R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance2b_wtwrapper_0()
{
    return packAnyValue28_2(MaterialInstance2_x24_syn_dzero_0());
}

Material1_Differential_0 Material1_Differential_x24_syn_dzero_0()
{
    Material1_Differential_0 result_5;
    result_5.a_4 = 0.0;
    result_5.b_4 = 0.0;
    result_5.c_1 = 0.0;
    return result_5;
}

AnyValue28 packAnyValue28_3(Material1_Differential_0 _S118)
{
    AnyValue28 _S119;
    _S119.field0_1 = 0U;
    _S119.field1_1 = 0U;
    _S119.field2_1 = 0U;
    _S119.field3_1 = 0U;
    _S119.field4_1 = 0U;
    _S119.field5_1 = 0U;
    _S119.field6_1 = 0U;
    _S119.field0_1 = (uint)(asuint(_S118.a_4));
    _S119.field1_1 = (uint)(asuint(_S118.b_4));
    _S119.field2_1 = (uint)(asuint(_S118.c_1));
    return _S119;
}

AnyValue28 U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material112DifferentialR18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material112Differentialb_wtwrapper_0()
{
    return packAnyValue28_3(Material1_Differential_x24_syn_dzero_0());
}

AnyValue28 U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material1R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material112Differentialb_wtwrapper_0()
{
    return packAnyValue28_3(Material1_x24_syn_dzero_0());
}

AnyValue28 packAnyValue28_4(MaterialInstance1_0 _S120)
{
    AnyValue28 _S121;
    _S121.field0_1 = 0U;
    _S121.field1_1 = 0U;
    _S121.field2_1 = 0U;
    _S121.field3_1 = 0U;
    _S121.field4_1 = 0U;
    _S121.field5_1 = 0U;
    _S121.field6_1 = 0U;
    _S121.field0_1 = (uint)(asuint(_S120.a_5));
    _S121.field1_1 = (uint)(asuint(_S120.b_5));
    _S121.field2_1 = (uint)(asuint(_S120.c_2));
    return _S121;
}

AnyValue28 U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance1R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance1b_wtwrapper_0()
{
    return packAnyValue28_4(MaterialInstance1_x24_syn_dzero_0());
}

AnyValue28 U_S4core15IDifferentiable5dzerop0p4core15IDifferentiable12Differential_0(uint2 _S122)
{
    switch(_S122.x)
    {
    case 0U:
        {
            return U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material212DifferentialR18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material212Differentialb_wtwrapper_0();
        }
    case 1U:
        {
            return U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material2R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material212Differentialb_wtwrapper_0();
        }
    case 2U:
        {
            return U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance2R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance2b_wtwrapper_0();
        }
    case 3U:
        {
            return U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material112DifferentialR18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material112Differentialb_wtwrapper_0();
        }
    case 4U:
        {
            return U_SR31dynamic_2Dxdispatch_2Dxmaterial9Material1R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial9Material112Differentialb_wtwrapper_0();
        }
    default:
        {
            return U_SR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance1R18_24x_u_usyn_udzerop0pR31dynamic_2Dxdispatch_2Dxmaterial17MaterialInstance1b_wtwrapper_0();
        }
    }
}

DiffPair_MaterialInstance1_0 DiffPair_MaterialInstance1_makePair_0(MaterialInstance1_0 _S123, MaterialInstance1_0 _S124)
{
    DiffPair_MaterialInstance1_0 _S125 = { _S123, _S124 };
    return _S125;
}

AnyValue40 DiffPair_MaterialInstance1_makePair_wtwrapper_0(AnyValue12 _S126, AnyValue28 _S127)
{
    return packAnyValue40_3(DiffPair_MaterialInstance1_makePair_0(unpackAnyValue12_0(_S126), unpackAnyValue28_1(_S127)));
}

DiffPair_MaterialInstance2_0 DiffPair_MaterialInstance2_makePair_0(MaterialInstance2_0 _S128, MaterialInstance2_0 _S129)
{
    DiffPair_MaterialInstance2_0 _S130 = { _S128, _S129 };
    return _S130;
}

AnyValue40 DiffPair_MaterialInstance2_makePair_wtwrapper_0(AnyValue12 _S131, AnyValue28 _S132)
{
    return packAnyValue40_1(DiffPair_MaterialInstance2_makePair_0(unpackAnyValue12_1(_S131), unpackAnyValue28_0(_S132)));
}

AnyValue40 _S133(uint2 _S134, AnyValue12 _S135, AnyValue28 _S136)
{
    switch(_S134.x)
    {
    case 0U:
        {
            return DiffPair_MaterialInstance1_makePair_wtwrapper_0(_S135, _S136);
        }
    default:
        {
            return DiffPair_MaterialInstance2_makePair_wtwrapper_0(_S135, _S136);
        }
    }
}

void U_SBwdReq_R31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance4evalp1pi_ffb_0(uint2 _S137, inout AnyValue40 _S138, inout DiffPair_float_0 _S139, float _S140)
{
    switch(_S137.x)
    {
    case 0U:
        {
            s_bwd_MaterialInstance1_eval_wtwrapper_0(_S138, _S139, _S140);
            return;
        }
    default:
        {
            s_bwd_MaterialInstance2_eval_wtwrapper_0(_S138, _S139, _S140);
            return;
        }
    }
}

MaterialInstance1_0 DiffPair_MaterialInstance1_getDiff_0(DiffPair_MaterialInstance1_0 _S141)
{
    return _S141.differential_0;
}

AnyValue28 DiffPair_MaterialInstance1_getDiff_wtwrapper_0(AnyValue40 _S142)
{
    return packAnyValue28_4(DiffPair_MaterialInstance1_getDiff_0(unpackAnyValue40_3(_S142)));
}

MaterialInstance2_0 DiffPair_MaterialInstance2_getDiff_0(DiffPair_MaterialInstance2_0 _S143)
{
    return _S143.differential_0;
}

AnyValue28 DiffPair_MaterialInstance2_getDiff_wtwrapper_0(AnyValue40 _S144)
{
    return packAnyValue28_2(DiffPair_MaterialInstance2_getDiff_0(unpackAnyValue40_1(_S144)));
}

AnyValue28 _S145(uint2 _S146, AnyValue40 _S147)
{
    switch(_S146.x)
    {
    case 0U:
        {
            return DiffPair_MaterialInstance1_getDiff_wtwrapper_0(_S147);
        }
    default:
        {
            return DiffPair_MaterialInstance2_getDiff_wtwrapper_0(_S147);
        }
    }
}

uint2 U_SR31dynamic_2Dxdispatch_2Dxmaterial9IMaterialI4core15IDifferentiable_0(uint2 _S148)
{
    switch(_S148.x)
    {
    case 0U:
        {
            return uint2(4U, 0U);
        }
    default:
        {
            return uint2(1U, 0U);
        }
    }
}

Material1_Differential_0 unpackAnyValue28_4(AnyValue28 _S149)
{
    Material1_Differential_0 _S150;
    _S150.a_4 = asfloat(_S149.field0_1);
    _S150.b_4 = asfloat(_S149.field1_1);
    _S150.c_1 = asfloat(_S149.field2_1);
    return _S150;
}

DiffPair_Material1_0 DiffPair_Material1_makePair_0(Material1_0 _S151, Material1_Differential_0 _S152)
{
    DiffPair_Material1_0 _S153 = { _S151, _S152 };
    return _S153;
}

AnyValue40 DiffPair_Material1_makePair_wtwrapper_0(AnyValue28 _S154, AnyValue28 _S155)
{
    return packAnyValue40_2(DiffPair_Material1_makePair_0(unpackAnyValue28_2(_S154), unpackAnyValue28_4(_S155)));
}

Material2_Differential_0 unpackAnyValue28_5(AnyValue28 _S156)
{
    Material2_Differential_0 _S157;
    _S157.a_1 = asfloat(_S156.field0_1);
    _S157.b_1 = asfloat(_S156.field1_1);
    return _S157;
}

DiffPair_Material2_0 DiffPair_Material2_makePair_0(Material2_0 _S158, Material2_Differential_0 _S159)
{
    DiffPair_Material2_0 _S160 = { _S158, _S159 };
    return _S160;
}

AnyValue40 DiffPair_Material2_makePair_wtwrapper_0(AnyValue28 _S161, AnyValue28 _S162)
{
    return packAnyValue40_0(DiffPair_Material2_makePair_0(unpackAnyValue28_3(_S161), unpackAnyValue28_5(_S162)));
}

AnyValue40 _S163(uint2 _S164, AnyValue28 _S165, AnyValue28 _S166)
{
    switch(_S164.x)
    {
    case 0U:
        {
            return DiffPair_Material1_makePair_wtwrapper_0(_S165, _S166);
        }
    default:
        {
            return DiffPair_Material2_makePair_wtwrapper_0(_S165, _S166);
        }
    }
}

void U_SBwdReq_R31dynamic_2Dxdispatch_2Dxmaterial9IMaterial21setupMaterialInstancep1pi_R31dynamic_2Dxdispatch_2Dxmaterial12ShadingInputR31dynamic_2Dxdispatch_2Dxmaterial9IMaterial16MaterialInstanceb_0(uint2 _S167, inout AnyValue40 _S168, ShadingInput_0 _S169, AnyValue28 _S170)
{
    switch(_S167.x)
    {
    case 0U:
        {
            s_bwd_Material1_setupMaterialInstance_wtwrapper_0(_S168, _S169, _S170);
            return;
        }
    default:
        {
            s_bwd_Material2_setupMaterialInstance_wtwrapper_0(_S168, _S169, _S170);
            return;
        }
    }
}

Material1_Differential_0 DiffPair_Material1_getDiff_0(DiffPair_Material1_0 _S171)
{
    return _S171.differential_0;
}

AnyValue28 DiffPair_Material1_getDiff_wtwrapper_0(AnyValue40 _S172)
{
    return packAnyValue28_3(DiffPair_Material1_getDiff_0(unpackAnyValue40_2(_S172)));
}

Material2_Differential_0 DiffPair_Material2_getDiff_0(DiffPair_Material2_0 _S173)
{
    return _S173.differential_0;
}

AnyValue28 DiffPair_Material2_getDiff_wtwrapper_0(AnyValue40 _S174)
{
    return packAnyValue28_1(DiffPair_Material2_getDiff_0(unpackAnyValue40_0(_S174)));
}

AnyValue28 _S175(uint2 _S176, AnyValue40 _S177)
{
    switch(_S176.x)
    {
    case 0U:
        {
            return DiffPair_Material1_getDiff_wtwrapper_0(_S177);
        }
    default:
        {
            return DiffPair_Material2_getDiff_wtwrapper_0(_S177);
        }
    }
}

void s_bwd_prop_getMaterial_0(int _S178, Tuple_0 _S179)
{
    getMaterial_bwd_0(_S178, _S179);
    return;
}

void s_bwd_prop_shade_0(int material_2, ShadingInput_0 input_6, inout DiffPair_float_0 dpx_3, float _s_dOut_4, s_bwd_prop_shade_Intermediates_0 _s_diff_ctx_1)
{
    AnyValue12 _S180 = _S80(_s_diff_ctx_1._S104, input_6);
    uint2 _S181 = U_SR31dynamic_2Dxdispatch_2Dxmaterial9IMaterial16MaterialInstanceIR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance_0(_s_diff_ctx_1._S104.value1_0);
    AnyValue40 _S182 = _S133(_s_diff_ctx_1._S104.value1_0, _S180, U_S4core15IDifferentiable5dzerop0p4core15IDifferentiable12Differential_0(U_SR31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstanceI4core15IDifferentiable_0(_S181)));
    DiffPair_float_0 _S183;
    _S183.primal_0 = dpx_3.primal_0;
    _S183.differential_0 = 0.0;
    U_SBwdReq_R31dynamic_2Dxdispatch_2Dxmaterial17IMaterialInstance4evalp1pi_ffb_0(_S181, _S182, _S183, _s_dOut_4);
    AnyValue28 _S184 = _S145(_s_diff_ctx_1._S104.value1_0, _S182);
    uint2 _S185 = U_SR31dynamic_2Dxdispatch_2Dxmaterial9IMaterialI4core15IDifferentiable_0(_s_diff_ctx_1._S104.value1_0);
    AnyValue40 _S186 = _S163(_s_diff_ctx_1._S104.value1_0, _s_diff_ctx_1._S104.value2_0, U_S4core15IDifferentiable5dzerop0p4core15IDifferentiable12Differential_0(_S185));
    U_SBwdReq_R31dynamic_2Dxdispatch_2Dxmaterial9IMaterial21setupMaterialInstancep1pi_R31dynamic_2Dxdispatch_2Dxmaterial12ShadingInputR31dynamic_2Dxdispatch_2Dxmaterial9IMaterial16MaterialInstanceb_0(_s_diff_ctx_1._S104.value1_0, _S186, input_6, _S184);
    Tuple_0 _S187 = { uint2(0U, 0U), _S185, _S175(_s_diff_ctx_1._S104.value1_0, _S186) };
    s_bwd_prop_getMaterial_0(material_2, _S187);
    dpx_3.primal_0 = dpx_3.primal_0;
    dpx_3.differential_0 = _S183.differential_0;
    return;
}

void s_bwd_shade_0(int _S188, ShadingInput_0 _S189, inout DiffPair_float_0 _S190, float _S191)
{
    s_bwd_prop_shade_Intermediates_0 _S192;
    float _S193 = s_primal_ctx_shade_0(_S188, _S189, _S190.primal_0, _S192);
    s_bwd_prop_shade_0(_S188, _S189, _S190, _S191, _S192);
    return;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{
    ShadingInput_0 _S194 = { 0.5 };
    float _S195 = shade_0(int(0), _S194, 0.60000002384185791);
    outputBuffer_0[int(0)] = _S195;
    DiffPair_float_0 dpx_4;
    dpx_4.primal_0 = 3.0;
    dpx_4.differential_0 = 0.0;
    s_bwd_shade_0(int(0), _S194, dpx_4, 1.0);
    outputBuffer_0[int(3)] = dpx_4.differential_0;
    return;
}

