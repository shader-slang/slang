#pragma pack_matrix(row_major)
RWStructuredBuffer<float > outputBuffer_0 : register(u0);

struct PathResult_0
{
    float thp_0;
    float L_0;
};

PathResult_0 PathResult_x24_syn_dzero_0()
{
    PathResult_0 result_0;
    result_0.thp_0 = 0.0;
    result_0.L_0 = 0.0;
    return result_0;
}

PathResult_0 PathResult_x24_syn_dadd_0(PathResult_0 SLANG_anonymous_0_0, PathResult_0 SLANG_anonymous_1_0)
{
    PathResult_0 result_1;
    result_1.thp_0 = SLANG_anonymous_0_0.thp_0 + SLANG_anonymous_1_0.thp_0;
    result_1.L_0 = SLANG_anonymous_0_0.L_0 + SLANG_anonymous_1_0.L_0;
    return result_1;
}

struct PathState_0
{
    uint depth_0;
    bool terminated_0;
};

void generatePath_0(uint pathID_0, out PathState_0 path_0)
{
    path_0.terminated_0 = false;
    path_0.depth_0 = 0U;
    return;
}

bool PathState_isHit_0(PathState_0 this_0)
{
    return !this_0.terminated_0;
}

struct MaterialParam_0
{
    float roughness_0;
};

void d_getParam_0(uint id_0, MaterialParam_0 diff_0)
{
    outputBuffer_0[id_0] = outputBuffer_0[id_0] + diff_0.roughness_0;
    return;
}

MaterialParam_0 MaterialParam_x24_syn_dzero_0()
{
    MaterialParam_0 result_2;
    result_2.roughness_0 = 0.0;
    return result_2;
}

MaterialParam_0 MaterialParam_x24_syn_dadd_0(MaterialParam_0 SLANG_anonymous_0_1, MaterialParam_0 SLANG_anonymous_1_1)
{
    MaterialParam_0 result_3;
    result_3.roughness_0 = SLANG_anonymous_0_1.roughness_0 + SLANG_anonymous_1_1.roughness_0;
    return result_3;
}

MaterialParam_0 getParam_0(uint id_1)
{
    MaterialParam_0 p_0;
    p_0.roughness_0 = 0.5;
    return p_0;
}

bool PathState_isTerminated_0(PathState_0 this_1)
{
    return this_1.terminated_0;
}

struct DiffPair_PathResult_0
{
    PathResult_0 primal_0;
    PathResult_0 differential_0;
};

struct BSDFSample_0
{
    float val_0;
};

struct s_bwd_prop_updatePathThroughput_Intermediates_0
{
    PathResult_0 _S1;
};

struct s_bwd_prop_generateScatterRay_Intermediates_0
{
    PathResult_0 _S2;
    s_bwd_prop_updatePathThroughput_Intermediates_0 _S3;
};

struct s_bwd_prop_generateScatterRay_Intermediates_1
{
    PathResult_0 _S4;
    PathState_0 _S5;
    s_bwd_prop_generateScatterRay_Intermediates_0 _S6;
};

struct s_bwd_prop_generateScatterRay_Intermediates_2
{
    PathResult_0 _S7;
    PathState_0 _S8;
    BSDFSample_0 _S9;
    bool _S10;
    s_bwd_prop_generateScatterRay_Intermediates_1 _S11;
};

struct s_bwd_prop_handleHit_Intermediates_0
{
    PathResult_0 _S12;
    PathState_0 _S13;
    MaterialParam_0 _S14;
    s_bwd_prop_generateScatterRay_Intermediates_2 _S15;
    float _S16;
};

struct s_bwd_prop_tracePath_Intermediates_0
{
    int _S17;
    PathState_0  _S18[int(5)];
    PathResult_0  _S19[int(5)];
    bool  _S20[int(5)];
    PathState_0  _S21[int(5)];
    PathResult_0  _S22[int(5)];
    s_bwd_prop_handleHit_Intermediates_0  _S23[int(5)];
    bool  _S24[int(5)];
};

void s_primal_ctx_handleMiss_0(inout PathState_0 path_1, inout PathResult_0 dprs_0)
{
    PathResult_0 _S25 = dprs_0;
    PathState_0 _S26 = path_1;
    PathResult_0 _S27 = _S25;
    _S27.L_0 = 0.0;
    PathState_0 _S28 = _S26;
    _S28.terminated_0 = true;
    path_1 = _S28;
    dprs_0 = _S27;
    return;
}

MaterialParam_0 s_primal_ctx_getParam_0(uint _S29)
{
    MaterialParam_0 _S30 = getParam_0(_S29);
    return _S30;
}

bool s_primal_ctx_bsdfGGXSample_0(MaterialParam_0 dpbsdfParams_0, out BSDFSample_0 dpresult_0)
{
    dpresult_0.val_0 = dpbsdfParams_0.roughness_0;
    return true;
}

void s_primal_ctx_updatePathThroughput_0(inout PathResult_0 dppath_0, float dpweight_0, out s_bwd_prop_updatePathThroughput_Intermediates_0 _s_diff_ctx_0)
{
    PathResult_0 _S31 = { 0.0, 0.0 };
    _s_diff_ctx_0._S1 = _S31;
    _s_diff_ctx_0._S1.thp_0 = 0.0;
    _s_diff_ctx_0._S1.L_0 = 0.0;
    PathResult_0 _S32 = dppath_0;
    _s_diff_ctx_0._S1 = dppath_0;
    float _S33 = _S32.thp_0 * dpweight_0;
    PathResult_0 _S34 = _S32;
    _S34.thp_0 = _S33;
    dppath_0 = _S34;
    return;
}

