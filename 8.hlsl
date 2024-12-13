#pragma pack_matrix(row_major)
RWStructuredBuffer<float > outputBuffer_0 : register(u0);

struct AnyValue8
{
    uint field0_0;
    uint field1_0;
};

struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};

struct B_0
{
    int data1_0;
    int data2_0;
};

struct DiffPair_B_0
{
    B_0 primal_0;
};

DiffPair_B_0 unpackAnyValue8_0(AnyValue8 _S1)
{
    DiffPair_B_0 _S2;
    _S2.primal_0.data1_0 = (int)(_S1.field0_0);
    _S2.primal_0.data2_0 = (int)(_S1.field1_0);
    return _S2;
}

void s_bwd_prop_B_calc_0(inout DiffPair_B_0 dpthis_0, inout DiffPair_float_0 dpx_0, float _s_dOut_0)
{
    float _S3 = dpx_0.primal_0 * (float(dpthis_0.primal_0.data1_0) * (float(dpthis_0.primal_0.data2_0) * _s_dOut_0));
    float _S4 = _S3 + _S3;
    dpx_0.primal_0 = dpx_0.primal_0;
    dpx_0.differential_0 = _S4;
    dpthis_0.primal_0 = dpthis_0.primal_0;
    return;
}

void s_bwd_B_calc_0(inout DiffPair_B_0 _S5, inout DiffPair_float_0 _S6, float _S7)
{
    s_bwd_prop_B_calc_0(_S5, _S6, _S7);
    return;
}

AnyValue8 packAnyValue8_0(DiffPair_B_0 _S8)
{
    AnyValue8 _S9;
    _S9.field0_0 = 0U;
    _S9.field1_0 = 0U;
    _S9.field0_0 = (uint)(_S8.primal_0.data1_0);
    _S9.field1_0 = (uint)(_S8.primal_0.data2_0);
    return _S9;
}

void s_bwd_B_calc_wtwrapper_0(inout AnyValue8 _S10, inout DiffPair_float_0 _S11, float _S12)
{
    DiffPair_B_0 _S13 = unpackAnyValue8_0(_S10);
    s_bwd_B_calc_0(_S13, _S11, _S12);
    _S10 = packAnyValue8_0(_S13);
    return;
}

DiffPair_float_0 s_fwd_B_calc_0(DiffPair_B_0 dpthis_1, DiffPair_float_0 dpx_1)
{
    float _S14 = dpx_1.differential_0 * dpx_1.primal_0;
    float _S15 = float(dpthis_1.primal_0.data1_0);
    float _S16 = float(dpthis_1.primal_0.data2_0);
    DiffPair_float_0 _S17 = { dpx_1.primal_0 * dpx_1.primal_0 * _S15 * _S16, (_S14 + _S14) * _S15 * _S16 };
    return _S17;
}

DiffPair_float_0 s_fwd_B_calc_wtwrapper_0(AnyValue8 _S18, DiffPair_float_0 _S19)
{
    DiffPair_float_0 _S20 = s_fwd_B_calc_0(unpackAnyValue8_0(_S18), _S19);
    return _S20;
}

struct A_0
{
    int data1_1;
};

struct DiffPair_A_0
{
    A_0 primal_0;
};

DiffPair_A_0 unpackAnyValue8_1(AnyValue8 _S21)
{
    DiffPair_A_0 _S22;
    _S22.primal_0.data1_1 = (int)(_S21.field0_0);
    return _S22;
}

void s_bwd_prop_A_calc_0(inout DiffPair_A_0 dpthis_2, inout DiffPair_float_0 dpx_2, float _s_dOut_1)
{
    float _S23 = dpx_2.primal_0;
    float _S24 = float(dpthis_2.primal_0.data1_1) * _s_dOut_1;
    float _S25 = dpx_2.primal_0 * (dpx_2.primal_0 * _S24);
    float _S26 = _S23 * _S23 * _S24 + _S25 + _S25;
    dpx_2.primal_0 = dpx_2.primal_0;
    dpx_2.differential_0 = _S26;
    dpthis_2.primal_0 = dpthis_2.primal_0;
    return;
}

