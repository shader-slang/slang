#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 7 "./tests/autodiff/path-tracer/pt-loop.slang"
RWStructuredBuffer<float > outputBuffer_0 : register(u0);

struct s_diff_PathData_0
{
    float3 thp_0;
};


#line 2633 "core.meta.slang"
s_diff_PathData_0 PathData_x24_syn_dzero_0()
{

#line 2633
    s_diff_PathData_0 result_0;

#line 2633
    result_0.thp_0 = (float3)0.0;

#line 2633
    return result_0;
}


#line 2633
s_diff_PathData_0 PathData_x24_syn_dadd_0(s_diff_PathData_0 SLANG_anonymous_0_0, s_diff_PathData_0 SLANG_anonymous_1_0)
{

#line 2633
    s_diff_PathData_0 result_1;

#line 2633
    result_1.thp_0 = SLANG_anonymous_0_0.thp_0 + SLANG_anonymous_1_0.thp_0;

#line 2633
    return result_1;
}


#line 26 "./tests/autodiff/path-tracer/pt-loop.slang"
bool traceRayInline_0(uint length_0)
{
    if(length_0 < 2U)
    {

#line 28
        return true;
    }
    else
    {

#line 29
        return false;
    }

#line 29
}


#line 53
void _bwd_d_getAlbedo_0(uint length_1, float3 dOut_0)
{
    outputBuffer_0[2U] = outputBuffer_0[2U] + dOut_0.x;
    return;
}


#line 37
float3 getAlbedoDerivative_0(uint length_2)
{
    return float3(1.0, 0.0, 0.0);
}


#line 39
struct DiffPair_float3_0
{
    float3 primal_0;
    float3 differential_0;
};


#line 32
float3 getAlbedo_0(uint length_3)
{
    return float3(0.89999997615814208984, 1.0, 1.0);
}


#line 44
DiffPair_float3_0 _fwd_d_getAlbedo_0(uint length_4)
{

#line 44
    DiffPair_float3_0 _S1 = { getAlbedo_0(length_4), getAlbedoDerivative_0(length_4) };



    return _S1;
}


#line 9
struct PathData_0
{
    float3 thp_1;
    uint length_5;
    bool terminated_0;
    bool isHit_0;
};


#line 9
struct DiffPair_PathData_0
{
    PathData_0 primal_0;
    s_diff_PathData_0 differential_0;
};


#line 9
DiffPair_PathData_0 s_fwd_PathData_x24init_0()
{

#line 9
    PathData_0 _S2 = { (float3)1.0, 0U, false, false };

#line 9
    s_diff_PathData_0 _S3 = { (float3)0.0 };

#line 9
    DiffPair_PathData_0 _S4 = { _S2, _S3 };

#line 9
    return _S4;
}


#line 21
void s_fwd_handleHit_0(inout DiffPair_PathData_0 dppathData_0)
{

#line 21
    DiffPair_PathData_0 _S5 = dppathData_0;

#line 21
    PathData_0 _S6;

#line 21
    if(dppathData_0.primal_0.length_5 >= 2U)
    {

#line 21
        _S6 = _S5.primal_0;

#line 21
        _S6.terminated_0 = true;

#line 21
        dppathData_0.primal_0 = _S6;

#line 21
        dppathData_0.differential_0 = _S5.differential_0;

#line 21
        return;
    }

#line 21
    DiffPair_float3_0 _S7 = _fwd_d_getAlbedo_0(_S5.primal_0.length_5);

#line 21
    float3 _S8 = _S5.primal_0.thp_1 * _S7.primal_0;

#line 21
    float3 _S9 = _S5.differential_0.thp_0 * _S7.primal_0 + _S7.differential_0 * _S5.primal_0.thp_1;

#line 21
    _S6 = _S5.primal_0;

#line 21
    _S6.thp_1 = _S8;

#line 21
    s_diff_PathData_0 _S10 = { _S9 };

#line 21
    _S6.length_5 = _S5.primal_0.length_5 + 1U;

#line 21
    dppathData_0.primal_0 = _S6;

#line 21
    dppathData_0.differential_0 = _S10;

#line 21
    return;
}


