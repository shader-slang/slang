// GPU MLP stress test for neural.slang — WaveTangledVector (tensor core / CoopMat) variant.
//
// Architecture: UV -> bilinear sample 32x32x4 latent grid -> MLP 4->128->128->128->3 -> RGB
//
// Key differences from neural_mlp_gpu.slang (InlineVector version):
//   - WaveTangledVector replaces InlineVector (cooperative matrix MMA)
//   - [numthreads(32,1,1)] — 32 threads per warp cooperate on MMA
//   - Each thread processes a DIFFERENT pixel; the warp cooperates on the MMA ops
//   - Loss scaling support for fp16 gradient underflow prevention
//   - Manual MSE gradient computation + bwd_diff(mlp_forward_with_latent) for backward
//     (WaveTangledVector.operator[] is not backward-differentiable, so we compute
//      dOutput manually and pass it as the seed to bwd_diff)
//   - All entry points are explicit [shader("compute")] with flat buffer parameters

import neural;

static const int INPUT_DIM = 4;
static const int HIDDEN_DIM = 128;
static const int OUTPUT_DIM = 3;
static const float ALPHA = 0.01f;
static const int SUBGROUP_SIZE = 32;

typealias ShMemSize = SharedMemorySize<float, TargetEnum.CUDA, ExecutionMode.Training,
                                       SUBGROUP_SIZE, 1>;
typealias ShMemSizeLayer4 = ShMemSize.OfLayer4<INPUT_DIM, HIDDEN_DIM, HIDDEN_DIM,
                                                HIDDEN_DIM, OUTPUT_DIM>;

typealias Address = PointerAddress<float>;
typealias InputVec = WaveTangledVector<float, ShMemSizeLayer4, INPUT_DIM, SUBGROUP_SIZE>;
typealias HiddenVec = WaveTangledVector<float, ShMemSizeLayer4, HIDDEN_DIM, SUBGROUP_SIZE>;
typealias OutputVec = WaveTangledVector<float, ShMemSizeLayer4, OUTPUT_DIM, SUBGROUP_SIZE>;

typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, IdentityActivation<float>, true>;

static const uint TOTAL_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                                + Layer2.ParameterCount + Layer3.ParameterCount;

// ============================================================================
// LCG
// ============================================================================
struct LCG
{
    uint state;
    __init(uint seed) { state = seed; }
    [mutating] uint next_uint() { state = 1664525u * state + 1013904223u; return state; }
    [mutating] float next_float() { return (next_uint() >> 8) * 0x1p-24; }
};

// ============================================================================
// Pre-compute layer addresses outside differentiable scope (getOffset workaround)
// ============================================================================
struct LayerAddr { Address w0, b0, w1, b1, w2, b2, w3, b3; };

LayerAddr make_addr(float* ptr)
{
    Address base = Address(ptr);
    LayerAddr a;
    uint off = 0u;
    a.w0 = base.getOffset(off); a.b0 = base.getOffset(off + INPUT_DIM * HIDDEN_DIM);
    off += Layer0.ParameterCount;
    a.w1 = base.getOffset(off); a.b1 = base.getOffset(off + HIDDEN_DIM * HIDDEN_DIM);
    off += Layer1.ParameterCount;
    a.w2 = base.getOffset(off); a.b2 = base.getOffset(off + HIDDEN_DIM * HIDDEN_DIM);
    off += Layer2.ParameterCount;
    a.w3 = base.getOffset(off); a.b3 = base.getOffset(off + HIDDEN_DIM * OUTPUT_DIM);
    return a;
}