bool s_primal_ctx_generateScatterRay_0(BSDFSample_0 dpbs_0, MaterialParam_0 dpbsdfParams_1, inout PathState_0 path_2, inout PathResult_0 dppathRes_0, out s_bwd_prop_generateScatterRay_Intermediates_0 _s_diff_ctx_1)
{
    PathResult_0 _S35 = { 0.0, 0.0 };
    s_bwd_prop_updatePathThroughput_Intermediates_0 _S36 = { _S35 };
    _s_diff_ctx_1._S2 = _S35;
    _s_diff_ctx_1._S3 = _S36;
    _s_diff_ctx_1._S2.thp_0 = 0.0;
    _s_diff_ctx_1._S2.L_0 = 0.0;
    PathResult_0 _S37 = dppathRes_0;
    _s_diff_ctx_1._S2 = dppathRes_0;
    PathState_0 _S38 = path_2;
    PathResult_0 _S39 = _S37;
    s_primal_ctx_updatePathThroughput_0(_S39, dpbs_0.val_0, _s_diff_ctx_1._S3);
    PathResult_0 _S40 = _S39;
    path_2 = _S38;
    dppathRes_0 = _S40;
    return true;
}

bool s_primal_ctx_generateScatterRay_1(BSDFSample_0 dpbs_1, MaterialParam_0 dpbsdfParams_2, inout PathState_0 path_3, inout PathResult_0 dppathRes_1, bool valid_0, out s_bwd_prop_generateScatterRay_Intermediates_1 _s_diff_ctx_2)
{
    PathResult_0 _S41 = { 0.0, 0.0 };
    PathState_0 _S42 = { 0U, false };
    s_bwd_prop_updatePathThroughput_Intermediates_0 _S43 = { _S41 };
    s_bwd_prop_generateScatterRay_Intermediates_0 _S44 = { _S41, _S43 };
    _s_diff_ctx_2._S4 = _S41;
    _s_diff_ctx_2._S5 = _S42;
    _s_diff_ctx_2._S6 = _S44;
    _s_diff_ctx_2._S4.thp_0 = 0.0;
    _s_diff_ctx_2._S4.L_0 = 0.0;
    _s_diff_ctx_2._S5.depth_0 = 0U;
    _s_diff_ctx_2._S5.terminated_0 = false;
    PathResult_0 _S45 = dppathRes_1;
    _s_diff_ctx_2._S4 = dppathRes_1;
    PathState_0 _S46 = path_3;
    _s_diff_ctx_2._S5 = path_3;
    bool _S47;
    PathState_0 _S48;
    PathResult_0 _S49;
    if(valid_0)
    {
        PathState_0 _S50 = _S46;
        PathResult_0 _S51 = _S45;
        bool _S52 = s_primal_ctx_generateScatterRay_0(dpbs_1, dpbsdfParams_2, _S50, _S51, _s_diff_ctx_2._S6);
        PathResult_0 _S53 = _S51;
        PathState_0 _S54 = _S50;
        _S47 = _S52;
        _S48 = _S54;
        _S49 = _S53;
    }
    else
    {
        _S47 = valid_0;
        _S48 = _S46;
        _S49 = _S45;
    }
    path_3 = _S48;
    dppathRes_1 = _S49;
    return _S47;
}

bool s_primal_ctx_generateScatterRay_2(MaterialParam_0 dpbsdfParams_3, inout PathState_0 path_4, inout PathResult_0 dppathRes_2, out s_bwd_prop_generateScatterRay_Intermediates_2 _s_diff_ctx_3)
{
    PathResult_0 _S55 = { 0.0, 0.0 };
    PathState_0 _S56 = { 0U, false };
    BSDFSample_0 _S57 = { 0.0 };
    s_bwd_prop_updatePathThroughput_Intermediates_0 _S58 = { _S55 };
    s_bwd_prop_generateScatterRay_Intermediates_0 _S59 = { _S55, _S58 };
    s_bwd_prop_generateScatterRay_Intermediates_1 _S60 = { _S55, _S56, _S59 };
    _s_diff_ctx_3._S7 = _S55;
    _s_diff_ctx_3._S8 = _S56;
    _s_diff_ctx_3._S9 = _S57;
    _s_diff_ctx_3._S10 = false;
    _s_diff_ctx_3._S11 = _S60;
    _s_diff_ctx_3._S7.thp_0 = 0.0;
    _s_diff_ctx_3._S7.L_0 = 0.0;
    _s_diff_ctx_3._S8.depth_0 = 0U;
    _s_diff_ctx_3._S8.terminated_0 = false;
    _s_diff_ctx_3._S9.val_0 = 0.0;
    _s_diff_ctx_3._S10 = false;
    PathResult_0 _S61 = dppathRes_2;
    _s_diff_ctx_3._S7 = dppathRes_2;
    PathState_0 _S62 = path_4;
    _s_diff_ctx_3._S8 = path_4;
    BSDFSample_0 _S63;
    _S63.val_0 = 0.0;
    bool _S64 = s_primal_ctx_bsdfGGXSample_0(dpbsdfParams_3, _S63);
    _s_diff_ctx_3._S9 = _S63;
    _s_diff_ctx_3._S10 = _S64;
    BSDFSample_0 _S65 = _S63;
    PathState_0 _S66 = _S62;
    PathResult_0 _S67 = _S61;
    bool _S68 = s_primal_ctx_generateScatterRay_1(_S65, dpbsdfParams_3, _S66, _S67, _S64, _s_diff_ctx_3._S11);
    PathResult_0 _S69 = _S67;
    PathState_0 _S70 = _S66;
    path_4 = _S70;
    dppathRes_2 = _S69;
    return _S68;
}