#line 21
DiffPair_float3_0 s_fwd_tracePath_0()
{

#line 21
    DiffPair_PathData_0 _S11 = s_fwd_PathData_x24init_0();

#line 21
    PathData_0 pathData_0;

#line 21
    if(traceRayInline_0(_S11.primal_0.length_5))
    {

#line 21
        pathData_0 = _S11.primal_0;

#line 21
        pathData_0.isHit_0 = true;

#line 21
    }
    else
    {

#line 21
        pathData_0 = _S11.primal_0;

#line 21
        pathData_0.terminated_0 = true;

#line 21
        pathData_0.isHit_0 = false;

#line 21
    }

#line 21
    s_diff_PathData_0 s_diff_pathData_0 = _S11.differential_0;

#line 21
    for(;;)
    {

#line 21
        if(!pathData_0.terminated_0)
        {
        }
        else
        {

#line 21
            break;
        }

#line 21
        PathData_0 pathData_1;

#line 21
        if(pathData_0.isHit_0)
        {

#line 21
            DiffPair_PathData_0 _S12;

#line 21
            _S12.primal_0 = pathData_0;

#line 21
            _S12.differential_0 = s_diff_pathData_0;

#line 21
            s_fwd_handleHit_0(_S12);

#line 21
            DiffPair_PathData_0 _S13 = _S12;

#line 21
            if(!traceRayInline_0(_S12.primal_0.length_5))
            {

#line 21
                pathData_1 = _S13.primal_0;

#line 21
                pathData_1.isHit_0 = false;

#line 21
            }
            else
            {

#line 21
                pathData_1 = _S13.primal_0;

#line 21
                pathData_1.isHit_0 = true;

#line 21
            }

#line 21
            pathData_0 = pathData_1;

#line 21
            s_diff_pathData_0 = _S13.differential_0;

#line 21
        }
        else
        {

#line 21
            pathData_1 = pathData_0;

#line 21
            pathData_1.terminated_0 = true;

#line 21
            pathData_0 = pathData_1;

#line 21
        }

#line 21
    }

#line 21
    DiffPair_float3_0 _S14 = { pathData_0.thp_1, s_diff_pathData_0.thp_0 };

#line 21
    return _S14;
}


#line 114
struct s_bwd_handleHit_Intermediates_0
{
    float3 _S15;
    PathData_0 _S16;
};


#line 114
struct s_bwd_tracePath_Intermediates_0
{
    bool  _S17[int(5)];
    PathData_0  _S18[int(5)];
    s_bwd_handleHit_Intermediates_0  _S19[int(5)];
    int _S20;
    PathData_0  _S21[int(5)];
    bool _S22;
};


#line 114
PathData_0 s_bwd_primal_PathData_x24init_0()
{

#line 114
    PathData_0 _S23 = { (float3)1.0, 0U, false, false };

#line 114
    return _S23;
}


#line 93
float3 s_bwd_getAlbedo_0(uint _S24)
{

#line 93
    return getAlbedo_0(_S24);
}


#line 93
void s_bwd_primal_handleHit_0(inout PathData_0 dppathData_1, out s_bwd_handleHit_Intermediates_0 _s_diff_ctx_0)
{

#line 93
    float3 _S25 = (float3)0.0;

#line 93
    PathData_0 _S26 = { _S25, 0U, false, false };

#line 93
    _s_diff_ctx_0._S15 = _S25;

#line 93
    _s_diff_ctx_0._S16 = _S26;

#line 93
    _s_diff_ctx_0._S15 = _S25;

#line 93
    PathData_0 _S27 = dppathData_1;

#line 93
    _s_diff_ctx_0._S16 = dppathData_1;

#line 93
    bool _S28 = _S27.length_5 >= 2U;

#line 93
    PathData_0 _S29;

#line 93
    if(_S28)
    {

#line 93
        _S29 = _S27;

#line 93
        _S29.terminated_0 = true;

#line 93
    }
    else
    {

#line 93
        _S29 = _S27;

#line 93
    }

#line 93
    bool _S30 = !_S28;

#line 93
    if(_S30)
    {

#line 93
        PathData_0 _S31 = _S29;

#line 93
        float3 _S32 = s_bwd_getAlbedo_0(_S29.length_5);

#line 93
        _s_diff_ctx_0._S15 = _S32;

#line 93
        _S29.thp_1 = _S29.thp_1 * _S32;

#line 93
        _S29.length_5 = _S31.length_5 + 1U;

#line 93
    }
    else
    {

#line 93
    }

#line 93
    dppathData_1 = _S29;

#line 93
    return;
}


