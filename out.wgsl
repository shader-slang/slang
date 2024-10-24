@binding(0) @group(0) var<storage, read_write> outputBuffer_0 : array<f32>;

fn computeLoop_0( y_0 : f32) -> vec3<f32>
{
    return vec3<f32>(0.0f);
}

struct DiffPair_float_0
{
     primal_0 : f32,
     differential_0 : f32,
};

struct s_bwd_prop_computeLoop_Intermediates_0
{
     _S1 : i32,
     _S2 : f32,
     _S3 : vec3<f32>,
};

fn s_primal_ctx_compute_0( dpx_0 : f32,  dpy_0 : f32) -> f32
{
    return dpx_0 * dpy_0;
}

fn s_primal_ctx_computeLoop_0( dpy_1 : f32,  _s_diff_ctx_0 : ptr<function, s_bwd_prop_computeLoop_Intermediates_0>) -> vec3<f32>
{
    var _S4 : vec3<f32> = vec3<f32>(0.0f);
    (*_s_diff_ctx_0)._S1 = i32(0);
    (*_s_diff_ctx_0)._S2 = 0.0f;
    (*_s_diff_ctx_0)._S3 = _S4;
    (*_s_diff_ctx_0)._S1 = i32(0);
    (*_s_diff_ctx_0)._S2 = 0.0f;
    (*_s_diff_ctx_0)._S3 = _S4;
    var _S5 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var _bflag_0 : bool = true;
    var w_0 : f32 = 0.0f;
    var w3_0 : vec3<f32> = _S5;
    var i_0 : i32 = i32(0);
    var _pc_0 : i32 = i32(0);
    for(;;)
    {
        (*_s_diff_ctx_0)._S1 = _pc_0;
        (*_s_diff_ctx_0)._S2 = w_0;
        (*_s_diff_ctx_0)._S3 = w3_0;
        if(_bflag_0)
        {
        }
        else
        {
            break;
        }
        var g_0 : f32;
        var _S6 : vec3<f32>;
        var _S7 : i32;
        if(i_0 < i32(8))
        {
            var _S8 : f32 = s_primal_ctx_compute_0(f32(i_0), dpy_1);
            if(_S8 > 0.0f)
            {
                g_0 = _S8;
            }
            else
            {
                g_0 = 0.0f;
            }
            var w_1 : f32 = w_0 + g_0;
            var w3_1 : vec3<f32> = w3_0 + vec3<f32>(g_0) * dpy_1;
            _S7 = i32(1);
            g_0 = w_1;
            _S6 = w3_1;
        }
        else
        {
            _S7 = i32(0);
            g_0 = 0.0f;
            _S6 = _S4;
        }
        if(_S7 != i32(1))
        {
            _bflag_0 = false;
        }
        if(_bflag_0)
        {
            var _S9 : i32 = i_0 + i32(1);
            w_0 = g_0;
            w3_0 = _S6;
            i_0 = _S9;
        }
        _pc_0 = _pc_0 + i32(1);
    }
    return _S4;
}

fn s_bwd_prop_compute_0( dpx_1 : ptr<function, DiffPair_float_0>,  dpy_2 : ptr<function, DiffPair_float_0>,  _s_dOut_0 : f32)
{
    var _S10 : f32 = (*dpx_1).primal_0 * _s_dOut_0;
    var _S11 : f32 = (*dpy_2).primal_0 * _s_dOut_0;
    (*dpy_2).primal_0 = (*dpy_2).primal_0;
    (*dpy_2).differential_0 = _S10;
    (*dpx_1).primal_0 = (*dpx_1).primal_0;
    (*dpx_1).differential_0 = _S11;
    return;
}