float s_primal_ctx_lightEval_0(uint depth_1)
{
    float _S71;
    if(depth_1 == 1U)
    {
        _S71 = 5.0;
    }
    else
    {
        _S71 = 0.0;
    }
    return _S71;
}

void s_primal_ctx_handleHit_0(inout PathState_0 path_5, inout PathResult_0 dprs_1, out s_bwd_prop_handleHit_Intermediates_0 _s_diff_ctx_4)
{
    PathResult_0 _S72 = { 0.0, 0.0 };
    PathState_0 _S73 = { 0U, false };
    MaterialParam_0 _S74 = { 0.0 };
    BSDFSample_0 _S75 = { 0.0 };
    s_bwd_prop_updatePathThroughput_Intermediates_0 _S76 = { _S72 };
    s_bwd_prop_generateScatterRay_Intermediates_0 _S77 = { _S72, _S76 };
    s_bwd_prop_generateScatterRay_Intermediates_1 _S78 = { _S72, _S73, _S77 };
    s_bwd_prop_generateScatterRay_Intermediates_2 _S79 = { _S72, _S73, _S75, false, _S78 };
    _s_diff_ctx_4._S12 = _S72;
    _s_diff_ctx_4._S13 = _S73;
    _s_diff_ctx_4._S14 = _S74;
    _s_diff_ctx_4._S15 = _S79;
    _s_diff_ctx_4._S16 = 0.0;
    _s_diff_ctx_4._S12.thp_0 = 0.0;
    _s_diff_ctx_4._S12.L_0 = 0.0;
    _s_diff_ctx_4._S13.depth_0 = 0U;
    _s_diff_ctx_4._S13.terminated_0 = false;
    _s_diff_ctx_4._S14.roughness_0 = 0.0;
    _s_diff_ctx_4._S16 = 0.0;
    PathResult_0 _S80 = dprs_1;
    _s_diff_ctx_4._S12 = dprs_1;
    PathState_0 _S81 = path_5;
    _s_diff_ctx_4._S13 = path_5;
    MaterialParam_0 _S82 = s_primal_ctx_getParam_0(0U);
    _s_diff_ctx_4._S14 = _S82;
    bool lastVertex_0 = _S82.roughness_0 > 0.80000001192092896;
    PathState_0 _S83;
    if(lastVertex_0)
    {
        _S83 = _S81;
        _S83.terminated_0 = true;
    }
    else
    {
        _S83 = _S81;
    }
    bool _S84 = !lastVertex_0;
    PathResult_0 _S85;
    if(_S84)
    {
        PathState_0 _S86 = _S83;
        PathResult_0 _S87 = _S80;
        bool _S88 = s_primal_ctx_generateScatterRay_2(_S82, _S86, _S87, _s_diff_ctx_4._S15);
        PathResult_0 _S89 = _S87;
        PathState_0 _S90 = _S86;
        float _S91 = s_primal_ctx_lightEval_0(_S90.depth_0);
        _s_diff_ctx_4._S16 = _S91;
        float _S92 = _S89.thp_0 * _S91;
        _S85 = _S89;
        _S85.L_0 = _S92;
        if(_S90.depth_0 < 1U)
        {
            _S83 = _S90;
            _S83.terminated_0 = false;
        }
        else
        {
            _S83 = _S90;
            _S83.terminated_0 = true;
        }
    }
    else
    {
        _S85 = _S80;
    }
    path_5 = _S83;
    dprs_1 = _S85;
    return;
}

void s_primal_ctx_nextHit_0(inout PathState_0 path_6, inout PathResult_0 dprs_2)
{
    PathResult_0 _S93 = dprs_2;
    PathState_0 _S94 = path_6;
    uint _S95 = _S94.depth_0 + 1U;
    PathState_0 _S96 = _S94;
    _S96.depth_0 = _S95;
    path_6 = _S96;
    dprs_2 = _S93;
    return;
}