void s_bwd_A_calc_0(inout DiffPair_A_0 _S27, inout DiffPair_float_0 _S28, float _S29)
{
    s_bwd_prop_A_calc_0(_S27, _S28, _S29);
    return;
}

AnyValue8 packAnyValue8_1(DiffPair_A_0 _S30)
{
    AnyValue8 _S31;
    _S31.field0_0 = 0U;
    _S31.field1_0 = 0U;
    _S31.field0_0 = (uint)(_S30.primal_0.data1_1);
    return _S31;
}

void s_bwd_A_calc_wtwrapper_0(inout AnyValue8 _S32, inout DiffPair_float_0 _S33, float _S34)
{
    DiffPair_A_0 _S35 = unpackAnyValue8_1(_S32);
    s_bwd_A_calc_0(_S35, _S33, _S34);
    _S32 = packAnyValue8_1(_S35);
    return;
}

DiffPair_float_0 s_fwd_A_calc_0(DiffPair_A_0 dpthis_3, DiffPair_float_0 dpx_3)
{
    float _S36 = dpx_3.primal_0 * dpx_3.primal_0;
    float _S37 = dpx_3.differential_0 * dpx_3.primal_0;
    float _S38 = float(dpthis_3.primal_0.data1_1);
    DiffPair_float_0 _S39 = { _S36 * dpx_3.primal_0 * _S38, ((_S37 + _S37) * dpx_3.primal_0 + dpx_3.differential_0 * _S36) * _S38 };
    return _S39;
}

DiffPair_float_0 s_fwd_A_calc_wtwrapper_0(AnyValue8 _S40, DiffPair_float_0 _S41)
{
    DiffPair_float_0 _S42 = s_fwd_A_calc_0(unpackAnyValue8_1(_S40), _S41);
    return _S42;
}

struct AnyValue16
{
    uint field0_1;
    uint field1_1;
    uint field2_0;
    uint field3_0;
};

AnyValue16 packAnyValue16_0(A_0 _S43)
{
    AnyValue16 _S44;
    _S44.field0_1 = 0U;
    _S44.field1_1 = 0U;
    _S44.field2_0 = 0U;
    _S44.field3_0 = 0U;
    _S44.field0_1 = (uint)(_S43.data1_1);
    return _S44;
}

A_0 unpackAnyValue16_0(AnyValue16 _S45)
{
    A_0 _S46;
    _S46.data1_1 = (int)(_S45.field0_1);
    return _S46;
}

A_0 A_x24init_0(int data1_2)
{
    A_0 _S47;
    _S47.data1_1 = data1_2;
    return _S47;
}

AnyValue8 packAnyValue8_2()
{
    AnyValue8 _S48;
    _S48.field0_0 = 0U;
    _S48.field1_0 = 0U;
    return _S48;
}

AnyValue8 U_SR16existential_2Dx11A12DifferentialR18_24x_u_usyn_udzerop0pR16existential_2Dx11A12Differentialb_wtwrapper_0()
{
    return packAnyValue8_2();
}

AnyValue8 U_SR16existential_2Dx11AR18_24x_u_usyn_udzerop0pR16existential_2Dx11A12Differentialb_wtwrapper_0()
{
    return packAnyValue8_2();
}

float A_calc_0(A_0 this_0, float x_0)
{
    return x_0 * x_0 * x_0 * float(this_0.data1_1);
}

AnyValue16 packAnyValue16_1(B_0 _S49)
{
    AnyValue16 _S50;
    _S50.field0_1 = 0U;
    _S50.field1_1 = 0U;
    _S50.field2_0 = 0U;
    _S50.field3_0 = 0U;
    _S50.field0_1 = (uint)(_S49.data1_0);
    _S50.field1_1 = (uint)(_S49.data2_0);
    return _S50;
}

