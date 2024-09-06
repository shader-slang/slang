#version 450
layout(row_major) uniform;
layout(row_major) buffer;
layout(std430, binding = 0) buffer StructuredBuffer_float_t_0 {
    float _data[];
} outputBuffer_0;
struct DiffPair_float_0
{
    float primal_0;
    float differential_0;
};

struct s_bwd_prop_test_simple_loop_Intermediates_0
{
    float  _S1[5];
    int _S2;
};

float s_primal_ctx_test_simple_loop_0(float dpy_0, out s_bwd_prop_test_simple_loop_Intermediates_0 _s_diff_ctx_0)
{
    float  _S3[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    _s_diff_ctx_0._S1 = _S3;
    _s_diff_ctx_0._S2 = 0;
    _s_diff_ctx_0._S1[0] = 0.0;
    _s_diff_ctx_0._S1[1] = 0.0;
    _s_diff_ctx_0._S1[2] = 0.0;
    _s_diff_ctx_0._S1[3] = 0.0;
    _s_diff_ctx_0._S1[4] = 0.0;
    _s_diff_ctx_0._S2 = 0;
    bool _bflag_0 = true;
    float t_0 = dpy_0;
    int i_0 = 0;
    int _pc_0 = 0;
    for(;;)
    {
        _s_diff_ctx_0._S1[_pc_0] = t_0;
        _s_diff_ctx_0._S2 = _pc_0;
        if(_bflag_0)
        {
        }
        else
        {
            break;
        }
        float _S4;
        int _S5;
        if(i_0 < 3)
        {
            float _S6 = t_0 * t_0;
            _S5 = 1;
            _S4 = _S6;
        }
        else
        {
            _S5 = 0;
            _S4 = 0.0;
        }
        if(_S5 != 1)
        {
            _bflag_0 = false;
        }
        if(_bflag_0)
        {
            int _S7 = i_0 + 1;
            t_0 = _S4;
            i_0 = _S7;
        }
        _pc_0 = _pc_0 + 1;
    }
    return t_0;
}

void s_bwd_prop_test_simple_loop_0(inout DiffPair_float_0 dpy_1, float _s_dOut_0, s_bwd_prop_test_simple_loop_Intermediates_0 _s_diff_ctx_1)
{
    int _dc_0 = _s_diff_ctx_1._S2 - 1;
    float _S8 = _s_dOut_0;
    for(;;)
    {
        if(_dc_0 >= 0)
        {
        }
        else
        {
            break;
        }
        bool _S9 = _dc_0 < 3;
        int _S10;
        if(_S9)
        {
            _S10 = 1;
        }
        else
        {
            _S10 = 0;
        }
        float _S11;
        float _S12;
        if(!(_S10 != 1))
        {
            _S11 = _S8;
            _S12 = 0.0;
        }
        else
        {
            _S11 = 0.0;
            _S12 = _S8;
        }
        if(_S9)
        {
            _S8 = _s_diff_ctx_1._S1[_dc_0] * _S11 + _s_diff_ctx_1._S1[_dc_0] * _S11 + _S12;
        }
        else
        {
            _S8 = _S12;
        }
        _dc_0 = _dc_0 - 1;
    }
    dpy_1.primal_0 = dpy_1.primal_0;
    dpy_1.differential_0 = _S8;
    return;
}

void s_bwd_test_simple_loop_0(inout DiffPair_float_0 _S13, float _S14)
{
    s_bwd_prop_test_simple_loop_Intermediates_0 _S15;
    float _S16 = s_primal_ctx_test_simple_loop_0(_S13.primal_0, _S15);
    s_bwd_prop_test_simple_loop_0(_S13, _S14, _S15);
    return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    DiffPair_float_0 dpa_0;
    dpa_0.primal_0 = 1.0;
    dpa_0.differential_0 = 0.0;
    s_bwd_test_simple_loop_0(dpa_0, 1.0);
    outputBuffer_0._data[uint(0)] = dpa_0.differential_0;
    DiffPair_float_0 dpa_1;
    dpa_1.primal_0 = 0.40000000596046448;
    dpa_1.differential_0 = 0.0;
    s_bwd_test_simple_loop_0(dpa_1, 1.0);
    outputBuffer_0._data[uint(1)] = dpa_1.differential_0;
    return;
}

