// MLP-only image fitting test.
//
// InlineVector mode exposes regular slangpy-callable functions.
// WaveTangledVector mode exposes explicit compute entry points selected with
// -DUSE_WAVE_TANGLED=1 from Python.

import slangpy;
import slang.neural;

static const int INPUT_DIM = 2;
static const int HIDDEN_DIM = 128;
static const int OUTPUT_DIM = 3;
static const float ALPHA = 0.01f;

#ifdef USE_WAVE_TANGLED
static const int SUBGROUP_SIZE = 32;
#ifndef WORKGROUP_SIZE
#define WORKGROUP_SIZE 64
#endif
static const int WORKGROUP_SZ = WORKGROUP_SIZE;
static const int SUBGROUP_COUNT = WORKGROUP_SZ / SUBGROUP_SIZE;
typealias ElemType = float;
typealias ShMemSize = SharedMemorySize<ElemType, TargetEnum.CUDA, ExecutionMode.Training,
                                       SUBGROUP_SIZE, SUBGROUP_COUNT>;
typealias ShMemSizeLayer4 = ShMemSize.OfLayer4<INPUT_DIM, HIDDEN_DIM, HIDDEN_DIM,
                                                HIDDEN_DIM, OUTPUT_DIM>;
typealias Address = PointerAddress<ElemType>;
typealias InputVec = WaveTangledVector<ElemType, ShMemSizeLayer4, INPUT_DIM, SUBGROUP_SIZE>;
typealias HiddenVec = WaveTangledVector<ElemType, ShMemSizeLayer4, HIDDEN_DIM, SUBGROUP_SIZE>;
typealias OutputVec = WaveTangledVector<ElemType, ShMemSizeLayer4, OUTPUT_DIM, SUBGROUP_SIZE>;
#else
typealias Address = PointerAddress<float>;
typealias InputVec = InlineVector<float, INPUT_DIM>;
typealias HiddenVec = InlineVector<float, HIDDEN_DIM>;
typealias OutputVec = InlineVector<float, OUTPUT_DIM>;
#endif

typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, Sigmoid<float>>;

static const uint TOTAL_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                                + Layer2.ParameterCount + Layer3.ParameterCount;

int get_total_params() { return int(TOTAL_PARAMS); }

struct LCG
{
    uint state;
    __init(uint seed) { state = seed; }
    [mutating] uint next_uint() { state = 1664525u * state + 1013904223u; return state; }
    [mutating] float next_float() { return (next_uint() >> 8) * 0x1p-24; }
};

#ifdef USE_WAVE_TANGLED
[Differentiable]
OutputVec mlp_forward_coords(
    Address paramAddr,
    no_diff float2 uv)
{
    InputVec input;
    input[0] = uv.x;
    input[1] = uv.y;

    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, paramAddr);
    let a1 = paramAddr.getOffset(Layer0.ParameterCount);
    let h1 = Layer1(act).eval<Address>(h0, a1);
    let a2 = a1.getOffset(Layer1.ParameterCount);
    let h2 = Layer2(act).eval<Address>(h1, a2);
    let a3 = a2.getOffset(Layer2.ParameterCount);
    return Layer3(Sigmoid<float>()).eval<Address>(h2, a3);
}