bool s_primal_ctx_tracePath_0(uint pathID_1, out PathState_0 path_7, inout PathResult_0 dppathRes_3, out s_bwd_prop_tracePath_Intermediates_0 _s_diff_ctx_5)
{
    PathState_0 _S97 = { 0U, false };
    PathState_0  _S98[int(5)] = { _S97, _S97, _S97, _S97, _S97 };
    PathResult_0 _S99 = { 0.0, 0.0 };
    PathResult_0  _S100[int(5)] = { _S99, _S99, _S99, _S99, _S99 };
    bool  _S101[int(5)] = { false, false, false, false, false };
    MaterialParam_0 _S102 = { 0.0 };
    BSDFSample_0 _S103 = { 0.0 };
    s_bwd_prop_updatePathThroughput_Intermediates_0 _S104 = { _S99 };
    s_bwd_prop_generateScatterRay_Intermediates_0 _S105 = { _S99, _S104 };
    s_bwd_prop_generateScatterRay_Intermediates_1 _S106 = { _S99, _S97, _S105 };
    s_bwd_prop_generateScatterRay_Intermediates_2 _S107 = { _S99, _S97, _S103, false, _S106 };
    s_bwd_prop_handleHit_Intermediates_0 _S108 = { _S99, _S97, _S102, _S107, 0.0 };
    s_bwd_prop_handleHit_Intermediates_0  _S109[int(5)] = { _S108, _S108, _S108, _S108, _S108 };
    _s_diff_ctx_5._S17 = int(0);
    _s_diff_ctx_5._S18 = _S98;
    _s_diff_ctx_5._S19 = _S100;
    _s_diff_ctx_5._S20 = _S101;
    _s_diff_ctx_5._S21 = _S98;
    _s_diff_ctx_5._S22 = _S100;
    _s_diff_ctx_5._S23 = _S109;
    _s_diff_ctx_5._S24 = _S101;
    _s_diff_ctx_5._S17 = int(0);
    _s_diff_ctx_5._S18[int(0)] = _S97;
    _s_diff_ctx_5._S18[int(1)] = _S97;
    _s_diff_ctx_5._S18[int(2)] = _S97;
    _s_diff_ctx_5._S18[int(3)] = _S97;
    _s_diff_ctx_5._S18[int(4)] = _S97;
    _s_diff_ctx_5._S19[int(0)] = _S99;
    _s_diff_ctx_5._S19[int(1)] = _S99;
    _s_diff_ctx_5._S19[int(2)] = _S99;
    _s_diff_ctx_5._S19[int(3)] = _S99;
    _s_diff_ctx_5._S19[int(4)] = _S99;
    _s_diff_ctx_5._S20[int(0)] = false;
    _s_diff_ctx_5._S20[int(1)] = false;
    _s_diff_ctx_5._S20[int(2)] = false;
    _s_diff_ctx_5._S20[int(3)] = false;
    _s_diff_ctx_5._S20[int(4)] = false;
    _s_diff_ctx_5._S21[int(0)] = _S97;
    _s_diff_ctx_5._S21[int(1)] = _S97;
    _s_diff_ctx_5._S21[int(2)] = _S97;
    _s_diff_ctx_5._S21[int(3)] = _S97;
    _s_diff_ctx_5._S21[int(4)] = _S97;
    _s_diff_ctx_5._S22[int(0)] = _S99;
    _s_diff_ctx_5._S22[int(1)] = _S99;
    _s_diff_ctx_5._S22[int(2)] = _S99;
    _s_diff_ctx_5._S22[int(3)] = _S99;
    _s_diff_ctx_5._S22[int(4)] = _S99;
    _s_diff_ctx_5._S23[int(0)] = _S108;
    _s_diff_ctx_5._S23[int(1)] = _S108;
    _s_diff_ctx_5._S23[int(2)] = _S108;
    _s_diff_ctx_5._S23[int(3)] = _S108;
    _s_diff_ctx_5._S23[int(4)] = _S108;
    _s_diff_ctx_5._S24[int(0)] = false;
    _s_diff_ctx_5._S24[int(1)] = false;
    _s_diff_ctx_5._S24[int(2)] = false;
    _s_diff_ctx_5._S24[int(3)] = false;
    _s_diff_ctx_5._S24[int(4)] = false;
    PathResult_0 _S110 = dppathRes_3;
    PathState_0 _S111;
    _S111.depth_0 = 0U;
    _S111.terminated_0 = false;
    generatePath_0(pathID_1, _S111);
    PathState_0 _S112 = _S111;
    bool _bflag_0 = true;
    int i_0 = int(0);
    PathState_0 _S113 = _S112;
    PathResult_0 _S114 = _S110;
    int _pc_0 = int(0);
    for(;;)
    {
        _s_diff_ctx_5._S17 = _pc_0;
        _s_diff_ctx_5._S18[_pc_0] = _S113;
        _s_diff_ctx_5._S19[_pc_0] = _S114;
        if(_bflag_0)
        {
        }
        else
        {
            break;
        }
        bool _S115 = i_0 < int(3);
        bool _bflag_1;
        int _S116;
        if(_S115)
        {
            bool _S117 = PathState_isHit_0(_S113);
            _s_diff_ctx_5._S20[_pc_0] = _S117;
            PathState_0 _S118;
            PathResult_0 _S119;
            if(_S117)
            {
                PathState_0 _S120 = _S113;
                PathResult_0 _S121 = _S114;
                s_bwd_prop_handleHit_Intermediates_0 _S122;
                _S122._S12 = _S99;
                _S122._S13 = _S97;
                _S122._S14 = _S102;
                _S122._S15 = _S107;
                _S122._S16 = 0.0;
                s_primal_ctx_handleHit_0(_S120, _S121, _S122);
                _s_diff_ctx_5._S21[_pc_0] = _S120;
                _s_diff_ctx_5._S22[_pc_0] = _S121;
                _s_diff_ctx_5._S23[_pc_0] = _S122;
                PathResult_0 _S123 = _S121;
                PathState_0 _S124 = _S120;
                bool _S125 = PathState_isTerminated_0(_S124);
                _s_diff_ctx_5._S24[_pc_0] = _S125;
                if(_S125)
                {
                    _bflag_1 = false;
                }
                else
                {
                    _bflag_1 = _S115;
                }
                if(_bflag_1)
                {
                    PathState_0 _S126 = _S124;
                    PathResult_0 _S127 = _S123;
                    s_primal_ctx_nextHit_0(_S126, _S127);
                    PathResult_0 _S128 = _S127;
                    PathState_0 _S129 = _S126;
                    _S118 = _S129;
                    _S119 = _S128;
                }
                else
                {
                    _S118 = _S124;
                    _S119 = _S123;
                }
            }
            else
            {
                PathState_0 _S130 = _S113;
                PathResult_0 _S131 = _S114;
                s_primal_ctx_handleMiss_0(_S130, _S131);
                PathResult_0 _S132 = _S131;
                PathState_0 _S133 = _S130;
                _bflag_1 = _S115;
                _S118 = _S133;
                _S119 = _S132;
            }
            if(_bflag_1)
            {
                _S116 = int(1);
            }
            else
            {
                _S116 = int(0);
            }
            _S113 = _S118;
            _S114 = _S119;
        }
        else
        {
            _S116 = int(0);
        }
        if(_S116 != int(1))
        {
            _bflag_1 = false;
        }
        else
        {
            _bflag_1 = _bflag_0;
        }
        if(_bflag_1)
        {
            int i_1 = i_0 + int(1);
            i_0 = i_1;
        }
        int _S134 = _pc_0 + int(1);
        _bflag_0 = _bflag_1;
        _pc_0 = _S134;
    }
    path_7 = _S113;
    dppathRes_3 = _S114;
    return true;
}