B_0 unpackAnyValue16_1(AnyValue16 _S51)
{
    B_0 _S52;
    _S52.data1_0 = (int)(_S51.field0_1);
    _S52.data2_0 = (int)(_S51.field1_1);
    return _S52;
}

B_0 B_x24init_0(int data1_3, int data2_1)
{
    B_0 _S53;
    _S53.data1_0 = data1_3;
    _S53.data2_0 = data2_1;
    return _S53;
}

AnyValue8 packAnyValue8_3()
{
    AnyValue8 _S54;
    _S54.field0_0 = 0U;
    _S54.field1_0 = 0U;
    return _S54;
}

AnyValue8 U_SR16existential_2Dx11B12DifferentialR18_24x_u_usyn_udzerop0pR16existential_2Dx11B12Differentialb_wtwrapper_0()
{
    return packAnyValue8_3();
}

AnyValue8 U_SR16existential_2Dx11BR18_24x_u_usyn_udzerop0pR16existential_2Dx11B12Differentialb_wtwrapper_0()
{
    return packAnyValue8_3();
}

float B_calc_0(B_0 this_1, float x_1)
{
    return x_1 * x_1 * float(this_1.data1_0) * float(this_1.data2_0);
}

struct Tuple_0
{
    uint2 value0_0;
    uint2 value1_0;
    AnyValue16 value2_0;
};

struct Tuple_1
{
    uint2 value0_1;
    uint2 value1_1;
    AnyValue8 value2_1;
};

struct NullDifferential_0
{
    uint dummy_0;
};

AnyValue8 packAnyValue8_4(NullDifferential_0 _S55)
{
    AnyValue8 _S56;
    _S56.field0_0 = 0U;
    _S56.field1_0 = 0U;
    _S56.field0_0 = _S55.dummy_0;
    return _S56;
}

struct DiffPair_IInterface_0
{
    Tuple_0 primal_0;
    Tuple_1 differential_0;
};

DiffPair_A_0 DiffPair_A_makePair_0(A_0 _S57)
{
    DiffPair_A_0 _S58 = { _S57 };
    return _S58;
}

AnyValue8 DiffPair_A_makePair_wtwrapper_0(AnyValue16 _S59, AnyValue8 _S60)
{
    return packAnyValue8_1(DiffPair_A_makePair_0(unpackAnyValue16_0(_S59)));
}

DiffPair_B_0 DiffPair_B_makePair_0(B_0 _S61)
{
    DiffPair_B_0 _S62 = { _S61 };
    return _S62;
}

AnyValue8 DiffPair_B_makePair_wtwrapper_0(AnyValue16 _S63, AnyValue8 _S64)
{
    return packAnyValue8_0(DiffPair_B_makePair_0(unpackAnyValue16_1(_S63)));
}

AnyValue8 _S65(uint2 _S66, AnyValue16 _S67, AnyValue8 _S68)
{
    switch(_S66.x)
    {
    case 0U:
        {
            return DiffPair_A_makePair_wtwrapper_0(_S67, _S68);
        }
    default:
        {
            return DiffPair_B_makePair_wtwrapper_0(_S67, _S68);
        }
    }
}

DiffPair_float_0 U_SFwdReq_R16existential_2Dx110IInterface4calcp1pi_ffb_0(uint2 _S69, AnyValue8 _S70, DiffPair_float_0 _S71)
{
    switch(_S69.x)
    {
    case 0U:
        {
            DiffPair_float_0 _S72 = s_fwd_A_calc_wtwrapper_0(_S70, _S71);
            return _S72;
        }
    default:
        {
            DiffPair_float_0 _S73 = s_fwd_B_calc_wtwrapper_0(_S70, _S71);
            return _S73;
        }
    }
}