fn s_bwd_prop_computeLoop_0( dpy_3 : ptr<function, DiffPair_float_0>,  s_diff_p_T_0 : vec3<f32>,  _s_diff_ctx_1 : s_bwd_prop_computeLoop_Intermediates_0)
{
    var _S12 : DiffPair_float_0 = (*dpy_3);
    var _S13 : vec3<f32> = vec3<f32>((*dpy_3).primal_0);
    var _S14 : vec3<f32> = vec3<f32>(_s_diff_ctx_1._S2);
    var _S15 : vec3<f32> = vec3<f32>(0.0f);
    var _S16 : vec3<f32> = s_diff_p_T_0 / (_S14 * _S14);
    var _S17 : vec3<f32> = _s_diff_ctx_1._S3 * - _S16;
    var _S18 : vec3<f32> = _S14 * _S16;
    var _S19 : f32 = _S17[i32(0)] + _S17[i32(1)] + _S17[i32(2)];
    var _dc_0 : i32 = _s_diff_ctx_1._S1 - i32(1);
    var _S20 : f32 = _S19;
    var _S21 : vec3<f32> = _S18;
    var _S22 : vec3<f32> = _S15;
    var _S23 : f32 = 0.0f;
    var _S24 : vec3<f32> = _S15;
    for(;;)
    {
        if(_dc_0 >= i32(0))
        {
        }
        else
        {
            break;
        }
        var _S25 : bool = _dc_0 < i32(8);
        var _S26 : i32;
        var g_1 : f32;
        var _S27 : vec3<f32>;
        var _S28 : bool;
        if(_S25)
        {
            var _S29 : f32 = f32(_dc_0);
            var _S30 : f32 = s_primal_ctx_compute_0(_S29, _S12.primal_0);
            var _S31 : bool = _S30 > 0.0f;
            if(_S31)
            {
                g_1 = _S30;
            }
            else
            {
                g_1 = 0.0f;
            }
            var _S32 : vec3<f32> = vec3<f32>(g_1);
            _S26 = i32(1);
            _S27 = _S32;
            _S28 = _S31;
            g_1 = _S29;
        }
        else
        {
            _S26 = i32(0);
            _S27 = _S15;
            _S28 = false;
            g_1 = 0.0f;
        }
        var _S33 : f32;
        var _S34 : f32;
        var _S35 : vec3<f32>;
        var _S36 : vec3<f32>;
        if(!(_S26 != i32(1)))
        {
            var _S37 : vec3<f32> = _S21 + _S24;
            _S33 = _S20;
            _S35 = _S37;
            _S34 = 0.0f;
            _S36 = _S15;
        }
        else
        {
            _S33 = 0.0f;
            _S35 = _S24;
            _S34 = _S20;
            _S36 = _S21;
        }
        if(_S25)
        {
            var _S38 : vec3<f32> = _S13 * _S35;
            var _S39 : f32 = _S38[i32(0)] + _S38[i32(1)] + _S38[i32(2)] + _S33;
            var _S40 : f32 = _S33 + _S34;
            var _S41 : vec3<f32> = _S35 + _S36;
            var _S42 : vec3<f32> = _S27 * _S35 + _S22;
            var _S43 : f32;
            if(_S28)
            {
                _S43 = _S39;
            }
            else
            {
                _S43 = 0.0f;
            }
            var _S44 : DiffPair_float_0;
            _S44.primal_0 = g_1;
            _S44.differential_0 = 0.0f;
            var _S45 : DiffPair_float_0;
            _S45.primal_0 = _S12.primal_0;
            _S45.differential_0 = 0.0f;
            s_bwd_prop_compute_0(&(_S44), &(_S45), _S43);
            var _S46 : f32 = _S45.differential_0 + _S23;
            _S20 = _S40;
            _S21 = _S41;
            _S22 = _S42;
            _S23 = _S46;
        }
        else
        {
            _S20 = _S34;
            _S21 = _S36;
        }
        _dc_0 = _dc_0 - i32(1);
        _S24 = _S15;
    }
    var _S47 : f32 = _S22[i32(0)] + _S22[i32(1)] + _S22[i32(2)] + _S23;
    (*dpy_3).primal_0 = (*dpy_3).primal_0;
    (*dpy_3).differential_0 = _S47;
    return;
}

fn s_bwd_prop_test_simple_loop_0( dpy_4 : ptr<function, DiffPair_float_0>,  _s_dOut_1 : f32)
{
    var _S48 : vec3<f32> = vec3<f32>(0.0f);
    var _S49 : s_bwd_prop_computeLoop_Intermediates_0;
    _S49._S1 = i32(0);
    _S49._S2 = 0.0f;
    _S49._S3 = _S48;
    var _S50 : vec3<f32> = s_primal_ctx_computeLoop_0((*dpy_4).primal_0, &(_S49));
    var _S51 : vec3<f32> = vec3<f32>(_s_dOut_1, 0.0f, 0.0f);
    var _S52 : DiffPair_float_0;
    _S52.primal_0 = (*dpy_4).primal_0;
    _S52.differential_0 = 0.0f;
    s_bwd_prop_computeLoop_0(&(_S52), _S51, _S49);
    var _S53 : f32 = _s_dOut_1 + _S52.differential_0;
    (*dpy_4).primal_0 = (*dpy_4).primal_0;
    (*dpy_4).differential_0 = _S53;
    return;
}

fn s_bwd_test_simple_loop_0( _S54 : ptr<function, DiffPair_float_0>,  _S55 : f32)
{
    s_bwd_prop_test_simple_loop_0(&((*_S54)), _S55);
    return;
}

@compute
@workgroup_size(1, 1, 1)
fn computeMain(@builtin(global_invocation_id) dispatchThreadID_0 : vec3<u32>)
{
    var dpa_0 : DiffPair_float_0;
    dpa_0.primal_0 = 1.0f;
    dpa_0.differential_0 = 0.0f;
    s_bwd_test_simple_loop_0(&(dpa_0), 1.0f);
    outputBuffer_0[i32(0)] = dpa_0.differential_0;
    var dpa_1 : DiffPair_float_0;
    dpa_1.primal_0 = 0.40000000596046448f;
    dpa_1.differential_0 = 0.0f;
    s_bwd_test_simple_loop_0(&(dpa_1), 0.5f);
    outputBuffer_0[i32(1)] = dpa_1.differential_0;
    outputBuffer_0[i32(2)] = computeLoop_0(1.0f).x;
    return;
}