void s_bwd_prop_handleMiss_0(PathState_0 path_8, inout DiffPair_PathResult_0 dprs_3)
{
    PathResult_0 _S135 = PathResult_x24_syn_dzero_0();
    PathResult_0 _S136 = PathResult_x24_syn_dadd_0(dprs_3.differential_0, _S135);
    _S136.L_0 = 0.0;
    PathResult_0 dprs_4 = PathResult_x24_syn_dadd_0(_S136, _S135);
    dprs_3.primal_0 = dprs_3.primal_0;
    dprs_3.differential_0 = dprs_4;
    return;
}

void s_bwd_prop_nextHit_0(PathState_0 path_9, inout DiffPair_PathResult_0 dprs_5)
{
    PathResult_0 _S137 = PathResult_x24_syn_dzero_0();
    PathResult_0 dprs_6 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(dprs_5.differential_0, _S137), _S137);
    dprs_5.primal_0 = dprs_5.primal_0;
    dprs_5.differential_0 = dprs_6;
    return;
}

struct DiffPair_MaterialParam_0
{
    MaterialParam_0 primal_0;
    MaterialParam_0 differential_0;
};

BSDFSample_0 BSDFSample_x24_syn_dzero_0()
{
    BSDFSample_0 result_4;
    result_4.val_0 = 0.0;
    return result_4;
}

struct DiffPair_BSDFSample_0
{
    BSDFSample_0 primal_0;
    BSDFSample_0 differential_0;
};

struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};

void s_bwd_prop_updatePathThroughput_0(inout DiffPair_PathResult_0 dppath_1, inout DiffPair_float_0 dpweight_1, s_bwd_prop_updatePathThroughput_Intermediates_0 _s_diff_ctx_6)
{
    DiffPair_PathResult_0 _S138 = dppath_1;
    PathResult_0 _S139 = PathResult_x24_syn_dzero_0();
    PathResult_0 _S140 = PathResult_x24_syn_dadd_0(dppath_1.differential_0, _S139);
    PathResult_0 _S141 = _S140;
    _S141.thp_0 = 0.0;
    float _S142 = _s_diff_ctx_6._S1.thp_0 * _S140.thp_0;
    float _S143 = dpweight_1.primal_0 * _S140.thp_0;
    PathResult_0 _S144 = PathResult_x24_syn_dadd_0(_S141, _S139);
    PathResult_0 _S145 = _S139;
    _S145.thp_0 = _S143;
    PathResult_0 dppath_2 = PathResult_x24_syn_dadd_0(_S144, _S145);
    dpweight_1.primal_0 = dpweight_1.primal_0;
    dpweight_1.differential_0 = _S142;
    dppath_1.primal_0 = _S138.primal_0;
    dppath_1.differential_0 = dppath_2;
    return;
}

void s_bwd_prop_generateScatterRay_0(inout DiffPair_BSDFSample_0 dpbs_2, inout DiffPair_MaterialParam_0 dpbsdfParams_4, PathState_0 path_10, inout DiffPair_PathResult_0 dppathRes_4, s_bwd_prop_generateScatterRay_Intermediates_0 _s_diff_ctx_7)
{
    DiffPair_PathResult_0 _S146 = dppathRes_4;
    PathResult_0 _S147 = PathResult_x24_syn_dzero_0();
    PathResult_0 _S148 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(dppathRes_4.differential_0, _S147), _S147);
    DiffPair_PathResult_0 _S149;
    _S149.primal_0 = _s_diff_ctx_7._S2;
    _S149.differential_0 = _S148;
    DiffPair_float_0 _S150;
    _S150.primal_0 = dpbs_2.primal_0.val_0;
    _S150.differential_0 = 0.0;
    s_bwd_prop_updatePathThroughput_0(_S149, _S150, _s_diff_ctx_7._S3);
    PathResult_0 dppathRes_5 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(_S149.differential_0, _S147), _S147);
    MaterialParam_0 _S151 = MaterialParam_x24_syn_dzero_0();
    dpbsdfParams_4.primal_0 = dpbsdfParams_4.primal_0;
    dpbsdfParams_4.differential_0 = _S151;
    BSDFSample_0 _S152 = BSDFSample_x24_syn_dzero_0();
    _S152.val_0 = _S150.differential_0;
    dpbs_2.primal_0 = dpbs_2.primal_0;
    dpbs_2.differential_0 = _S152;
    dppathRes_4.primal_0 = _S146.primal_0;
    dppathRes_4.differential_0 = dppathRes_5;
    return;
}

BSDFSample_0 BSDFSample_x24_syn_dadd_0(BSDFSample_0 SLANG_anonymous_0_2, BSDFSample_0 SLANG_anonymous_1_2)
{
    BSDFSample_0 result_5;
    result_5.val_0 = SLANG_anonymous_0_2.val_0 + SLANG_anonymous_1_2.val_0;
    return result_5;
}