DiffPair_float_0 s_fwd_doThing_0(DiffPair_IInterface_0 dpobj_0, DiffPair_float_0 dpx_4)
{
    DiffPair_float_0 _S74 = { dpx_4.primal_0, dpx_4.differential_0 };
    DiffPair_float_0 _S75 = U_SFwdReq_R16existential_2Dx110IInterface4calcp1pi_ffb_0(dpobj_0.primal_0.value1_0, _S65(dpobj_0.primal_0.value1_0, dpobj_0.primal_0.value2_0, dpobj_0.differential_0.value2_1), _S74);
    DiffPair_float_0 _S76 = { _S75.primal_0, _S75.differential_0 };
    return _S76;
}

DiffPair_float_0 s_fwd_f_0(uint id_0, DiffPair_float_0 dpx_5)
{
    Tuple_0 obj_0;
    Tuple_1 s_diff_obj_0;
    if(id_0 == 0U)
    {
        A_0 _S77 = A_x24init_0(int(2));
        AnyValue16 _S78 = packAnyValue16_0(_S77);
        uint2 _S79 = uint2(0U, 0U);
        NullDifferential_0 _S80 = { 0U };
        AnyValue8 _S81 = packAnyValue8_4(_S80);
        uint2 _S82 = uint2(4U, 0U);
        obj_0.value0_0 = uint2(0U, 0U);
        obj_0.value1_0 = _S79;
        obj_0.value2_0 = _S78;
        s_diff_obj_0.value0_1 = uint2(0U, 0U);
        s_diff_obj_0.value1_1 = _S82;
        s_diff_obj_0.value2_1 = _S81;
    }
    else
    {
        B_0 _S83 = B_x24init_0(int(2), int(3));
        AnyValue16 _S84 = packAnyValue16_1(_S83);
        uint2 _S85 = uint2(1U, 0U);
        NullDifferential_0 _S86 = { 0U };
        AnyValue8 _S87 = packAnyValue8_4(_S86);
        uint2 _S88 = uint2(4U, 0U);
        obj_0.value0_0 = uint2(0U, 0U);
        obj_0.value1_0 = _S85;
        obj_0.value2_0 = _S84;
        s_diff_obj_0.value0_1 = uint2(0U, 0U);
        s_diff_obj_0.value1_1 = _S88;
        s_diff_obj_0.value2_1 = _S87;
    }
    DiffPair_IInterface_0 _S89 = { obj_0, s_diff_obj_0 };
    DiffPair_float_0 _S90 = { dpx_5.primal_0, dpx_5.differential_0 };
    DiffPair_float_0 _S91 = s_fwd_doThing_0(_S89, _S90);
    DiffPair_float_0 _S92 = { _S91.primal_0, _S91.differential_0 };
    return _S92;
}

struct s_bwd_prop_f_Intermediates_0
{
    B_0 _S93;
    A_0 _S94;
};

float _S95(Tuple_0 _S96, float _S97)
{
    switch(_S96.value1_0.x)
    {
    case 0U:
        {
            float _S98 = A_calc_0(unpackAnyValue16_0(_S96.value2_0), _S97);
            return _S98;
        }
    default:
        {
            float _S99 = B_calc_0(unpackAnyValue16_1(_S96.value2_0), _S97);
            return _S99;
        }
    }
}

float s_primal_ctx_doThing_0(Tuple_0 dpobj_1, float dpx_6)
{
    float _S100 = _S95(dpobj_1, dpx_6);
    return _S100;
}

float s_primal_ctx_f_0(uint id_1, float dpx_7, out s_bwd_prop_f_Intermediates_0 _s_diff_ctx_0)
{
    B_0 _S101 = { int(0), int(0) };
    A_0 _S102 = { int(0) };
    _s_diff_ctx_0._S93 = _S101;
    _s_diff_ctx_0._S94 = _S102;
    _s_diff_ctx_0._S93.data1_0 = int(0);
    _s_diff_ctx_0._S93.data2_0 = int(0);
    _s_diff_ctx_0._S94.data1_1 = int(0);
    Tuple_0 obj_1;
    if(id_1 == 0U)
    {
        A_0 _S103 = A_x24init_0(int(2));
        _s_diff_ctx_0._S94 = _S103;
        AnyValue16 _S104 = packAnyValue16_0(_S103);
        uint2 _S105 = uint2(0U, 0U);
        obj_1.value0_0 = uint2(0U, 0U);
        obj_1.value1_0 = _S105;
        obj_1.value2_0 = _S104;
    }
    else
    {
        B_0 _S106 = B_x24init_0(int(2), int(3));
        _s_diff_ctx_0._S93 = _S106;
        AnyValue16 _S107 = packAnyValue16_1(_S106);
        uint2 _S108 = uint2(1U, 0U);
        obj_1.value0_0 = uint2(0U, 0U);
        obj_1.value1_0 = _S108;
        obj_1.value2_0 = _S107;
    }
    return s_primal_ctx_doThing_0(obj_1, dpx_7);
}