#line 93
float3 s_bwd_primal_tracePath_0(out s_bwd_tracePath_Intermediates_0 _s_diff_ctx_1)
{

#line 93
    bool  _S33[int(5)] = { false, false, false, false, false };

#line 93
    float3 _S34 = (float3)0.0;

#line 93
    PathData_0 _S35 = { _S34, 0U, false, false };

#line 93
    PathData_0  _S36[int(5)] = { _S35, _S35, _S35, _S35, _S35 };

#line 93
    s_bwd_handleHit_Intermediates_0 _S37 = { _S34, _S35 };

#line 93
    s_bwd_handleHit_Intermediates_0  _S38[int(5)] = { _S37, _S37, _S37, _S37, _S37 };

#line 93
    _s_diff_ctx_1._S17 = _S33;

#line 93
    _s_diff_ctx_1._S18 = _S36;

#line 93
    _s_diff_ctx_1._S19 = _S38;

#line 93
    _s_diff_ctx_1._S20 = int(0);

#line 93
    _s_diff_ctx_1._S21 = _S36;

#line 93
    _s_diff_ctx_1._S22 = false;

#line 93
    _s_diff_ctx_1._S17[int(0)] = false;

#line 93
    _s_diff_ctx_1._S17[int(1)] = false;

#line 93
    _s_diff_ctx_1._S17[int(2)] = false;

#line 93
    _s_diff_ctx_1._S17[int(3)] = false;

#line 93
    _s_diff_ctx_1._S17[int(4)] = false;

#line 93
    _s_diff_ctx_1._S18[int(0)] = _S35;

#line 93
    _s_diff_ctx_1._S18[int(1)] = _S35;

#line 93
    _s_diff_ctx_1._S18[int(2)] = _S35;

#line 93
    _s_diff_ctx_1._S18[int(3)] = _S35;

#line 93
    _s_diff_ctx_1._S18[int(4)] = _S35;

#line 93
    _s_diff_ctx_1._S19[int(0)] = _S37;

#line 93
    _s_diff_ctx_1._S19[int(1)] = _S37;

#line 93
    _s_diff_ctx_1._S19[int(2)] = _S37;

#line 93
    _s_diff_ctx_1._S19[int(3)] = _S37;

#line 93
    _s_diff_ctx_1._S19[int(4)] = _S37;

#line 93
    _s_diff_ctx_1._S20 = int(0);

#line 93
    _s_diff_ctx_1._S21[int(0)] = _S35;

#line 93
    _s_diff_ctx_1._S21[int(1)] = _S35;

#line 93
    _s_diff_ctx_1._S21[int(2)] = _S35;

#line 93
    _s_diff_ctx_1._S21[int(3)] = _S35;

#line 93
    _s_diff_ctx_1._S21[int(4)] = _S35;

#line 93
    PathData_0 _S39 = s_bwd_primal_PathData_x24init_0();

#line 93
    bool _S40 = traceRayInline_0(_S39.length_5);

#line 93
    _s_diff_ctx_1._S22 = _S40;

#line 93
    PathData_0 pathData_2;

#line 93
    if(_S40)
    {

#line 93
        pathData_2 = _S39;

#line 93
        pathData_2.isHit_0 = true;

#line 93
    }
    else
    {

#line 93
        pathData_2 = _S39;

#line 93
        pathData_2.terminated_0 = true;

#line 93
        pathData_2.isHit_0 = false;

#line 93
    }

#line 93
    int _pc_0 = int(0);

#line 93
    for(;;)
    {

#line 93
        _s_diff_ctx_1._S20 = _pc_0;

#line 93
        _s_diff_ctx_1._S21[_pc_0] = pathData_2; // isHit= true, true, false, false
                                                // terminated= false, false, false, true
                                                // length= 0, 1, 2, 2

#line 93
        if(!pathData_2.terminated_0)
        {
        }
        else
        {

#line 93
            break;
        }

#line 93
        PathData_0 pathData_3;

#line 93
        if(pathData_2.isHit_0)
        {

#line 93
            PathData_0 _S41 = pathData_2;

#line 93
            s_bwd_handleHit_Intermediates_0 _S42;

#line 93
            _S42._S15 = _S34;

#line 93
            _S42._S16 = _S35;

#line 93
            s_bwd_primal_handleHit_0(_S41, _S42);

#line 93
            _s_diff_ctx_1._S19[_pc_0] = _S42; 

#line 93
            _s_diff_ctx_1._S18[_pc_0] = pathData_2; 

#line 93
            PathData_0 _S43 = _S41;

#line 93
            bool _S44 = traceRayInline_0(_S41.length_5);

#line 93
            _s_diff_ctx_1._S17[_pc_0] = _S44;

#line 93
            if(!_S44)
            {

#line 93
                pathData_3 = _S43;

#line 93
                pathData_3.isHit_0 = false;

#line 93
            }
            else
            {

#line 93
                pathData_3 = _S43;

#line 93
                pathData_3.isHit_0 = true;

#line 93
            }

#line 93
            pathData_2 = pathData_3;

#line 93
        }
        else
        {

#line 93
            pathData_3 = pathData_2;

#line 93
            pathData_3.terminated_0 = true;

#line 93
            pathData_2 = pathData_3;

#line 93
        }

#line 93
        _pc_0 = _pc_0 + int(1);

#line 93
    }

#line 93
    return pathData_2.thp_1;
}