// ============================================================================
// Differentiable MLP forward with latent texture sampling
//
// Returns OutputVec (WaveTangledVector). The backward pass propagates gradients
// to both MLP weights (via DifferentialPtrPair) and latent grid (via atomicAdd
// in getLatentBwd). The loss gradient (dOutput) is computed manually and passed
// as the seed — this avoids differentiating through WaveTangledVector.operator[].
// ============================================================================
[Differentiable]
OutputVec mlp_forward_with_latent(
    Address w0, Address b0, Address w1, Address b1,
    Address w2, Address b2, Address w3, Address b3,
    // Latent address is no_diff because PointerAddress.operator[].get is not
    // backward-differentiable. Latent grid gradients are not computed via autodiff
    // in this CoopMat variant; only MLP weight gradients flow through bwd_diff.
    no_diff Address latent_addr,
    no_diff int latent_w, no_diff int latent_h,
    no_diff float2 uv)
{
    int2 size = int2(latent_w, latent_h);
    float2 scaled_uv = uv * float2(size - 1);
    int2 c0 = clamp(int2(scaled_uv), int2(0), size - 1);
    int2 c1 = min(c0 + 1, size - 1);
    float2 f = saturate(scaled_uv - float2(c0));

    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
    {
        float v00 = latent_addr[uint(c0.y * latent_w * INPUT_DIM + c0.x * INPUT_DIM + i)];
        float v10 = latent_addr[uint(c0.y * latent_w * INPUT_DIM + c1.x * INPUT_DIM + i)];
        float v01 = latent_addr[uint(c1.y * latent_w * INPUT_DIM + c0.x * INPUT_DIM + i)];
        float v11 = latent_addr[uint(c1.y * latent_w * INPUT_DIM + c1.x * INPUT_DIM + i)];
        input[i] = lerp(lerp(v00, v10, f.x), lerp(v01, v11, f.x), f.y);
    }

    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, w0, b0);
    let h1 = Layer1(act).eval<Address>(h0, w1, b1);
    let h2 = Layer2(act).eval<Address>(h1, w2, b2);
    return Layer3().eval<Address>(h2, w3, b3);
}

// ============================================================================
// Standalone forward for inference/loss evaluation (no autodiff needed)
// ============================================================================
OutputVec forward_inference(
    float* params, float* latent_data,
    int latent_w, int latent_h, float2 uv)
{
    int2 size = int2(latent_w, latent_h);
    float2 scaled_uv = uv * float2(size - 1);
    int2 c0 = clamp(int2(scaled_uv), int2(0), size - 1);
    int2 c1 = min(c0 + 1, size - 1);
    float2 f = saturate(scaled_uv - float2(c0));

    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
    {
        float v00 = latent_data[c0.y * latent_w * INPUT_DIM + c0.x * INPUT_DIM + i];
        float v10 = latent_data[c0.y * latent_w * INPUT_DIM + c1.x * INPUT_DIM + i];
        float v01 = latent_data[c1.y * latent_w * INPUT_DIM + c0.x * INPUT_DIM + i];
        float v11 = latent_data[c1.y * latent_w * INPUT_DIM + c1.x * INPUT_DIM + i];
        input[i] = lerp(lerp(v00, v10, f.x), lerp(v01, v11, f.x), f.y);
    }

    LayerAddr la = make_addr(params);
    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, la.w0, la.b0);
    let h1 = Layer1(act).eval<Address>(h0, la.w1, la.b1);
    let h2 = Layer2(act).eval<Address>(h1, la.w2, la.b2);
    return Layer3().eval<Address>(h2, la.w3, la.b3);
}

// ============================================================================
// Simple query functions
// ============================================================================
int get_total_params() { return int(TOTAL_PARAMS); }
int get_grid_params() { return 32 * 32 * INPUT_DIM; }