void s_bwd_prop_generateScatterRay_1(inout DiffPair_BSDFSample_0 dpbs_3, inout DiffPair_MaterialParam_0 dpbsdfParams_5, PathState_0 path_11, inout DiffPair_PathResult_0 dppathRes_6, bool valid_1, s_bwd_prop_generateScatterRay_Intermediates_1 _s_diff_ctx_8)
{
    DiffPair_BSDFSample_0 _S153 = dpbs_3;
    DiffPair_MaterialParam_0 _S154 = dpbsdfParams_5;
    DiffPair_PathResult_0 _S155 = dppathRes_6;
    PathResult_0 dppathRes_7 = PathResult_x24_syn_dzero_0();
    PathState_0 _S156;
    PathResult_0 dppathRes_8;
    if(valid_1)
    {
        _S156 = _s_diff_ctx_8._S5;
        dppathRes_8 = _s_diff_ctx_8._S4;
    }
    else
    {
        _S156.depth_0 = 0U;
        _S156.terminated_0 = false;
        dppathRes_8.thp_0 = 0.0;
        dppathRes_8.L_0 = 0.0;
    }
    BSDFSample_0 _S157 = BSDFSample_x24_syn_dzero_0();
    MaterialParam_0 _S158 = MaterialParam_x24_syn_dzero_0();
    PathResult_0 _S159 = PathResult_x24_syn_dadd_0(_S155.differential_0, dppathRes_7);
    MaterialParam_0 _S160;
    BSDFSample_0 _S161;
    if(valid_1)
    {
        PathResult_0 _S162 = PathResult_x24_syn_dadd_0(_S159, dppathRes_7);
        DiffPair_BSDFSample_0 _S163;
        _S163.primal_0 = _S153.primal_0;
        _S163.differential_0 = _S157;
        DiffPair_MaterialParam_0 _S164;
        _S164.primal_0 = _S154.primal_0;
        _S164.differential_0 = _S158;
        DiffPair_PathResult_0 _S165;
        _S165.primal_0 = dppathRes_8;
        _S165.differential_0 = _S162;
        s_bwd_prop_generateScatterRay_0(_S163, _S164, _S156, _S165, _s_diff_ctx_8._S6);
        MaterialParam_0 _S166 = MaterialParam_x24_syn_dadd_0(_S164.differential_0, _S158);
        BSDFSample_0 _S167 = BSDFSample_x24_syn_dadd_0(_S163.differential_0, _S157);
        dppathRes_8 = PathResult_x24_syn_dadd_0(_S165.differential_0, dppathRes_7);
        _S160 = _S166;
        _S161 = _S167;
    }
    else
    {
        dppathRes_8 = PathResult_x24_syn_dadd_0(_S159, dppathRes_7);
        _S160 = _S158;
        _S161 = _S157;
    }
    PathResult_0 dppathRes_9 = PathResult_x24_syn_dadd_0(dppathRes_7, dppathRes_8);
    dpbsdfParams_5.primal_0 = dpbsdfParams_5.primal_0;
    dpbsdfParams_5.differential_0 = _S160;
    dpbs_3.primal_0 = dpbs_3.primal_0;
    dpbs_3.differential_0 = _S161;
    dppathRes_6.primal_0 = _S155.primal_0;
    dppathRes_6.differential_0 = dppathRes_9;
    return;
}

void s_bwd_prop_bsdfGGXSample_0(inout DiffPair_MaterialParam_0 dpbsdfParams_6, BSDFSample_0 dpresult_1)
{
    BSDFSample_0 _S168 = BSDFSample_x24_syn_dadd_0(dpresult_1, BSDFSample_x24_syn_dzero_0());
    MaterialParam_0 _S169 = MaterialParam_x24_syn_dzero_0();
    _S169.roughness_0 = _S168.val_0;
    dpbsdfParams_6.primal_0 = dpbsdfParams_6.primal_0;
    dpbsdfParams_6.differential_0 = _S169;
    return;
}

void s_bwd_prop_generateScatterRay_2(inout DiffPair_MaterialParam_0 dpbsdfParams_7, PathState_0 path_12, inout DiffPair_PathResult_0 dppathRes_10, s_bwd_prop_generateScatterRay_Intermediates_2 _s_diff_ctx_9)
{
    DiffPair_PathResult_0 _S170 = dppathRes_10;
    PathResult_0 _S171 = PathResult_x24_syn_dzero_0();
    BSDFSample_0 _S172 = BSDFSample_x24_syn_dzero_0();
    PathResult_0 _S173 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(dppathRes_10.differential_0, _S171), _S171);
    DiffPair_BSDFSample_0 _S174;
    _S174.primal_0 = _s_diff_ctx_9._S9;
    _S174.differential_0 = _S172;
    MaterialParam_0 _S175 = MaterialParam_x24_syn_dzero_0();
    DiffPair_MaterialParam_0 _S176;
    _S176.primal_0 = dpbsdfParams_7.primal_0;
    _S176.differential_0 = _S175;
    DiffPair_PathResult_0 _S177;
    _S177.primal_0 = _s_diff_ctx_9._S7;
    _S177.differential_0 = _S173;
    s_bwd_prop_generateScatterRay_1(_S174, _S176, _s_diff_ctx_9._S8, _S177, _s_diff_ctx_9._S10, _s_diff_ctx_9._S11);
    BSDFSample_0 _S178 = BSDFSample_x24_syn_dadd_0(_S174.differential_0, _S172);
    DiffPair_MaterialParam_0 _S179;
    _S179.primal_0 = dpbsdfParams_7.primal_0;
    _S179.differential_0 = _S175;
    s_bwd_prop_bsdfGGXSample_0(_S179, _S178);
    PathResult_0 dppathRes_11 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(_S177.differential_0, _S171), _S171);
    MaterialParam_0 _S180 = MaterialParam_x24_syn_dadd_0(_S176.differential_0, _S179.differential_0);
    dpbsdfParams_7.primal_0 = dpbsdfParams_7.primal_0;
    dpbsdfParams_7.differential_0 = _S180;
    dppathRes_10.primal_0 = _S170.primal_0;
    dppathRes_10.differential_0 = dppathRes_11;
    return;
}