#line 93
void s_bwd_getAlbedo_1(uint _S45, float3 _S46)
{

#line 93
    _bwd_d_getAlbedo_0(_S45, _S46);

#line 93
    return;
}


#line 93
void s_bwd_handleHit_0(inout DiffPair_PathData_0 dppathData_2, s_bwd_handleHit_Intermediates_0 _s_diff_ctx_2)
{

#line 93
    float3 _S47 = (float3)0.0;

#line 93
    DiffPair_PathData_0 _S48 = dppathData_2;

#line 93
    s_diff_PathData_0 dppathData_3 = PathData_x24_syn_dzero_0();

#line 93
    bool _S49 = _s_diff_ctx_2._S16.length_5 >= 2U;

#line 93
    bool _S50 = !_S49;

#line 93
    PathData_0 _S51;

#line 93
    if(_S49)
    {

#line 93
        _S51 = _s_diff_ctx_2._S16;

#line 93
        _S51.terminated_0 = true;

#line 93
    }
    else
    {

#line 93
        _S51 = _s_diff_ctx_2._S16;

#line 93
    }

#line 93
    float3 _S52;

#line 93
    uint _S53;

#line 93
    if(_S50)
    {

#line 93
        _S52 = _S51.thp_1;

#line 93
        _S53 = _S51.length_5;

#line 93
    }
    else
    {

#line 93
        _S52 = _S47;

#line 93
        _S53 = 0U;

#line 93
    }

#line 93
    s_diff_PathData_0 _S54 = PathData_x24_syn_dadd_0(PathData_x24_syn_dadd_0(_S48.differential_0, dppathData_3), dppathData_3);

#line 93
    s_diff_PathData_0 dppathData_4;

#line 93
    if(_S50)
    {

#line 93
        s_diff_PathData_0 _S55 = { _S47 };

#line 93
        float3 _S56 = _s_diff_ctx_2._S15 * _S54.thp_0;

#line 93
        s_bwd_getAlbedo_1(_S53, _S52 * _S54.thp_0);

#line 93
        s_diff_PathData_0 _S57 = PathData_x24_syn_dadd_0(_S55, dppathData_3);

#line 93
        s_diff_PathData_0 _S58 = dppathData_3;

#line 93
        _S58.thp_0 = _S56;

#line 93
        dppathData_4 = PathData_x24_syn_dadd_0(_S57, _S58);

#line 93
    }
    else
    {

#line 93
        dppathData_4 = PathData_x24_syn_dadd_0(_S54, dppathData_3);

#line 93
    }

#line 93
    if(_S49)
    {

#line 93
    }
    else
    {

#line 93
        dppathData_4 = PathData_x24_syn_dadd_0(dppathData_4, dppathData_3);

#line 93
    }

#line 93
    s_diff_PathData_0 _S59 = PathData_x24_syn_dadd_0(dppathData_3, dppathData_4);

#line 93
    dppathData_2.primal_0 = _S48.primal_0;

#line 93
    dppathData_2.differential_0 = _S59;

#line 93
    return;
}