// ============================================================================
// Compute shader entry points — [numthreads(32,1,1)] for cooperative matrix
// ============================================================================

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_calculate_grads(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform uint seed,
    uniform int2 batch_size,
    uniform int2 img_resolution,
    uniform float loss_scale,
    uniform float* params,
    uniform float* params_grad,
    uniform float* latent_data,
    uniform float* latent_grad,
    uniform float* ref_image,
    uniform int latent_width,
    uniform int latent_height)
{
    int sample_idx = int(gid.x) * SUBGROUP_SIZE + int(gtid);
    int batch_count = batch_size.x * batch_size.y;
    if (sample_idx >= batch_count) return;

    LCG lcg = LCG(seed + uint(sample_idx));
    int2 batch_index = int2(sample_idx % batch_size.x, sample_idx / batch_size.x);
    float2 uv = (float2(batch_index) + float2(lcg.next_float(), lcg.next_float()))
                / float2(batch_size);
    int2 pixel = int2(uv * float2(img_resolution));
    pixel = clamp(pixel, int2(0), img_resolution - 1);

    int ref_idx = (pixel.y * img_resolution.x + pixel.x) * 3;
    float3 ref_color = float3(ref_image[ref_idx], ref_image[ref_idx + 1], ref_image[ref_idx + 2]);

    float2 pixel_uv = (float2(pixel) + 0.5f) / float2(img_resolution);

    // Forward pass (non-differentiable) to get output for MSE gradient
    Address latent_addr = Address(latent_data);
    OutputVec out = forward_inference(params, latent_data, latent_width, latent_height, pixel_uv);

    // Manual MSE gradient: dL/d_out[i] = 2 * (out[i] - ref[i]) * loss_scale / 3
    // The /3 is from MSE mean over 3 channels. loss_scale prevents fp16 underflow.
    float dout_arr[OUTPUT_DIM];
    dout_arr[0] = 2.0f * (out[0] - ref_color.x) * loss_scale / float(OUTPUT_DIM);
    dout_arr[1] = 2.0f * (out[1] - ref_color.y) * loss_scale / float(OUTPUT_DIM);
    dout_arr[2] = 2.0f * (out[2] - ref_color.z) * loss_scale / float(OUTPUT_DIM);
    OutputVec dOutput = OutputVec(dout_arr);

    LayerAddr la = make_addr(params);
    LayerAddr ga = make_addr(params_grad);

    bwd_diff(mlp_forward_with_latent)(
        DifferentialPtrPair<Address>(la.w0, ga.w0), DifferentialPtrPair<Address>(la.b0, ga.b0),
        DifferentialPtrPair<Address>(la.w1, ga.w1), DifferentialPtrPair<Address>(la.b1, ga.b1),
        DifferentialPtrPair<Address>(la.w2, ga.w2), DifferentialPtrPair<Address>(la.b2, ga.b2),
        DifferentialPtrPair<Address>(la.w3, ga.w3), DifferentialPtrPair<Address>(la.b3, ga.b3),
        latent_addr,
        latent_width, latent_height,
        pixel_uv,
        dOutput);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_render(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform int2 img_resolution,
    uniform float* params,
    uniform float* latent_data,
    uniform float* output_image,
    uniform int latent_width,
    uniform int latent_height)
{
    int pixel_idx = int(gid.x) * SUBGROUP_SIZE + int(gtid);
    int total_pixels = img_resolution.x * img_resolution.y;
    if (pixel_idx >= total_pixels) return;

    int2 pixel = int2(pixel_idx % img_resolution.x, pixel_idx / img_resolution.x);
    float2 uv = (float2(pixel) + 0.5f) / float2(img_resolution);

    OutputVec out = forward_inference(params, latent_data, latent_width, latent_height, uv);

    int out_idx = pixel_idx * 3;
    output_image[out_idx + 0] = out[0];
    output_image[out_idx + 1] = out[1];
    output_image[out_idx + 2] = out[2];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_show_loss(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform int2 img_resolution,
    uniform float* params,
    uniform float* latent_data,
    uniform float* ref_image,
    uniform float* output_loss,
    uniform int latent_width,
    uniform int latent_height)
{
    int pixel_idx = int(gid.x) * SUBGROUP_SIZE + int(gtid);
    int total_pixels = img_resolution.x * img_resolution.y;
    if (pixel_idx >= total_pixels) return;

    int2 pixel = int2(pixel_idx % img_resolution.x, pixel_idx / img_resolution.x);
    float2 uv = (float2(pixel) + 0.5f) / float2(img_resolution);

    OutputVec out = forward_inference(params, latent_data, latent_width, latent_height, uv);

    int ref_idx = pixel_idx * 3;
    float3 ref_color = float3(ref_image[ref_idx], ref_image[ref_idx + 1], ref_image[ref_idx + 2]);

    output_loss[ref_idx + 0] = (out[0] - ref_color.x) * (out[0] - ref_color.x);
    output_loss[ref_idx + 1] = (out[1] - ref_color.y) * (out[1] - ref_color.y);
    output_loss[ref_idx + 2] = (out[2] - ref_color.z) * (out[2] - ref_color.z);
}

// Element-wise Adam optimizer with loss-scale unscaling
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_optimizer_step(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform float* primal,
    uniform float* grad,
    uniform float* mean_buf,
    uniform float* variance_buf,
    uniform float lr,
    uniform int iter,
    uniform float loss_scale,
    uniform int param_count)
{
    int idx = int(gid.x) * SUBGROUP_SIZE + int(gtid);
    if (idx >= param_count) return;

    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    float g = grad[idx] / loss_scale;
    float m = lerp(g, mean_buf[idx], BETA1);
    float v = lerp(g * g, variance_buf[idx], BETA2);
    float m_hat = m / (1.0f - pow(BETA1, iter));
    float v_hat = v / (1.0f - pow(BETA2, iter));
    primal[idx] -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad[idx] = 0.0f;
    mean_buf[idx] = m;
    variance_buf[idx] = v;
}