void s_bwd_prop_getParam_0(uint _S181, MaterialParam_0 _S182)
{
    d_getParam_0(_S181, _S182);
    return;
}

void s_bwd_prop_handleHit_0(PathState_0 path_13, inout DiffPair_PathResult_0 dprs_7, s_bwd_prop_handleHit_Intermediates_0 _s_diff_ctx_10)
{
    DiffPair_PathResult_0 _S183 = dprs_7;
    PathResult_0 dprs_8 = PathResult_x24_syn_dzero_0();
    bool lastVertex_1 = _s_diff_ctx_10._S14.roughness_0 > 0.80000001192092896;
    PathState_0 _S184;
    if(lastVertex_1)
    {
        _S184 = _s_diff_ctx_10._S13;
        _S184.terminated_0 = true;
    }
    else
    {
        _S184 = _s_diff_ctx_10._S13;
    }
    bool _S185 = !lastVertex_1;
    PathResult_0 dprs_9;
    if(_S185)
    {
        dprs_9 = _s_diff_ctx_10._S12;
    }
    else
    {
        _S184.depth_0 = 0U;
        _S184.terminated_0 = false;
        dprs_9.thp_0 = 0.0;
        dprs_9.L_0 = 0.0;
    }
    MaterialParam_0 _S186 = MaterialParam_x24_syn_dzero_0();
    PathResult_0 _S187 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(_S183.differential_0, dprs_8), dprs_8);
    MaterialParam_0 _S188;
    if(_S185)
    {
        PathResult_0 _S189 = PathResult_x24_syn_dadd_0(_S187, dprs_8);
        PathResult_0 _S190 = _S189;
        _S190.L_0 = 0.0;
        float _S191 = _s_diff_ctx_10._S16 * _S189.L_0;
        PathResult_0 _S192 = PathResult_x24_syn_dadd_0(_S190, dprs_8);
        PathResult_0 _S193 = dprs_8;
        _S193.thp_0 = _S191;
        PathResult_0 _S194 = PathResult_x24_syn_dadd_0(_S192, _S193);
        DiffPair_MaterialParam_0 _S195;
        _S195.primal_0 = _s_diff_ctx_10._S14;
        _S195.differential_0 = _S186;
        DiffPair_PathResult_0 _S196;
        _S196.primal_0 = dprs_9;
        _S196.differential_0 = _S194;
        s_bwd_prop_generateScatterRay_2(_S195, _S184, _S196, _s_diff_ctx_10._S15);
        PathResult_0 _S197 = PathResult_x24_syn_dadd_0(_S196.differential_0, dprs_8);
        _S188 = MaterialParam_x24_syn_dadd_0(_S195.differential_0, _S186);
        dprs_9 = _S197;
    }
    else
    {
        PathResult_0 _S198 = PathResult_x24_syn_dadd_0(_S187, dprs_8);
        _S188 = _S186;
        dprs_9 = _S198;
    }
    MaterialParam_0 _S199 = _S186;
    _S199.roughness_0 = 0.0;
    s_bwd_prop_getParam_0(0U, MaterialParam_x24_syn_dadd_0(_S188, _S199));
    PathResult_0 dprs_10 = PathResult_x24_syn_dadd_0(dprs_8, dprs_9);
    dprs_7.primal_0 = _S183.primal_0;
    dprs_7.differential_0 = dprs_10;
    return;
}