uint2 U_SR16existential_2Dx110IInterfaceI4core15IDifferentiable_0(uint2 _S109)
{
    switch(_S109.x)
    {
    case 0U:
        {
            return uint2(1U, 0U);
        }
    default:
        {
            return uint2(3U, 0U);
        }
    }
}

AnyValue8 U_S16NullDifferential5dzerop0p16NullDifferentialb_wtwrapper_0()
{
    NullDifferential_0 _S110 = { 0U };
    return packAnyValue8_4(_S110);
}

AnyValue8 U_S4core15IDifferentiable5dzerop0p4core15IDifferentiable12Differential_0(uint2 _S111)
{
    switch(_S111.x)
    {
    case 0U:
        {
            return U_SR16existential_2Dx11A12DifferentialR18_24x_u_usyn_udzerop0pR16existential_2Dx11A12Differentialb_wtwrapper_0();
        }
    case 1U:
        {
            return U_SR16existential_2Dx11AR18_24x_u_usyn_udzerop0pR16existential_2Dx11A12Differentialb_wtwrapper_0();
        }
    case 2U:
        {
            return U_SR16existential_2Dx11B12DifferentialR18_24x_u_usyn_udzerop0pR16existential_2Dx11B12Differentialb_wtwrapper_0();
        }
    case 3U:
        {
            return U_SR16existential_2Dx11BR18_24x_u_usyn_udzerop0pR16existential_2Dx11B12Differentialb_wtwrapper_0();
        }
    default:
        {
            return U_S16NullDifferential5dzerop0p16NullDifferentialb_wtwrapper_0();
        }
    }
}

void U_SBwdReq_R16existential_2Dx110IInterface4calcp1pi_ffb_0(uint2 _S112, inout AnyValue8 _S113, inout DiffPair_float_0 _S114, float _S115)
{
    switch(_S112.x)
    {
    case 0U:
        {
            s_bwd_A_calc_wtwrapper_0(_S113, _S114, _S115);
            return;
        }
    default:
        {
            s_bwd_B_calc_wtwrapper_0(_S113, _S114, _S115);
            return;
        }
    }
}

AnyValue8 DiffPair_A_getDiff_wtwrapper_0(AnyValue8 _S116)
{
    return packAnyValue8_2();
}

AnyValue8 DiffPair_B_getDiff_wtwrapper_0(AnyValue8 _S117)
{
    return packAnyValue8_3();
}

AnyValue8 _S118(uint2 _S119, AnyValue8 _S120)
{
    switch(_S119.x)
    {
    case 0U:
        {
            return DiffPair_A_getDiff_wtwrapper_0(_S120);
        }
    default:
        {
            return DiffPair_B_getDiff_wtwrapper_0(_S120);
        }
    }
}