#line 93
void s_bwd_tracePath_0(float3 _s_dOut_0, s_bwd_tracePath_Intermediates_0 _s_diff_ctx_3)
{

#line 93
    s_diff_PathData_0 _S60 = PathData_x24_syn_dzero_0();

#line 93
    int _S61 = _s_diff_ctx_3._S20 - int(1);

#line 93
    s_diff_PathData_0 _S62 = _S60;

#line 93
    _S62.thp_0 = _s_dOut_0;

#line 93
    s_diff_PathData_0 _S63 = PathData_x24_syn_dadd_0(_S60, _S62);

#line 93
    int _dc_0 = _S61;

#line 93
    s_diff_PathData_0 _S64 = _S63;

#line 93
    s_diff_PathData_0 _S65 = _S60;

#line 93
    for(;;)
    {

#line 93
        if(_dc_0 >= int(0))
        {
        }
        else
        {

#line 93
            break;
        }

#line 93
        int _S66 = _dc_0;

#line 93
        bool _S67;

#line 93
        if(_s_diff_ctx_3._S21[_dc_0].isHit_0)
        {

#line 93
            _S67 = !_s_diff_ctx_3._S17[_dc_0];

#line 93
        }
        else
        {

#line 93
            _S67 = false;

#line 93
        }

#line 93
        if(_s_diff_ctx_3._S21[_S66].isHit_0)
        {

#line 93
            s_diff_PathData_0 _S68;

#line 93
            if(_S67)
            {

#line 93
                _S68 = _S64;

#line 93
            }
            else
            {

#line 93
                _S68 = PathData_x24_syn_dadd_0(_S64, _S65);

#line 93
            }

#line 93
            s_diff_PathData_0 _S69 = PathData_x24_syn_dadd_0(_S60, _S68);

#line 93
            DiffPair_PathData_0 _S70;

#line 93
            _S70.primal_0 = _s_diff_ctx_3._S18[_dc_0];

#line 93
            _S70.differential_0 = _S69;

#line 93
            s_bwd_handleHit_0(_S70, _s_diff_ctx_3._S19[_dc_0]);

#line 93
            _S64 = PathData_x24_syn_dadd_0(PathData_x24_syn_dadd_0(_S70.differential_0, _S60), _S60);

#line 93
            _S65 = _S70.differential_0;

#line 93
        }
        else
        {

#line 93
            _S64 = PathData_x24_syn_dadd_0(_S64, _S60);

#line 93
        }

#line 93
        _dc_0 = _dc_0 - int(1);

#line 93
    }

#line 93
    return;
}


#line 93
void s_bwd_tracePath_1(float3 _S71)
{

#line 93
    s_bwd_tracePath_Intermediates_0 _S72;

#line 93
    float3 _S73 = s_bwd_primal_tracePath_0(_S72);

#line 93
    s_bwd_tracePath_0(_S71, _S72);

#line 93
    return;
}


#line 108
[numthreads(1, 1, 1)]
void computeMain(uint3 dispathThreadID_0 : SV_DISPATCHTHREADID)
{

#line 110
    DiffPair_float3_0 dpThp_0 = s_fwd_tracePath_0();
    float _S74 = dpThp_0.primal_0.x;

#line 111
    outputBuffer_0[0U] = _S74;
    float _S75 = dpThp_0.differential_0.x;

#line 112
    outputBuffer_0[1U] = _S75;

    s_bwd_tracePath_1(float3(1.0, 0.0, 0.0));
    return;
}