void s_bwd_prop_tracePath_0(uint pathID_2, inout DiffPair_PathResult_0 dppathRes_12, s_bwd_prop_tracePath_Intermediates_0 _s_diff_ctx_11)
{
    DiffPair_PathResult_0 _S200 = dppathRes_12;
    PathResult_0 _S201 = PathResult_x24_syn_dzero_0();
    PathResult_0 _S202 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(dppathRes_12.differential_0, _S201), _S201);
    int _dc_0 = _s_diff_ctx_11._S17 - int(1);
    PathResult_0 _S203 = _S202;
    PathResult_0 _S204 = _S201;
    PathResult_0 _S205 = _S201;
    PathResult_0 _S206 = _S201;
    PathResult_0 _S207 = _S201;
    PathResult_0 _S208 = _S201;
    PathResult_0 _S209 = _S201;
    for(;;)
    {
        if(_dc_0 >= int(0))
        {
        }
        else
        {
            break;
        }
        bool _S210 = _dc_0 < int(3);
        PathResult_0 _S211;
        PathResult_0 _S212;
        PathResult_0 _S213;
        bool _bflag_2;
        PathState_0 _S214;
        PathState_0 _S215;
        PathState_0 _S216;
        if(_S210)
        {
            if(_s_diff_ctx_11._S20[_dc_0])
            {
                int _S217 = _dc_0;
                int _S218 = _dc_0;
                int _S219 = _dc_0;
                int _S220 = _dc_0;
                if(_s_diff_ctx_11._S24[_dc_0])
                {
                    _bflag_2 = false;
                }
                else
                {
                    _bflag_2 = _S210;
                }
                if(_bflag_2)
                {
                    _S214 = _s_diff_ctx_11._S21[_S220];
                    _S211 = _s_diff_ctx_11._S22[_S219];
                }
                else
                {
                    _S214.depth_0 = 0U;
                    _S214.terminated_0 = false;
                    _S211.thp_0 = 0.0;
                    _S211.L_0 = 0.0;
                }
                PathState_0 _S221 = _S214;
                PathResult_0 _S222 = _S211;
                _S214.depth_0 = 0U;
                _S214.terminated_0 = false;
                _S211.thp_0 = 0.0;
                _S211.L_0 = 0.0;
                _S215 = _S221;
                _S212 = _S222;
                _S216 = _s_diff_ctx_11._S18[_S217];
                _S213 = _s_diff_ctx_11._S19[_S218];
            }
            else
            {
                _S214 = _s_diff_ctx_11._S18[_dc_0];
                _S211 = _s_diff_ctx_11._S19[_dc_0];
                _bflag_2 = false;
                _S215.depth_0 = 0U;
                _S215.terminated_0 = false;
                _S212.thp_0 = 0.0;
                _S212.L_0 = 0.0;
                _S216.depth_0 = 0U;
                _S216.terminated_0 = false;
                _S213.thp_0 = 0.0;
                _S213.L_0 = 0.0;
            }
        }
        else
        {
            _S214.depth_0 = 0U;
            _S214.terminated_0 = false;
            _S211.thp_0 = 0.0;
            _S211.L_0 = 0.0;
            _bflag_2 = false;
            _S215.depth_0 = 0U;
            _S215.terminated_0 = false;
            _S212.thp_0 = 0.0;
            _S212.L_0 = 0.0;
            _S216.depth_0 = 0U;
            _S216.terminated_0 = false;
            _S213.thp_0 = 0.0;
            _S213.L_0 = 0.0;
        }
        PathResult_0 _S223 = PathResult_x24_syn_dadd_0(_S203, _S204);
        if(_S210)
        {
            PathResult_0 _S224 = PathResult_x24_syn_dadd_0(_S223, _S205);
            PathResult_0 _S225;
            PathResult_0 _S226;
            PathResult_0 _S227;
            PathResult_0 _S228;
            PathResult_0 _S229;
            if(_s_diff_ctx_11._S20[_dc_0])
            {
                if(_bflag_2)
                {
                    PathResult_0 _S230 = PathResult_x24_syn_dadd_0(_S224, _S208);
                    DiffPair_PathResult_0 _S231;
                    _S231.primal_0 = _S212;
                    _S231.differential_0 = _S230;
                    s_bwd_prop_nextHit_0(_S215, _S231);
                    _S225 = PathResult_x24_syn_dadd_0(_S231.differential_0, _S201);
                    _S226 = _S201;
                }
                else
                {
                    _S225 = PathResult_x24_syn_dadd_0(_S224, _S207);
                    _S226 = _S208;
                }
                PathResult_0 _S232 = PathResult_x24_syn_dadd_0(_S209, _S225);
                DiffPair_PathResult_0 _S233;
                _S233.primal_0 = _S213;
                _S233.differential_0 = _S232;
                s_bwd_prop_handleHit_0(_S216, _S233, _s_diff_ctx_11._S23[_dc_0]);
                PathResult_0 _S234 = _S226;
                _S225 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(_S233.differential_0, _S201), _S201);
                _S226 = _S206;
                _S227 = _S201;
                _S228 = _S234;
                _S229 = _S201;
            }
            else
            {
                PathResult_0 _S235 = PathResult_x24_syn_dadd_0(_S224, _S206);
                DiffPair_PathResult_0 _S236;
                _S236.primal_0 = _S211;
                _S236.differential_0 = _S235;
                s_bwd_prop_handleMiss_0(_S214, _S236);
                _S225 = PathResult_x24_syn_dadd_0(PathResult_x24_syn_dadd_0(_S236.differential_0, _S201), _S201);
                _S226 = _S201;
                _S227 = _S207;
                _S228 = _S208;
                _S229 = _S209;
            }
            _S203 = _S225;
            _S205 = _S201;
            _S206 = _S226;
            _S207 = _S227;
            _S208 = _S228;
            _S209 = _S229;
        }
        else
        {
            _S203 = PathResult_x24_syn_dadd_0(_S223, _S201);
        }
        _dc_0 = _dc_0 - int(1);
        _S204 = _S201;
    }
    PathResult_0 _S237 = PathResult_x24_syn_dadd_0(_S203, _S201);
    PathResult_0 _S238 = _S201;
    _S238.L_0 = 0.0;
    _S238.thp_0 = 0.0;
    PathResult_0 dppathRes_13 = PathResult_x24_syn_dadd_0(_S237, _S238);
    dppathRes_12.primal_0 = _S200.primal_0;
    dppathRes_12.differential_0 = dppathRes_13;
    return;
}

void s_bwd_tracePath_0(uint _S239, inout DiffPair_PathResult_0 _S240)
{
    PathResult_0 _S241 = _S240.primal_0;
    PathState_0 _S242;
    s_bwd_prop_tracePath_Intermediates_0 _S243;
    bool _S244 = s_primal_ctx_tracePath_0(_S239, _S242, _S241, _S243);
    s_bwd_prop_tracePath_0(_S239, _S240, _S243);
    return;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{
    PathResult_0 pathRes_0;
    pathRes_0.L_0 = 1.0;
    pathRes_0.thp_0 = 1.0;
    PathResult_0 pathResD_0;
    pathResD_0.L_0 = 1.0;
    pathResD_0.thp_0 = 0.0;
    DiffPair_PathResult_0 dpx_0;
    dpx_0.primal_0 = pathRes_0;
    dpx_0.differential_0 = pathResD_0;
    s_bwd_tracePath_0(1U, dpx_0);
    return;
}