void s_bwd_prop_doThing_0(inout DiffPair_IInterface_0 dpobj_2, inout DiffPair_float_0 dpx_8, float _s_dOut_2)
{
    uint2 _S121 = U_SR16existential_2Dx110IInterfaceI4core15IDifferentiable_0(dpobj_2.primal_0.value1_0);
    uint2 _S122 = uint2(0U, 0U);
    AnyValue8 _S123 = _S65(dpobj_2.primal_0.value1_0, dpobj_2.primal_0.value2_0, U_S4core15IDifferentiable5dzerop0p4core15IDifferentiable12Differential_0(_S121));
    DiffPair_float_0 _S124;
    _S124.primal_0 = dpx_8.primal_0;
    _S124.differential_0 = 0.0;
    U_SBwdReq_R16existential_2Dx110IInterface4calcp1pi_ffb_0(dpobj_2.primal_0.value1_0, _S123, _S124, _s_dOut_2);
    Tuple_1 _S125 = { _S122, _S121, _S118(dpobj_2.primal_0.value1_0, _S123) };
    dpx_8.primal_0 = dpx_8.primal_0;
    dpx_8.differential_0 = _S124.differential_0;
    dpobj_2.primal_0 = dpobj_2.primal_0;
    dpobj_2.differential_0 = _S125;
    return;
}

void s_bwd_prop_f_0(uint id_2, inout DiffPair_float_0 dpx_9, float _s_dOut_3, s_bwd_prop_f_Intermediates_0 _s_diff_ctx_1)
{
    DiffPair_float_0 _S126 = dpx_9;
    Tuple_0 obj_2;
    if(id_2 == 0U)
    {
        AnyValue16 _S127 = packAnyValue16_0(_s_diff_ctx_1._S94);
        uint2 _S128 = uint2(0U, 0U);
        obj_2.value0_0 = uint2(0U, 0U);
        obj_2.value1_0 = _S128;
        obj_2.value2_0 = _S127;
    }
    else
    {
        AnyValue16 _S129 = packAnyValue16_1(_s_diff_ctx_1._S93);
        uint2 _S130 = uint2(1U, 0U);
        obj_2.value0_0 = uint2(0U, 0U);
        obj_2.value1_0 = _S130;
        obj_2.value2_0 = _S129;
    }
    NullDifferential_0 _S131 = { 0U };
    Tuple_1 _S132 = { uint2(0U, 0U), uint2(4U, 0U), packAnyValue8_4(_S131) };
    DiffPair_IInterface_0 _S133;
    _S133.primal_0 = obj_2;
    _S133.differential_0 = _S132;
    DiffPair_float_0 _S134;
    _S134.primal_0 = _S126.primal_0;
    _S134.differential_0 = 0.0;
    s_bwd_prop_doThing_0(_S133, _S134, _s_dOut_3);
    dpx_9.primal_0 = dpx_9.primal_0;
    dpx_9.differential_0 = _S134.differential_0;
    return;
}

void s_bwd_f_0(uint _S135, inout DiffPair_float_0 _S136, float _S137)
{
    s_bwd_prop_f_Intermediates_0 _S138;
    float _S139 = s_primal_ctx_f_0(_S135, _S136.primal_0, _S138);
    s_bwd_prop_f_0(_S135, _S136, _S137, _S138);
    return;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{
    uint _S140 = dispatchThreadID_0.x;
    DiffPair_float_0 _S141 = { 1.0, 2.0 };
    DiffPair_float_0 _S142 = s_fwd_f_0(_S140, _S141);
    outputBuffer_0[int(0)] = _S142.differential_0;
    uint _S143 = _S140 + 1U;
    DiffPair_float_0 _S144 = { 1.5, 1.0 };
    DiffPair_float_0 _S145 = s_fwd_f_0(_S143, _S144);
    outputBuffer_0[int(1)] = _S145.differential_0;
    DiffPair_float_0 dpx_10;
    dpx_10.primal_0 = 1.0;
    dpx_10.differential_0 = 0.0;
    s_bwd_f_0(_S140, dpx_10, 2.0);
    outputBuffer_0[int(2)] = dpx_10.differential_0;
    DiffPair_float_0 dpx_11;
    dpx_11.primal_0 = 1.5;
    dpx_11.differential_0 = 0.0;
    s_bwd_f_0(_S143, dpx_11, 1.0);
    outputBuffer_0[int(3)] = dpx_11.differential_0;
    return;
}