OutputVec forward_inference(float* params, float2 uv)
{
    Address paramAddr = Address(params);
    InputVec input;
    input[0] = uv.x;
    input[1] = uv.y;

    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, paramAddr);
    let a1 = paramAddr.getOffset(Layer0.ParameterCount);
    let h1 = Layer1(act).eval<Address>(h0, a1);
    let a2 = a1.getOffset(Layer1.ParameterCount);
    let h2 = Layer2(act).eval<Address>(h1, a2);
    let a3 = a2.getOffset(Layer2.ParameterCount);
    return Layer3(Sigmoid<float>()).eval<Address>(h2, a3);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_calculate_grads(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform uint seed,
    uniform int2 batch_size,
    uniform int2 img_resolution,
    uniform float* ref_image,
    uniform float* params,
    uniform float* params_grad)
{
    int sample_idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    int batch_count = batch_size.x * batch_size.y;
    if (sample_idx >= batch_count) return;

    LCG lcg = LCG(seed + uint(sample_idx));
    int2 batch_index = int2(sample_idx % batch_size.x, sample_idx / batch_size.x);
    float2 uv = (float2(batch_index) + float2(lcg.next_float(), lcg.next_float()))
                / float2(batch_size);
    int2 pixel = clamp(int2(uv * float2(img_resolution)), int2(0), img_resolution - 1);
    float2 pixel_uv = (float2(pixel) + 0.5f) / float2(img_resolution);

    int ref_idx = (pixel.y * img_resolution.x + pixel.x) * 3;
    float3 ref_color = float3(ref_image[ref_idx], ref_image[ref_idx + 1], ref_image[ref_idx + 2]);

    OutputVec out = forward_inference(params, pixel_uv);
    float doutArr[OUTPUT_DIM];
    doutArr[0] = 2.0f * (float(out[0]) - ref_color.x) / float(OUTPUT_DIM);
    doutArr[1] = 2.0f * (float(out[1]) - ref_color.y) / float(OUTPUT_DIM);
    doutArr[2] = 2.0f * (float(out[2]) - ref_color.z) / float(OUTPUT_DIM);
    OutputVec dOutput = OutputVec(doutArr);

    bwd_diff(mlp_forward_coords)(
        DifferentialPtrPair<Address>(Address(params), Address(params_grad)),
        pixel_uv,
        dOutput);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_render(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform int2 img_resolution,
    uniform float* params,
    uniform float* output_image)
{
    int pixel_idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    int total_pixels = img_resolution.x * img_resolution.y;
    if (pixel_idx >= total_pixels) return;

    int2 pixel = int2(pixel_idx % img_resolution.x, pixel_idx / img_resolution.x);
    float2 uv = (float2(pixel) + 0.5f) / float2(img_resolution);
    OutputVec out = forward_inference(params, uv);

    int out_idx = pixel_idx * 3;
    output_image[out_idx + 0] = float(out[0]);
    output_image[out_idx + 1] = float(out[1]);
    output_image[out_idx + 2] = float(out[2]);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_show_loss(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform int2 img_resolution,
    uniform float* params,
    uniform float* ref_image,
    uniform float* output_loss)
{
    int pixel_idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    int total_pixels = img_resolution.x * img_resolution.y;
    if (pixel_idx >= total_pixels) return;

    int2 pixel = int2(pixel_idx % img_resolution.x, pixel_idx / img_resolution.x);
    float2 uv = (float2(pixel) + 0.5f) / float2(img_resolution);
    OutputVec out = forward_inference(params, uv);

    int ref_idx = pixel_idx * 3;
    float3 ref_color = float3(ref_image[ref_idx], ref_image[ref_idx + 1], ref_image[ref_idx + 2]);
    output_loss[ref_idx + 0] = pow(float(out[0]) - ref_color.x, 2);
    output_loss[ref_idx + 1] = pow(float(out[1]) - ref_color.y, 2);
    output_loss[ref_idx + 2] = pow(float(out[2]) - ref_color.z, 2);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_optimizer_step(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform float* primal,
    uniform float* grad,
    uniform float* mean_buf,
    uniform float* variance_buf,
    uniform float lr,
    uniform int iter,
    uniform int param_count)
{
    int idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    if (idx >= param_count) return;

    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    float g = grad[idx];
    float m = lerp(g, mean_buf[idx], BETA1);
    float v = lerp(g * g, variance_buf[idx], BETA2);
    float m_hat = m / (1.0f - pow(BETA1, iter));
    float v_hat = v / (1.0f - pow(BETA2, iter));
    primal[idx] -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad[idx] = 0.0f;
    mean_buf[idx] = m;
    variance_buf[idx] = v;
}
#else
[Differentiable]
OutputVec mlp_forward(Address paramAddr, InputVec input)
{
    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, paramAddr);
    let a1 = paramAddr.getOffset(Layer0.ParameterCount);
    let h1 = Layer1(act).eval<Address>(h0, a1);
    let a2 = a1.getOffset(Layer1.ParameterCount);
    let h2 = Layer2(act).eval<Address>(h1, a2);
    let a3 = a2.getOffset(Layer2.ParameterCount);
    return Layer3(Sigmoid<float>()).eval<Address>(h2, a3);
}

struct Network
{
    float* params;
    float* params_grad;
};

[Differentiable]
float3 loss(
    Address paramAddr,
    no_diff int2 pixel, no_diff int2 resolution, no_diff float3 reference)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    InputVec input;
    input[0] = uv.x;
    input[1] = uv.y;
    let out = mlp_forward(paramAddr, input);
    float3 error = float3(out[0], out[1], out[2]) - reference;
    return error * error;
}

void calculate_grads(uint seed, int2 batch_index, int2 batch_size,
                     Tensor<float3, 2> reference, Network network)
{
    LCG lcg = LCG(seed);
    float2 uv = (batch_index + float2(lcg.next_float(), lcg.next_float())) / float2(batch_size);
    int2 resolution = int2(reference.shape[1], reference.shape[0]);
    int2 pixel = int2(uv * float2(resolution));
    float3 ref_color = reference[pixel];

    bwd_diff(loss)(
        DifferentialPtrPair<Address>(Address(network.params), Address(network.params_grad)),
        pixel, resolution, ref_color, float3(1.0f));
}

float3 render(int2 pixel, int2 resolution, Network network)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    InputVec input;
    input[0] = uv.x;
    input[1] = uv.y;
    let out = mlp_forward(Address(network.params), input);
    return float3(out[0], out[1], out[2]);
}

float3 show_loss(int2 pixel, int2 resolution, float3 reference, Network network)
{
    return loss(Address(network.params), pixel, resolution, reference);
}

void optimizer_step(inout float primal, inout float grad,
                    inout float mean, inout float variance,
                    float lr, int iter)
{
    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    mean = lerp(grad, mean, BETA1);
    variance = lerp(grad * grad, variance, BETA2);
    float m_hat = mean / (1.0f - pow(BETA1, iter));
    float v_hat = variance / (1.0f - pow(BETA2, iter));
    primal -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad = 0.0f;
}
#endif
